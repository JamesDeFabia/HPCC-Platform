/*##############################################################################

    HPCC SYSTEMS software Copyright (C) 2021 HPCC Systems®.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
############################################################################## */

#pragma warning (disable : 4786)
// TpWrapper.cpp: implementation of the CTpWrapper class.
//
//////////////////////////////////////////////////////////////////////

#include "TpWrapper.hpp"
#include <stdio.h>
#include "workunit.hpp"
#include "exception_util.hpp"
#include "portlist.h"
#include "daqueue.hpp"
#include "dautils.hpp"
#include "dameta.hpp"

void CTpWrapper::appendTpMachine(double clientVersion, IConstEnvironment* constEnv, IConstInstanceInfo& instanceInfo, IArrayOf<IConstTpMachine>& machines)
{
    SCMStringBuffer name, networkAddress, description, directory;
    Owned<IEspTpMachine> machine = createTpMachine();
    Owned<IConstMachineInfo> machineInfo = instanceInfo.getMachine();
    machine->setName(machineInfo->getName(name).str());
    machine->setOS(machineInfo->getOS());
    machine->setNetaddress(machineInfo->getNetAddress(networkAddress).str());
    machine->setDirectory(instanceInfo.getDirectory(directory).str());
    machine->setPort(instanceInfo.getPort());
    machine->setType(eqSparkThorProcess); //for now, the appendTpMachine is only used for SparkThor.
    machines.append(*machine.getLink());
}

extern TPWRAPPER_API ISashaCommand* archiveOrRestoreWorkunits(StringArray& wuids, IProperties* params, bool archive, bool dfu)
{
    StringBuffer sashaAddress;
    if (params && params->hasProp("sashaServerIP"))
    {
        sashaAddress.set(params->queryProp("sashaServerIP"));
        sashaAddress.append(':').append(params->getPropInt("sashaServerPort", DEFAULT_SASHA_PORT));
    }
    else
        getSashaService(sashaAddress, "sasha-wu-archiver", true);

    SocketEndpoint ep(sashaAddress);
    Owned<INode> node = createINode(ep);
    Owned<ISashaCommand> cmd = createSashaCommand();
    cmd->setAction(archive ? SCA_ARCHIVE : SCA_RESTORE);
    if (dfu)
        cmd->setDFU(true);

    ForEachItemIn(i, wuids)
        cmd->addId(wuids.item(i));

    if (!cmd->send(node, 1*60*1000))
        throw MakeStringException(ECLWATCH_CANNOT_CONNECT_ARCHIVE_SERVER,
            "Sasha (%s) took too long to respond for Archive/restore workunit.",
            sashaAddress.str());
    return cmd.getClear();
}

extern TPWRAPPER_API IStringIterator* getContainerTargetClusters(const char* processType, const char* processName)
{
    Owned<CStringArrayIterator> ret = new CStringArrayIterator;
    Owned<IPropertyTreeIterator> queues = getComponentConfigSP()->getElements("queues");
    ForEach(*queues)
    {
        IPropertyTree& queue = queues->query();
        if (!isEmptyString(processType))
        {
            const char* type = queue.queryProp("@type");
            if (isEmptyString(type) || !strieq(type, processType))
                continue;
        }
        const char* qName = queue.queryProp("@name");
        if (isEmptyString(qName))
            continue;

        if (!isEmptyString(processName) && !strieq(qName, processName))
            continue;

        ret->append_unique(qName);
    }
    if (!isEmptyString(processType) && !strieq("roxie", processType))
        return ret.getClear();

    Owned<IPropertyTreeIterator> services = getGlobalConfigSP()->getElements("services[@type='roxie']");
    ForEach(*services)
    {
        IPropertyTree& service = services->query();
        const char* targetName = service.queryProp("@target");
        if (isEmptyString(targetName))
            continue;

        if (!isEmptyString(processName) && !strieq(targetName, processName))
            continue;

        ret->append_unique(targetName);
    }
    return ret.getClear();
}

extern TPWRAPPER_API bool matchNetAddressRequest(const char* netAddressReg, bool ipReq, IConstTpMachine& tpMachine)
{
    if (ipReq)
        return streq(netAddressReg, tpMachine.getNetaddress());
    return streq(netAddressReg, tpMachine.getConfigNetaddress());
}

extern TPWRAPPER_API bool validateDropZonePath(const char* dropZoneName, const char* netAddr, const char* pathToCheck)
{
    if (isEmptyString(netAddr))
        throw makeStringException(ECLWATCH_INVALID_INPUT, "NetworkAddress not defined.");

    if (isEmptyString(pathToCheck))
        throw makeStringException(ECLWATCH_INVALID_INPUT, "Path not defined.");

    if (containsRelPaths(pathToCheck)) //Detect a path like: /home/lexis/runtime/var/lib/HPCCSystems/mydropzone/../../../
        return false;

    bool isIPAddressReq = isIPAddress(netAddr);
    IArrayOf<IConstTpDropZone> allTpDropZones;
    CTpWrapper tpWrapper;
    tpWrapper.getTpDropZones(9999, nullptr, false, allTpDropZones); //version 9999: get the latest information about dropzone
    ForEachItemIn(i, allTpDropZones)
    {
        IConstTpDropZone& dropZone = allTpDropZones.item(i);
        if (!isEmptyString(dropZoneName) && !streq(dropZoneName, dropZone.getName()))
            continue;

        StringBuffer directory(dropZone.getPath());
        addPathSepChar(directory);

        if (!hasPrefix(pathToCheck, directory, true))
            continue;

        IArrayOf<IConstTpMachine>& tpMachines = dropZone.getTpMachines();
        ForEachItemIn(ii, tpMachines)
        {
            if (matchNetAddressRequest(netAddr, isIPAddressReq, tpMachines.item(ii)))
                return true;
        }
    }
    return false;
}

extern TPWRAPPER_API SecAccessFlags getDropZoneScopePermissions(IEspContext& context, const char* dropZoneName, const char* dropZonePath, const char* dropZoneHost)
{
    if (isEmptyString(dropZonePath))
        throw makeStringException(ECLWATCH_INVALID_CLUSTER_NAME, "getDropZoneScopePermissions(): DropZone path must be specified.");

    Owned<IPropertyTree> plane;
    if (isEmptyString(dropZoneName))
    {
        plane.setown(findDropZonePlane(dropZonePath, dropZoneHost, true, true));
        dropZoneName = plane->queryProp("@name");
    }
    else
    {
        plane.setown(getDropZonePlane(dropZoneName));
        if (!plane)
            throw makeStringExceptionV(ECLWATCH_INVALID_INPUT, "getDropZoneScopePermissions(): DropZone %s not found.", dropZoneName);
    }

    //If the dropZonePath is an absolute path, change it to a relative path.
    StringBuffer s;
    const char* prefix = plane->queryProp("@prefix");
    if (hasPrefix(dropZonePath, prefix, true))
    {
        const char* p = dropZonePath + strlen(prefix);
        if (!*p || !isPathSepChar(p[0]))
            addPathSepChar(s);
        s.append(p);
        dropZonePath = s.str();
    }

    Owned<IUserDescriptor> userDesc = createUserDescriptor();
    userDesc->set(context.queryUserId(), context.queryPassword(), context.querySignature());
    return queryDistributedFileDirectory().getDropZoneScopePermissions(dropZoneName, dropZonePath, userDesc);
}

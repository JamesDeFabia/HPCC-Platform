/*##############################################################################

    Copyright (C) 2011 HPCC Systems.

    All rights reserved. This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
############################################################################## */


#include "jlib.hpp"
#include "eclhelper.hpp"

#include "thjoin.ipp"

#include "thor.hpp"
#include "thexception.hpp"
#include "thbufdef.hpp"
#include "thorport.hpp"

#include "tsortm.hpp"

#define JOIN_SOCKETS 2

class JoinActivityMaster : public CMasterActivity
{
    IThorSorterMaster *imaster;
    IHThorJoinArg *helper;
    bool islocal;
    bool rightpartition;
    unsigned selfJoinWarnLevel, lastMsgTime;
    mptag_t mpTagRPC;
    Owned<IBarrier> barrier;
    Owned<ProgressInfo> lhsProgress, rhsProgress;

    bool nosortPrimary()
    {
        if (ALWAYS_SORT_PRIMARY)
            return false;
        return (rightpartition?helper->isRightAlreadySorted():helper->isLeftAlreadySorted());
    }

    bool nosortSecondary()
    {
        return (rightpartition?helper->isLeftAlreadySorted():helper->isRightAlreadySorted());
    }
    class cLimitedCmp: implements ICompare  // special to improve limited match partitioning
    {
        ICompare *base;
        ICompare *pcmp;
    public:
        cLimitedCmp(ICompare *_base, ICompare *_pcmp)
        {
            base = _base;
            pcmp = _pcmp;
        }

        int docompare(const void *l,const void *r) const
        {
            int ret = base->docompare(l,r);
            if (ret!=0)
                return ret;
            ret = pcmp->docompare(l,r);
            if (ret==1)         // this effectively checks if 1st char different
                return 1;
            if (ret==-1)
                return -1;
            return 0;
        }


    } *climitedcmp;
public:
    JoinActivityMaster(CMasterGraphElement * info, bool local) : CMasterActivity(info)
    {
        ActPrintLog("JoinActivityMaster");
        lhsProgress.setown(new ProgressInfo);
        rhsProgress.setown(new ProgressInfo);
        helper = NULL;
        islocal = local;
        imaster = NULL;
        selfJoinWarnLevel = INITIAL_SELFJOIN_MATCH_WARNING_LEVEL;
        lastMsgTime = 0;
        mpTagRPC = container.queryJob().allocateMPTag();
        barrier.setown(container.queryJob().createBarrier(container.queryJob().allocateMPTag()));
        climitedcmp = NULL;
    }
    ~JoinActivityMaster()
    {
        container.queryJob().freeMPTag(mpTagRPC);
        delete climitedcmp;
    }
    void init()
    {
    }
    void serializeSlaveData(MemoryBuffer &dst, unsigned slave)
    {
        if (!islocal)
        {
            dst.append((int)mpTagRPC);
            dst.append((int)barrier->queryTag());
        }
    }
    void preStart(size32_t parentExtractSz, const byte *parentExtract)
    {
        CMasterActivity::preStart(parentExtractSz, parentExtract);
        ActPrintLog("preStart");
        if (!islocal) {
            imaster = CreateThorSorterMaster(this);
            unsigned s=0;
            for (; s<container.queryJob().querySlaves(); s++)
            {
                SocketEndpoint ep;
                ep.deserialize(queryInitializationData(s));
                
                imaster->AddSlave(&container.queryJob().queryJobComm(), s+1,ep,mpTagRPC);
            }
        }
    }
    void process()
    {
        ActPrintLog("process");
        CMasterActivity::process();
        if (!islocal) {
            helper = (IHThorJoinArg *)queryHelper();
            StringBuffer skewV;
            double skewError;
            container.queryJob().getWorkUnitValue("overrideSkewError", skewV);
            if (skewV.length())
                skewError = atof(skewV.str());
            else
            {
                skewError = helper->getSkew();
                if (!skewError)
                {
                    container.queryJob().getWorkUnitValue("defaultSkewError", skewV.clear());
                    if (skewV.length())
                        skewError = atof(skewV.str());
                }
            }
            container.queryJob().getWorkUnitValue("defaultSkewWarning", skewV.clear());
            double defaultSkewWarning = skewV.length() ? atof(skewV.str()) : 0;
            double skewWarning = defaultSkewWarning;
            unsigned __int64 skewThreshold = container.queryJob().getWorkUnitValueInt("overrideSkewThreshold", 0);
            if (!skewThreshold)
            {
                skewThreshold = helper->getThreshold();         
                if (!skewThreshold)
                    skewThreshold = container.queryJob().getWorkUnitValueInt("defaultSkewThreshold", 0);
            }
            try
            {
                size32_t maxdeviance=globals->getPropInt("@sort_max_deviance", 10*1024*1024);
                rightpartition = (container.getKind() == TAKjoin)&&((helper->getJoinFlags()&JFpartitionright)!=0);
                bool betweenjoin = (helper->getJoinFlags()&JFslidingmatch)!=0;
                if (!container.queryLocalOrGrouped() && container.getKind() == TAKselfjoin)
                {
                    if (betweenjoin) {
                        throw MakeActivityException(this, -1, "SELF BETWEEN JOIN not supported"); // Gavin shouldn't generate
                    }
                    Owned<IRowInterfaces> rowif = createRowInterfaces(container.queryInput(0)->queryHelper()->queryOutputMeta(),queryActivityId(),queryCodeContext());
                    ICompare *cmpleft = helper->queryCompareLeft();
                    if ((helper->getJoinFlags()&JFlimitedprefixjoin)&&(helper->getJoinLimit())) {
                        delete climitedcmp;
                        climitedcmp = new cLimitedCmp(helper->queryCompareLeftRight(),helper->queryPrefixCompare());
                        cmpleft = climitedcmp;
                        // partition by L/R
                    }
                    imaster->SortSetup(rowif,cmpleft, helper->querySerializeLeft(), false, true, NULL, NULL);
                    if (barrier->wait(false)) { // local sort complete
                        if (!imaster->Sort(skewThreshold,skewWarning,skewError,maxdeviance,false,false,false,0)) {
                            Owned<IThorException> e = MakeActivityException(this, TE_JoinFailedSkewExceeded,"JOIN failed, skew exceeded (1)");
                            fireException(e);
                        }
                        ActPrintLog("JOIN waiting for barrier.1");
                        barrier->wait(false);
                        ActPrintLog("JOIN barrier.1 raised");
                    }
                    imaster->SortDone();
                }
                else if (!nosortPrimary()||betweenjoin) {
                    Owned<IRowInterfaces> rowif = createRowInterfaces(container.queryInput(rightpartition?1:0)->queryHelper()->queryOutputMeta(),queryActivityId(),queryCodeContext());
                    imaster->SortSetup(rowif,rightpartition?helper->queryCompareRight():helper->queryCompareLeft(),rightpartition?helper->querySerializeRight():helper->querySerializeLeft(),false,true, NULL, NULL);
                    ActPrintLog("JOIN waiting for barrier.1");
                    if (barrier->wait(false)) {
                        ActPrintLog("JOIN barrier.1 raised");
                        if (!imaster->Sort(skewThreshold,skewWarning,skewError,maxdeviance,false,false,false,0)) {
                            Owned<IThorException> e = MakeActivityException(this, TE_JoinFailedSkewExceeded,"JOIN failed, skew exceeded (2)");
                            fireException(e);
                        }
                        ActPrintLog("JOIN waiting for barrier.2");
                        if (barrier->wait(false)) { // merge complete
                            ActPrintLog("JOIN barrier.2 raised");
                            imaster->SortDone();
                            // NB on the cosort should use same serializer as sort (but in fact it only gets used when 0 rows on primary side)
                            Owned<IRowInterfaces> rowif2 = createRowInterfaces(container.queryInput(rightpartition?0:1)->queryHelper()->queryOutputMeta(),queryActivityId(),queryCodeContext());
                            imaster->SortSetup(rowif2,rightpartition?helper->queryCompareLeft():helper->queryCompareRight(),rightpartition?helper->querySerializeRight():helper->querySerializeLeft(),true,false, NULL, NULL); //serializers OK
                            ActPrintLog("JOIN waiting for barrier.3");
                            if (barrier->wait(false)) { // local sort complete
                                ActPrintLog("JOIN barrier.3 raised");
                                if (!imaster->Sort(skewThreshold,skewWarning,skewError,maxdeviance,false,nosortPrimary(),betweenjoin,0)) {
                                    Owned<IThorException> e = MakeActivityException(this, TE_JoinFailedSkewExceeded,"JOIN failed, skew exceeded (3)");
                                    fireException(e);
                                }
                                ActPrintLog("JOIN waiting for barrier.4");
                                barrier->wait(false); // merge complete
                                ActPrintLog("JOIN barrier.4 raised");
                            }
                        }
                        imaster->SortDone();
                    }
                }
                else { // only sort non-partition side
                    Owned<IRowInterfaces> rowif = createRowInterfaces(container.queryInput(rightpartition?0:1)->queryHelper()->queryOutputMeta(),queryActivityId(),queryCodeContext());
                    Owned<IRowInterfaces> auxrowif = createRowInterfaces(container.queryInput(rightpartition?1:0)->queryHelper()->queryOutputMeta(),queryActivityId(),queryCodeContext());
                    imaster->SortSetup(rowif,rightpartition?helper->queryCompareLeft():helper->queryCompareRight(),rightpartition?helper->querySerializeLeft():helper->querySerializeRight(),true,true, NULL, auxrowif);
                    ActPrintLog("JOIN waiting for barrier.1");
                    if (barrier->wait(false)) { // local sort complete
                        ActPrintLog("JOIN barrier.1 raised");
                        if (!imaster->Sort(skewThreshold,skewWarning,skewError,maxdeviance,false,nosortPrimary(),false,0)) {
                            Owned<IThorException> e = MakeActivityException(this, TE_JoinFailedSkewExceeded,"JOIN failed, skew exceeded (3)");
                            fireException(e);
                        }
                        ActPrintLog("JOIN waiting for barrier.2");
                        barrier->wait(false); // merge complete
                        ActPrintLog("JOIN barrier.2 raised");
                    }
                    imaster->SortDone();
                }
            }
            catch (IMP_Exception *e)
            {
                if (e->errorCode()!=MPERR_link_closed) 
                    throw;
                ActPrintLogEx(&queryContainer(), thorlog_null, MCwarning, "WARNING: MPERR_link_closed in SortDone");
                e->Release();
            }
            ::Release(imaster);
        }
        ActPrintLog("process exit");
    }
    void deserializeStats(unsigned node, MemoryBuffer &mb)
    {
        CMasterActivity::deserializeStats(node, mb);
        rowcount_t lhsProgressCount, rhsProgressCount;
        mb.read(lhsProgressCount);
        lhsProgress->set(node, lhsProgressCount);
        if (TAKselfjoin != container.getKind() && TAKselfjoinlight != container.getKind())
        {
            mb.read(rhsProgressCount);
            rhsProgress->set(node, rhsProgressCount);
        }
    }
    void getXGMML(unsigned idx, IPropertyTree *edge)
    {
        CMasterActivity::getXGMML(idx, edge);

        assertex(0 == idx);
        StringBuffer label;
        lhsProgress->processInfo();
        label.append("@progressInput-").append(container.queryInput(0)->queryId());
        edge->setPropInt64(label.str(), lhsProgress->queryTotal());

        if (TAKselfjoin != container.getKind() && TAKselfjoinlight != container.getKind())
        {
            rhsProgress->processInfo();
            label.clear().append("@progressInput-").append(container.queryInput(1)->queryId());
            edge->setPropInt64(label.str(), rhsProgress->queryTotal());
        }
    }
#define MSGTIME (5*60*1000)
    virtual bool fireException(IException *_e)
    {
        IThorException *e = QUERYINTERFACE(_e, IThorException);
        if (NULL != e && (TAKselfjoin == container.getKind() || TAKselfjoinlight == container.getKind()) && TE_SelfJoinMatchWarning == e->errorCode())
        {
            // Output these warning ever if not reported since MSGTIME
            // OR if count is > than scaled threshold.
            unsigned count;
            e->queryData().read(count);
            if (count >= selfJoinWarnLevel)
            {
                selfJoinWarnLevel *= 2;
                Owned<IException> e = MakeActivityWarning(this, -1, "SELFJOIN: Warning %d preliminary matches, join will take some time", count);
                CMasterActivity::fireException(e);
                lastMsgTime = msTick();
            }
            else if (msTick() > lastMsgTime + MSGTIME)
            {
                Owned<IException> e = MakeActivityWarning(this, -1, "SELFJOIN: Warning %d preliminary matches, join will take some time", count);
                CMasterActivity::fireException(e);
                lastMsgTime = msTick();
            }
            return true;
        }
        return CMasterActivity::fireException(e);
    }
};


CActivityBase *createJoinActivityMaster(CMasterGraphElement *container)
{
    return new JoinActivityMaster(container, container->queryLocalOrGrouped());
}



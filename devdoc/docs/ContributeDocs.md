# Contributing Documentation to the HPCC Systems Platform Project

This document is intended for anyone that wants to contribute
documentation to our project. The first audience is platform developers,
so we can streamline the process of documenting new features. However,
these guidelines apply to anyone who wants to contribute to any of our
documentation (Language Reference, Programmer’s Guide, etc.).

This set of guidelines should help you understand the information needed
to create sufficient documentation for any new feature you add. The good
news is that we are all here to help you and support your writing
efforts. We will help by advising you along the way, and by reviewing
and editing your submissions.

## Documenting a New Software Feature--Required and Optional Components

When you create a new feature or function, clear documentation is
crucial for both internal teams and external users. You worked hard on
the feature, so it deserves proper notice and usage.

We are an open-source project, so contributions are always welcome,
including documentation.

You can contribute on many levels:

1.  Developer Notes

2.  End user “Readmes” in the form of MD files in the GitHub repository

3.  Blogs

4.  Formal documentation

Regardless of the form you are planning to deliver, here are the
required and optional components to include in a document.

> **Tip**: VS Code is very good at editing MD files. There is a Preview
> panel available to be able to see the rendered form.
>
> In addition, GitHub Copilot is MD-aware and can help you write and
> format. For example, you can ask the Copilot, “How can I align the
> content within the cells of my Markdown table?” GitHub copilot will
> show you the alignment options.

### Required Components:

1.  **Overview**

    -   **What it is:** Briefly describe the feature's purpose and the
        problem it solves.

    -   **Why it matters:** Explain the value proposition for users and
        the overall impact on the software.

    -   **Target audience:** Specify who this feature is designed for
        (for example, all users or specific user roles).

    -   **Use Cases:** Provide concrete examples of how a user might
        leverage this feature in a real-world scenario.

2.  **Installation and Configuration:** Details on how to install and
    basic setup for use, if needed. This must include any system
    requirements or dependencies.

3.  **User Guide / Functionality**

    -   **How it works:** Provide a task-oriented, step-by-step guide
        for using the feature, including screenshots for visual
        learners.

    -   **Tips, Tricks, and Techniques:** Explain any shortcuts or
        clever uses of the feature that may be non-obvious.

    -   **Inputs and Outputs:** Detail the information users need to
        provide to the feature and the format of the results.

    -   **Error Handling:** Explain what happens if users encounter
        errors and how to troubleshoot common issues.

4.  **Limitations and Considerations:**

    -   **Limitations:** Acknowledge any restrictions or boundaries
        associated with the feature's functionality.

### Optional Components:

1.  **Advanced Usage**

    -   **Detailed configuration options:** If the feature offers
        advanced settings or customizations, provide in-depth
        instructions for experienced users.

2.  **API Reference (for technical audiences)**

    -   **Technical specifications:** For features with an API
        component, include detailed API reference documentation for
        developers integrating it into their applications.

3.  **FAQs**

    -   **Frequently Asked Questions:** Address any commonly anticipated
        user questions about the feature to pre-empt confusion.

4.  **Additional Resources**

    -   **Links to related documentation:** Include links to relevant
        documentation for features that interact with this new addition.

    -   **Tutorials:** Consider creating tutorials for a more
        interactive learning experience.

    -   **Videos:** Consider requesting that a video be made to provide
        a more interactive visual learning experience. You should
        provide a simple script or outline of what should be shown in
        the video.

### General Tips

-   **Target your audience:** Tailor the level of detail and technical
    jargon you use based on whether the documentation is for developers
    or end-users.

-   **Clarity and Conciseness:** Use clear, concise language and
    maintain a consistent structure for easy navigation. Always use
    present tense, active voice. Remember, you’re writing for the
    open-source community, not academics, so keep it simple and
    straight-forward. See the HPCC Style Guide for additional guidance.

-   **Visual Aids:** Screenshots, diagrams, and flowcharts can
    significantly enhance understanding. A picture can communicate
    instantly what a thousand words cannot.

-   **Maintain and Update:** Regularly review and update documentation
    as the feature evolves or based on user feedback.

By following these guidelines and including the required and optional
components, you can create comprehensive documentation that empowers
users and streamlines the adoption of your new software feature.

### Placement

In general, it makes sense to keep your documentation near the code. For
example, a document about ECL Agent should go in the ECLAgent folder.

However, there are times where that is either not possible or a document
may cover more than one component. In those cases, there are a few
options as shown below.

#### Other Folders

**DevDocs**: This is a general folder for any developer document.

**Docs**: This is a folder for documents about documentation.

**Userdoc**: This is a collection of docs targeted toward the end-user
rather than developers.

**Userdoc**/**troubleshoot**: Information related to troubleshooting
particular components

**userdoc**/**azure**: Useful information about Azure Cloud portal and
cli

**userdoc**/**roxie**: Useful information for running roxie

**userdoc**/**thor**: Useful information for running thor

**userdoc**/**blogs**: COMING SOON: Location and storage of original
text for blogs. It also has docs with guidelines and instructions on
writing Blogs

#### Pull Requests

You can include your documentation with your code in a Pull Request or
create a separate Jira and Pull Request for the documentation. This
depends on the size of the code and doc. For a large project, a separate
Pull request for the documentation is better.

# Writing Style Guide

This section covers the best practice information for writing and
contributing to the HPCC Systems® Platform documentation.

## General 

We strive to maintain a consistent voice. These guidelines can help your
writing match that voice.

-   **Use present tense, active voice**. Documentation should be you
    speaking directly to the reader. Simply tell them what to do.

    This example sentence: "The user selects the file menu." is passive
    voice. You wouldn’t say it that way in a conversation.

    You should use active voice wording, such as: "Select the file
    menu".

    Similarly, instructions like these are active voice: "Press the
    button" or "Submit the file".

    Documentation is you instructing the user. Just tell them what to
    do.

-   **Be Brief and keep it simple.** Be efficient with your words. Keep
    sentences short and concise. Keep paragraphs short as well. Use just
    a few sentences per paragraph. Use simple words wherever possible
    and try to avoid lengthy explanations.

-   **Consistency:** Be consistent. Use the same voice across all
    documents. Use the same term, use the same spelling, punctuation,
    etc. Follow the conventions in this guide.

-   When writing formal documentation that is more than a new feature
    announcement, do not refer to a new feature, or coming features as
    such, this does not hold up well over time.

    Do not refer to how things were done in the past. For example, "in
    the past we had to do X-Y-Z steps and now we no longer have to. This
    ‘new feature’ can do it in one step, Z". This only adds potential
    confusion. Just instruct on exactly what needs to be done now, using
    words efficiently as possible, so for this example just say “perform
    step Z”

## Terms 

There are many terms specific to HPCC Systems® and the HPCC Systems
platform. Use the following style guide for word usage and
capitalization guidelines to use when referring to system components or
other HPCC-specific entities. Maintain consistent usage throughout all
docs.

### HPCC Systems® 

Officially and legally the platform's name is HPCC Systems® and it is a
registered trademark.

You should always refer to the platform as HPCC Systems® and the
registration mark ® should appear in the first and most prominent
mention of the name HPCC Systems®.

While it is acceptable to use the ® anywhere in a document, it is
required to be used in the first and most prominent mention - so the
average reader will be aware. Any usage after that first and most
prominent is optional.

**Components and Tools**

-   HPCC Systems Platform

-   Dali

-   Sasha

-   Thor

-   hThor

-   ROXIE

-   DFU Server (Distributed File Utility Server)

-   ESP Server (Enterprise Server Platform)

-   ESP Services

-   WsECL

-   ECL Watch

-   ECL Server

-   ECLCC Server

-   ECL Agent

-   ECL IDE

-   ECL Plug-in for Eclipse

-   ECL Playground

-   LDAP

-   dafilesrv

-   VS Code (no hyphen)

-   ECL Language Extension for VS Code

-   Configuration Manager (not ConfigMgr or ConfigManager)

**Note:** when referring to the startup command, use configmgr (always
lowercase)

### Other Terms 

-   HPCCSystems.com or [http://HPCCSystems.com](http://hpccsystems.com/)
    (do not include the www portion)

-   ECL (Enterprise Control Language)

-   ECL command-line interface

**Note:** when referring to the ECL command-line tool command, ecl is
lowercase

-   DFU Plus command-line interface

<!-- -->

-   cloud-native (always hyphenated when used as an adjective)

<!-- -->

-   DFU Workunits

-   ECL Workunits

-   Workunit

-   WUID

-   HPCC Systems®

-   multi-node

-   Superfiles, subfiles

-   package map

-   The **username** is the (usually unique) thing you type in with your
    password, for example: bobsmith66.

-   The **user name** is the name of the user, the user's real-life
    name, for example: Bob Smith.

###  Common Documentation terms 

Use the following conventions for these commonly used terms:

-   right-click

-   double-click

-   drag-and-drop

-   click-and-drag

-   plug-in

-   drop list

-   bare metal (no hyphen)

-   blue-green (not Blue/Green)

-   Common Vulnerabilities and Exposures (CVEs)

###  Usage Instructions 

-   You **click** a link.

-   You **select** a tab.

-   You **press** a button.

-   You **check** a (check)box.

### Word Choices 

#### Write up vs Write-up 

Hyphenated when used as a noun. No Hyphen when used as a verb phrase.

Examples:

-   Did you read the write-up?

-   Would you write up the steps to reproduce?

#### Assure vs ensure vs insure 

To “assure” a person of something is to make him or her confident of it.

According to the Associated Press style, to “ensure” that something
happens is to make certain that it does, and to “insure” is to issue an
insurance policy.

Other authorities, however, consider “ensure” and “insure”
interchangeable. We prefer ensure when it is not talking about
insurance.

If you have any questions, feel free to contact us at
<docfeedback@hpccsystems.com>

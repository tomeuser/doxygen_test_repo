#pragma once

class buildInfo {
public:
    static const char *firmwareVersion;
    static const char *interfaceVersion;
    static const char *gitLastCommitHash;
    static const char *gitBranch;
    static const char *buildEnvironment;
    static const char *buildTimeStamp;
};
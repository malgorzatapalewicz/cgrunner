#include "CgroupManager.h"
#include "ProcessRunner.h"
#include "ArgumentParser.h"
#include <iostream>
#include <sys/wait.h>

using namespace CgroupManager;
using namespace ArgumentParser;
using namespace ErrorUtils;

void checkRootPrivileges() {
    if (geteuid() != 0) {
        reportUserError("This program requires root privileges to manipulate cgroups.");
        exit(1);
    }
}

void validateExecCommand(char* argv[]) {
    if (std::string(argv[1]) != "exec") {
        reportUserError("Usage: ./cgrunner exec --cpu <cpu_percent> --memory <memory_bytes> \"<program> [args...]\"");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    checkRootPrivileges();
    validateExecCommand(argv);

    argc -= 1;
    argv += 1;

    int cpuPercent = 0;
    size_t memoryBytes = 0;
    std::string commandLine;
    bool cpuLimitProvided = false, memoryLimitProvided = false;

    parseArguments(argc, argv, cpuLimitProvided, memoryLimitProvided, cpuPercent, memoryBytes);
    validateProvidedLimits(cpuLimitProvided, memoryLimitProvided);
    std::string groupName = generateCgroupName(cpuPercent, memoryBytes);

    if (!isCgroupInitialized(groupName, cpuPercent, memoryBytes)) {
        return 1;
    }

    runProgramInCgroup(argc, argv, groupName);
    cleanUpCgroup(groupName);

    return 0;
}
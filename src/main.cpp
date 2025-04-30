#include "CgroupManager.h"
#include "ProcessRunner.h"
#include "ArgumentParser.h"
#include <iostream>
#include <cstring>
#include <getopt.h>
#include <sys/wait.h>

using namespace ProcessRunner;
using namespace CgroupManager;
using namespace ArgumentParser;
using namespace ErrorUtils;

void checkRootPrivileges() {
    if (geteuid() != 0) {
        reportUserError("This program requires root privileges to manipulate cgroups.");
        exit(1);
    }
}

void validateExecCommand(char *argv[]) {
    if (std::string(argv[1]) != "exec") {
        reportUserError("Usage: ./cgrunner exec --cpu <cpu_percent> --memory <memory_bytes> \"<program> [args...]\"");
        exit(1);
    }
}

bool createCgroup(const std::string &groupName, int cpuPercent, size_t memoryBytes) {
    deleteCgroup(groupName);
    if (!createGroup(groupName)) {
        reportSystemError("Failed to create cgroup " + groupName, errno);
        return false;
    }

    if (!setCpuLimit(groupName, cpuPercent) || !setMemoryLimit(groupName, memoryBytes)) {
        reportSystemError("Failed to set CPU or memory limits", errno);
        return false;
    }

    return true;
}

void runProgramInCgroup(int argc, char *argv[], const std::string &groupName) {
    pid_t pid = fork();
    if (pid == 0) {
        if (!addProcessToCgroup(groupName, getpid())) {
            reportSystemError("Failed to add process to cgroup " + groupName, errno);
            _exit(1);
        }

        std::string commandLine;
        for (int i = optind; i < argc; ++i) {
            if (i > optind) commandLine += " ";
            commandLine += argv[i];
        }

        runProgram(commandLine);
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        reportSystemError("Fork failed", errno);
    }
}

void cleanUpCgroup(const std::string &groupName) {
    if (!deleteCgroup(groupName)) {
        reportSystemError("Failed to delete cgroup " + groupName, errno);
    }
}

int main(int argc, char *argv[]) {
    checkRootPrivileges();
    validateExecCommand(argv);

    argc -= 1;
    argv += 1;

    int cpuPercent = 0;
    size_t memoryBytes = 0;
    std::string commandLine;
    bool cpuLimitProvided = false, memoryLimitProvided = false;

    parseArguments(argc, argv, cpuLimitProvided, memoryLimitProvided, cpuPercent, memoryBytes);

    if (!cpuLimitProvided || !memoryLimitProvided) {
        reportUserError("Both CPU and memory limits must be specified.");
        return 1;
    }

    std::string groupName = "cgroup-" + std::to_string(cpuPercent) + "-" + std::to_string(memoryBytes);

    if (!createCgroup(groupName, cpuPercent, memoryBytes)) return 1;
    runProgramInCgroup(argc, argv, groupName);
    cleanUpCgroup(groupName);
    return 0;
}
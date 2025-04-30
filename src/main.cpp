#include "CgroupManager.h"
#include "ProcessRunner.h"
#include <iostream>
#include <cstring>
#include <getopt.h>
#include <sys/wait.h>

using namespace ProcessRunner;
using namespace CgroupManager;
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

bool startsWithDash(const char *arg) {
    return arg[0] == '-';
}

template <typename T>
bool isProperLimit(const char *arg, T &limitValue, const char *limitType) {
    if (startsWithDash(arg)) {
        reportUserError(std::string(limitType) + " limit must be a positive number.");
        return false;
    }

    try {
        if constexpr (std::is_same_v<T, int>) {
            limitValue = std::stoi(arg);
        } else if constexpr (std::is_same_v<T, size_t>) {
            limitValue = std::stoul(arg);
        }
        return true;
    } catch (const std::invalid_argument &) {
        reportUserError(std::string(limitType) + " limit must be a valid number.");
    } catch (const std::out_of_range &) {
        reportUserError(std::string(limitType) + " limit value is out of range.");
    }
    return false;
}

bool isCpuLimitExceeded(const char *arg, int &cpuPercent) {
    if (isProperLimit(arg, cpuPercent, "CPU")) {
        if (cpuPercent > 0 && cpuPercent <= 100) return true;
        reportUserError("CPU limit must be a number between 1 and 100.");
    }

    return false;
}

bool isMemoryLimitValid(const char *arg, size_t &memoryBytes) {
    if (isProperLimit(arg, memoryBytes, "Memory")) {
        if (memoryBytes > 0) return true;
        reportUserError("Memory limit must be greater than 0.");
    }

    return false;
}

void parseArguments(int argc, char *argv[], bool &cpuSet, bool &memorySet, int &cpuPercent, size_t &memoryBytes) {
    static struct option options[] = {
        {"cpu", required_argument, nullptr, 'c'},
        {"memory", required_argument, nullptr, 'm'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "c:m:", options, nullptr)) != -1) {
        switch (opt) {
            case 'c':
                cpuSet = isCpuLimitExceeded(optarg, cpuPercent);
                if (!cpuSet) return;
                break;
            case 'm':
                memorySet = isMemoryLimitValid(optarg, memoryBytes);
                if (!memorySet) return;
                break;
            default:
                reportUserError("Invalid option.");
                return;
        }
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
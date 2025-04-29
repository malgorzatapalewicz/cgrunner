#include "CgroupManager.h"
#include "ProcessRunner.h"
#include <iostream>
#include <cstring>
#include <getopt.h>
#include <sys/wait.h>

using namespace ProcessRunner;
using namespace CgroupManager;
using namespace ErrorUtils;

namespace {

void checkRootPrivileges() {
    if (geteuid() != 0) {
        reportUserError("This program requires root privileges to manipulate cgroups.");
    }
}

void validateSubcommand(int argc, char* argv[]) {
    if (argc < 2 || std::string(argv[1]) != "exec") {
        throw std::invalid_argument("Usage: ./cgrunner exec --cpu <cpu_percent> --memory <memory_bytes> \"<program> [args...]\"");
    }

    //shift arguments to start from the CPU and memory flags
    argc -= 1;
    argv += 1;
}


struct ResourceLimits {
    int cpuPercent = 0;
    size_t memoryBytes = 0;
};

void parseArguments(int argc, char* argv[], ResourceLimits& limits, int& commandIndex) {
    static struct option long_options[] = {
        {"cpu", required_argument, nullptr, 'c'},
        {"memory", required_argument, nullptr, 'm'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    bool cpuSet = false, memorySet = false;

    while ((opt = getopt_long(argc, argv, "c:m:", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'c':
                limits.cpuPercent = std::stoi(optarg);
                if (limits.cpuPercent <= 0 || limits.cpuPercent > 100) {
                    throw std::invalid_argument("CPU limit must be a number between 1 and 100.");
                }
                cpuSet = true;
                break;
            case 'm':
                limits.memoryBytes = std::stoul(optarg);
                if (limits.memoryBytes == 0) {
                    throw std::invalid_argument("Memory limit must be greater than 0.");
                }
                memorySet = true;
                break;
            default:
                throw std::invalid_argument("Invalid option.");
        }
    }

    if (!cpuSet || !memorySet) {
        throw std::invalid_argument("Both CPU and memory limits must be specified.");
    }

    commandIndex = optind;
}

std::string buildCommand(int argc, char* argv[], int startIndex) {
    std::string command;
    for (int i = startIndex; i < argc; ++i) {
        if (i > startIndex) command += " ";
        command += argv[i];
    }
    return command;
}

std::string generateGroupName(const ResourceLimits& limits) {
    return "cgroup-" + std::to_string(limits.cpuPercent) + "-" + std::to_string(limits.memoryBytes);
}

int handleSystemError(const std::string& msg) {
    reportSystemError(msg, errno);
    return 1;
}

bool setupCgroup(const std::string& groupName, const ResourceLimits& limits) {
    deleteCgroup(groupName);  // cleanup before creating

    if (!createGroup(groupName)) return false;
    if (!setCpuLimit(groupName, limits.cpuPercent)) return false;
    if (!setMemoryLimit(groupName, limits.memoryBytes)) return false;

    return true;
}

int runProcessWithinCgroup(const std::string& groupName, const std::string& command) {
    pid_t pid = fork();

    if (pid == 0) {
        if (!addProcessToCgroup(groupName, getpid())) {
            reportSystemError("Failed to add process to cgroup " + groupName, errno);
            _exit(1);
        }
        runProgram(command);
        _exit(1);  // only reached if exec failed

    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return status;

    } else {
        throw std::runtime_error("Fork failed");
    }
}

int getExitCodeFromStatus(int status) {
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}

} 


int main(int argc, char* argv[]) {
    try {
        checkRootPrivileges();
        validateSubcommand(argc, argv); 

        ResourceLimits limits;
        int commandIndex = 0;

        parseArguments(argc, argv, limits, commandIndex);
        std::string command = buildCommand(argc, argv, commandIndex);
        std::string groupName = generateGroupName(limits);

        if (!setupCgroup(groupName, limits)) return handleSystemError("Failed to create or configure cgroup");
        int status = runProcessWithinCgroup(groupName, command);
        if (!deleteCgroup(groupName)) return handleSystemError("Failed to delete cgroup " + groupName);

        return getExitCodeFromStatus(status);

    } catch (const std::invalid_argument& e) {
        reportUserError(e.what());

    } catch (const std::runtime_error& e) {
        reportSystemError(e.what(), errno);
    }
    return 1;
}
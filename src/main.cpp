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

bool parseCpuLimit(const char* arg, int& cpuPercent) {
    try {
        if (arg[0] == '-') return false;
        cpuPercent = std::stoi(arg);
        return cpuPercent > 0 && cpuPercent <= 100;
    } catch (...) {
        return false;
    }
}

bool parseMemoryLimit(const char* arg, size_t& memoryBytes) {
    try {
        if (arg[0] == '-') return false;
        memoryBytes = std::stoul(arg);
        return memoryBytes > 0;
    } catch (...) {
        return false;
    }
}

int main(int argc, char* argv[]) {

    checkRootPrivileges();

    if (argc < 2 || std::string(argv[1]) != "exec") {
        reportUserError("Usage: ./cgrunner exec --cpu <cpu_percent> --memory <memory_bytes> \"<program> [args...]\"");
        return 1;
    }

    argc -= 1;
    argv += 1;

    int cpuPercent = 0;
    size_t memoryBytes = 0;
    std::string commandLine;
    bool cpuSet = false, memorySet = false;


    static struct option long_options[] = {
        {"cpu", required_argument, nullptr, 'c'},
        {"memory", required_argument, nullptr, 'm'},
        {nullptr, 0, nullptr, 0}  
    };

    int opt;
while ((opt = getopt_long(argc, argv, "c:m:", long_options, nullptr)) != -1) {
    switch (opt) {
        case 'c':
            try {
                cpuPercent = std::stoi(optarg);
                if (cpuPercent <= 0 || cpuPercent > 100) {
                    reportUserError("CPU limit must be a number between 1 and 100.");
                    return 1;
                }
                cpuSet = true;
            } catch (const std::exception& e) {
                reportUserError("Invalid CPU limit");
                return 1;
            }
            break;
        case 'm':
            try {
                memoryBytes = std::stoul(optarg);
                if (memoryBytes == 0) {
                    reportUserError("Memory limit must be greater than 0.");
                    return 1;
                }
                memorySet = true;
            } catch (const std::exception& e) {
                reportUserError("Invalid memory limit");
                return 1;
            }
            break;
        default:
            reportUserError("Invalid option.");
            return 1;
    }
}


    if (!cpuSet || !memorySet) {
        reportUserError("Both CPU and memory limits must be specified.");
        return 1;
    }

    std::string groupName = "cgroup-" + std::to_string(cpuPercent) + "-" + std::to_string(memoryBytes);
    
    deleteCgroup(groupName);
    if (!createGroup(groupName)){
        reportSystemError("Failed to create cgroup " + groupName, errno);
        return 1;
    }


    if (!setCpuLimit(groupName, cpuPercent) || !setMemoryLimit(groupName, memoryBytes)) {
        reportSystemError("Failed to set CPU or memory limits", errno);
        return 1;
    }


    pid_t pid = fork();
    if (pid == 0) {  
        if (!addProcessToCgroup(groupName, getpid())){
            reportSystemError("Failed to add process to cgroup " + groupName, errno);
            _exit(1);
        };

        //std::string commandLineWithArgs = std::string(argv[optind]); 
        std::string commandLineWithArgs;
        for (int i = optind; i < argc; ++i) {
        if (i > optind) commandLineWithArgs += " ";
        commandLineWithArgs += argv[i];
} 
        runProgram(commandLineWithArgs);
        _exit(1);

    } else if (pid > 0) {  
        int status;
        waitpid(pid, &status, 0);  
        if (!deleteCgroup(groupName)){
            reportSystemError("Failed to delete cgroup " + groupName, errno);
            return 1;
        }

    } else {
        reportSystemError("Fork failed", errno);
        return 1;
    }

    return 0;
}

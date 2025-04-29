#include "CgroupManager.h"
#include "ErrorUtils.h"

#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

namespace CgroupManager {

    const mode_t DIRECTORY_PERMISSIONS = 0755;
    const int CPU_PERIOD = 100000; //[ms]
    const std::string CGROUP_BASE_PATH = "/sys/fs/cgroup/";

    std::string buildCgroupPath(const std::string& groupName, const std::string& file){
        return CGROUP_BASE_PATH + groupName + (file.empty() ? "" : "/" + file);
    }

    bool createGroup(const std::string& groupName){
        std::string cgroupPath = buildCgroupPath(groupName);
            
        if(mkdir(cgroupPath.c_str(), DIRECTORY_PERMISSIONS) == -1){ 
            ErrorUtils::reportError("Error creating cgroup", errno);
            return false;
        }
        return true;
    }

    bool setCpuLimit(const std::string& groupName, int cpuPercent) {
        std::string cpuLimitPath = buildCgroupPath(groupName, "cpu.max");
        return writeToFile(cpuLimitPath, cpuPercent * 1000);
    }

    bool setMemoryLimit(const std::string& groupName, size_t memoryBytes) {
        std::string memoryLimitPath = buildCgroupPath(groupName, "memory.max");
        return writeToFile(memoryLimitPath, memoryBytes);
    }

    bool addProcessToCgroup(const std::string& groupName, pid_t pid){
        std::string cgroupProcsPath = buildCgroupPath(groupName, "cgroup.procs");
        return writeToFile(cgroupProcsPath, pid);
    }
    
    bool deleteCgroup(const std::string& groupName) {
        std::string cgroupPath = buildCgroupPath(groupName);

        if (rmdir(cgroupPath.c_str()) == -1) {
            ErrorUtils::reportError("Error removing cgroup", errno);
            return false;
        }

        std::cout << "Cgroup deleted: " << cgroupPath << std::endl;
        return true;
    }

}
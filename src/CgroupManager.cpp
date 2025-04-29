#include "CgroupManager.h"

#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>


namespace CgroupManager {

    static const mode_t DIRECTORY_PERMISSIONS = 0755;
    static const int CPU_PERIOD = 100000; //[ms]
    static const std::string CGROUP_BASE_PATH = "/sys/fs/cgroup/";

    template <typename T>
    static bool writeToFile(const std::string& filePath, const T& content){
    std::ofstream file(filePath);
    if(!file){
        reportSystemError("Error opening file", errno);
        return false;
    }
    file << content;
    file.close();
    return true;
}

    static std::string buildCgroupPath(const std::string& groupName, const std::string& file = ""){
        return CGROUP_BASE_PATH + groupName + (file.empty() ? "" : "/" + file);
    }

    bool createGroup(const std::string& groupName){
        std::string cgroupPath = buildCgroupPath(groupName);
            
        if(mkdir(cgroupPath.c_str(), DIRECTORY_PERMISSIONS) == -1){ 
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
            return false;
        }

        std::cout << "Cgroup deleted: " << cgroupPath << std::endl;
        return true;
    }

}
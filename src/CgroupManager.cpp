#include "CgroupManager.h"
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <fstream>
#include <unistd.h>


namespace CgroupManager {

    const mode_t DIRECTORY_PERMISSIONS = 0755;
    const int CPU_PERIOD = 100000; //[ms]

    void reportError(const std::string& message, int errorCode){
        std::cerr << message << ": " << strerror(errorCode) << std::endl;
    }

  
    bool createGroup(const std::string& groupName){
        std::string cgroupPath = "/sys/fs/cgroup/" + groupName;
            
        if(mkdir(cgroupPath.c_str(), DIRECTORY_PERMISSIONS) == -1){ 
            reportError("Error creating cgroup", errno);
            return false;
        }
        return true;
    }

    bool setCpuLimit(const std::string& groupName, int cpuPercent) {
        std::string cpuLimitPath = "/sys/fs/cgroup/" + groupName + "/cpu.max";
        std::ofstream file(cpuLimitPath);

        if (!file.is_open()) {
            reportError("Error opening cpu.max", errno);
            return false;
        }

        file << cpuPercent * 1000 << " " << CPU_PERIOD;  
        file.close();
        return true;
    }

    bool setMemoryLimit(const std::string& groupName, size_t memoryBytes) {
        std::string memoryLimitPath = "/sys/fs/cgroup/" + groupName + "/memory.max";
        std::ofstream file(memoryLimitPath);

        if (!file.is_open()) {
            reportError("Error opening memory.max", errno);
            return false;
        }

        file << memoryBytes;
        file.close();
        return true;
    }

    bool addProcessToCgroup(const std::string& groupName, pid_t pid) {
        std::string cgroupProcsPath = "/sys/fs/cgroup/" + groupName + "/cgroup.procs";
        std::ofstream file(cgroupProcsPath);

        if (!file.is_open()) {
            reportError("Error opening cgroup.procs", errno);
            return false;
        }

        file << pid;  
        file.close();
        return true;
    }
    
    bool deleteCgroup(const std::string& groupName) {
        std::string cgroupPath = "/sys/fs/cgroup/" + groupName;

        if (rmdir(cgroupPath.c_str()) == -1) {
            reportError("Error removing cgroup", errno);
            return false;
        }

        std::cout << "Cgroup deleted: " << cgroupPath << std::endl;
        return true;
    }

}
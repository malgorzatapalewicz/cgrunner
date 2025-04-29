#ifndef CGROUPMANAGER_H
#define CGROUPMANAGER_H

#include "ErrorUtils.h"
#include <string>
#include <sys/types.h> 
#include <fstream>

using namespace ErrorUtils;

namespace CgroupManager {

    extern const mode_t DIRECTORY_PERMISSIONS;
    extern const int CPU_PERIOD;
    extern const std::string CGROUP_BASE_PATH;

    template <typename T>
    bool writeToFile(const std::string& filePath, const T& content){
    std::ofstream file(filePath);
    if(!file){
        reportError("Error opening file", errno);
        return false;
    }
    file << content;
    file.close();
    return true;
}


    std::string buildCgroupPath(const std::string& groupName, const std::string& file = "");
    bool createGroup(const std::string& groupName);
    bool setCpuLimit(const std::string& groupName, int cpuPercent);
    bool setMemoryLimit(const std::string& groupName, size_t memoryBytes);
    bool addProcessToCgroup(const std::string& groupName, pid_t pid);
    bool deleteCgroup(const std::string& groupName);

}

#endif 
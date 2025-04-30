#ifndef CGROUPMANAGER_H
#define CGROUPMANAGER_H

#include "ErrorUtils.h"
#include <string>
#include <sys/types.h>
#include <fstream>

using namespace ErrorUtils;

namespace CgroupManager {
    bool createGroup(const std::string &groupName);
    bool setCpuLimit(const std::string &groupName, int cpuPercent);
    bool setMemoryLimit(const std::string &groupName, size_t memoryBytes);
    bool addProcessToCgroup(const std::string &groupName, pid_t pid);
    bool deleteCgroup(const std::string &groupName);
}

#endif
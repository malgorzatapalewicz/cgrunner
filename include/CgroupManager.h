#ifndef CGROUPMANAGER_H
#define CGROUPMANAGER_H

#include "ErrorUtils.h"
#include <string>
#include <sys/types.h>
#include <fstream>

using namespace ErrorUtils;

namespace CgroupManager
{
    std::string generateCgroupName(int cpuPercent, size_t memoryBytes);
    bool isCgroupInitialized(const std::string &groupName, int cpuPercent, size_t memoryBytes);
    void runProgramInCgroup(int argc, char *argv[], const std::string &groupName);
    void cleanUpCgroup(const std::string &groupName);
}

#endif
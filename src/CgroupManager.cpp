#include "CgroupManager.h"
#include "ProcessRunner.h"

#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace ProcessRunner;

namespace CgroupManager {

    static const mode_t DIRECTORY_PERMISSIONS = 0755;
    static const int CPU_PERIOD = 100000; // [ms]
    static const std::string CGROUP_BASE_PATH = "/sys/fs/cgroup/";

    template <typename T>
    static bool writeToFile(const std::string& filePath, const T& content) {
        std::ofstream file(filePath);
        if (!file) {
            reportSystemError("Error opening file", errno);
            return false;
        }
        file << content;
        file.close();
        return true;
    }

    static std::string buildCgroupPath(const std::string& groupName, const std::string& file = "") {
        return CGROUP_BASE_PATH + groupName + (file.empty() ? "" : "/" + file);
    }

    static bool createCgroup(const std::string& groupName) {
        return mkdir(buildCgroupPath(groupName).c_str(), DIRECTORY_PERMISSIONS) != -1;
    }

    static bool setCpuLimit(const std::string& groupName, int cpuPercent) {
        return writeToFile(buildCgroupPath(groupName, "cpu.max"), cpuPercent * 1000);
    }

    static bool setMemoryLimit(const std::string& groupName, size_t memoryBytes) {
        return writeToFile(buildCgroupPath(groupName, "memory.max"), memoryBytes);
    }

    static bool addProcessToCgroup(const std::string& groupName, pid_t pid) {
        return writeToFile(buildCgroupPath(groupName, "cgroup.procs"), pid);
    }

    static bool deleteCgroup(const std::string& groupName) {
        return rmdir(buildCgroupPath(groupName).c_str()) == 0;
    }

    std::string generateCgroupName(int cpuPercent, size_t memoryBytes) {
        return "cgroup-" + std::to_string(cpuPercent) + "-" + std::to_string(memoryBytes);
    }

    bool isCgroupInitialized(const std::string& groupName, int cpuPercent, size_t memoryBytes) {
        if (!createCgroup(groupName)) {
            reportSystemError("Failed to create cgroup " + groupName, errno);
            return false;
        }

        if (!setCpuLimit(groupName, cpuPercent) || !setMemoryLimit(groupName, memoryBytes)) {
            reportSystemError("Failed to set CPU or memory limits", errno);
            return false;
        }

        return true;
    }

    static std::string buildCommandLine(int argc, char* argv[]) {
        std::string result;
        for (int i = optind; i < argc; ++i) {
            if (i > optind) result += " ";
            result += argv[i];
        }
        return result;
    }

    static void executeInCgroup(const std::string& groupName, int argc, char* argv[]) {
        if (!addProcessToCgroup(groupName, getpid())) {
            reportSystemError("Failed to add process to cgroup " + groupName, errno);
            _exit(1);
        }

        std::string commandLine = buildCommandLine(argc, argv);
        runProgram(commandLine);
        _exit(1);
    }

    void runProgramInCgroup(int argc, char* argv[], const std::string& groupName) {
        pid_t pid = fork();

        if (pid == 0) {
            executeInCgroup(groupName, argc, argv);
        }
        else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
        }
        else {
            reportSystemError("Fork failed", errno);
        }
    }

    void cleanUpCgroup(const std::string& groupName) {
        if (!deleteCgroup(groupName)) {
            reportSystemError("Failed to delete cgroup " + groupName, errno);
        }
    }

}

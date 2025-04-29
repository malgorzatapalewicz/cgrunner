#ifndef PROCESSRUNNER_H
#define PROCESSRUNNER_H

#include <string>
#include <vector>

namespace ProcessRunner {
    
    pid_t spawnProcess();
    void runProgram(const std::string& commandLine);
    std::vector<std::string> parseCommandLine(const std::string& commandLine);
    std::vector<char*> convertToArgv(std::vector<std::string>& args);
}

#endif 
#include "ProcessRunner.h"
#include "ErrorUtils.h"

#include <unistd.h>
#include <vector>
#include <sstream> 
#include <iterator>

using namespace ErrorUtils;

namespace ProcessRunner {

    static std::vector<std::string> parseCommandLine(const std::string& commandLine){
        std::istringstream iss(commandLine);
        return {std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>()};
    }

    static std::vector<char*> convertToArgv(std::vector<std::string>& args){
        std::vector<char*> argv;
        for(auto& arg : args){
            argv.push_back(&arg[0]);
        }
        argv.push_back(nullptr);
        return argv;
    }

    void runProgram(const std::string& commandLine) {
        std::vector<std::string> args = parseCommandLine(commandLine);
        std::vector<char*> argv = convertToArgv(args);

        execvp(argv[0], argv.data());

        reportSystemError("Exec failed", errno);
        _exit(1);
    }


} 
#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include <string>

namespace ArgumentParser {

    template <typename T>
    bool isLimitParsable(const char *arg, T &limitValue, const char *limitType);
    void parseArguments(int argc, char *argv[], bool &cpuSet, bool &memorySet, int &cpuPercent, size_t &memoryBytes);

}

#endif
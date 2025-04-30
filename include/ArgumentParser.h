#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H

#include <string>

namespace ArgumentParser
{
    void parseArguments(int argc, char *argv[], bool &cpuSet, bool &memorySet, int &cpuPercent, size_t &memoryBytes);
    void validateProvidedLimits(bool cpu, bool memory);
}

#endif
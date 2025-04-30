#include "ArgumentParser.h"
#include "ErrorUtils.h"

#include <getopt.h>
#include <string>
#include <stdexcept>

using namespace ErrorUtils;

namespace ArgumentParser {

    static const struct option* getLongOptions() {
        static const struct option options[] = {
            {"cpu",    required_argument, nullptr, 'c'},
            {"memory", required_argument, nullptr, 'm'},
            {nullptr,  0,                 nullptr,  0  }
        };
        return options;
    }

    static bool hasNegativeSign(const char* arg) {
        return arg[0] == '-';
    }

    static bool isNonNegativeLimit(const char* arg, const char* limitType) {
        return hasNegativeSign(arg) ? (reportUserError(std::string(limitType) + " limit must be a positive number."), false) : true;
    }

    template <typename T>
    bool isLimitParsable(const char* arg, T& limitValue, const char* limitType) {
        if (!isNonNegativeLimit(arg, limitType)) {
            return false;
        }

        try {
            if constexpr (std::is_same_v<T, int>) {
                limitValue = std::stoi(arg);
            }
            else if constexpr (std::is_same_v<T, size_t>) {
                limitValue = std::stoul(arg);
            }
            return true;
        }
        catch (const std::invalid_argument&) {
            reportUserError(std::string(limitType) + " limit must be a valid number.");
        }
        catch (const std::out_of_range&) {
            reportUserError(std::string(limitType) + " limit value is out of range.");
        }
        return false;
    }

    static bool isCpuLimitValid(const char* arg, int& cpuPercent) {
        if (isLimitParsable(arg, cpuPercent, "CPU")) {
            if (cpuPercent > 0 && cpuPercent <= 100) {
                return true;
            }
            reportUserError("CPU limit must be a number between 1 and 100.");
        }
        return false;
    }

    static bool isMemoryLimitValid(const char* arg, size_t& memoryBytes) {
        if (isLimitParsable(arg, memoryBytes, "Memory")) {
            if (memoryBytes > 0) {
                return true;
            }
            reportUserError("Memory limit must be greater than 0.");
        }
        return false;
    }

    static bool parseCpuLimit(const char* arg, bool& cpuLimitProvided, int& cpuPercent) {
        return cpuLimitProvided = isCpuLimitValid(arg, cpuPercent);
    }

    static bool parseMemoryLimit(const char* arg, bool& memoryLimitProvided, size_t& memoryBytes) {
        return memoryLimitProvided = isMemoryLimitValid(arg, memoryBytes);
    }

    static bool processOption(int opt, const char* optarg,
                              bool& cpuLimitProvided, bool& memoryLimitProvided,
                              int& cpuPercent, size_t& memoryBytes) {
        switch (opt) {
            case 'c':
                return parseCpuLimit(optarg, cpuLimitProvided, cpuPercent);
            case 'm':
                return parseMemoryLimit(optarg, memoryLimitProvided, memoryBytes);
            default:
                reportUserError("Invalid option.");
                return false;
        }
    }

    void parseArguments(int argc, char* argv[],
                        bool& cpuLimitProvided, bool& memoryLimitProvided,
                        int& cpuPercent, size_t& memoryBytes) {
        int opt;
        while ((opt = getopt_long(argc, argv, "c:m:", getLongOptions(), nullptr)) != -1) {
            if (!processOption(opt, optarg, cpuLimitProvided, memoryLimitProvided, cpuPercent, memoryBytes)) {
                return;
            }
        }
    }

    void validateProvidedLimits(bool cpuLimitProvided, bool memoryLimitProvided) {
        if (!cpuLimitProvided || !memoryLimitProvided) {
            reportUserError("Both CPU and memory limits must be specified.");
            exit(1);
        }
    }
    

}
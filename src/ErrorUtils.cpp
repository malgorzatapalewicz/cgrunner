#include "ErrorUtils.h"
#include <iostream>
#include <cstring>

namespace ErrorUtils {

void reportSystemError(const std::string &message, int errorCode) {
    std::cerr << message << ": " << strerror(errorCode) << std::endl;
}

void reportUserError(const std::string &message) {
    std::cerr << "Error: " << message << std::endl;
}

}
#include "ErrorUtils.h"
#include <iostream>
#include <cstring>

namespace ErrorUtils {

    void reportError(const std::string& message, int errorCode){
        std::cerr << message << ": " << strerror(errorCode) << std::endl;
    }
}
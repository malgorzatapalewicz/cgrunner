#pragma once
#include <string>

namespace ErrorUtils {
    
    void reportSystemError(const std::string &message, int errorCode);
    void reportUserError(const std::string &message);
}

#include "Helpers.h"
#include <cstdarg>
#include <vector>
#include <cstdio>

namespace Helpers {
    std::string format(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        
        // Determine required size
        va_list args_copy;
        va_copy(args_copy, args);
        int size = std::vsnprintf(nullptr, 0, fmt, args_copy);
        va_end(args_copy);
        
        if (size < 0) {
            va_end(args);
            return "";
        }
        
        // Format string
        std::vector<char> buffer(size + 1);
        std::vsnprintf(buffer.data(), buffer.size(), fmt, args);
        va_end(args);
        
        return std::string(buffer.data(), size);
    }
}


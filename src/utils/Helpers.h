#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <cctype>

namespace Helpers {
    // Check if character is a separator (for syntax highlighting)
    inline bool isSeparator(int c) {
        return std::isspace(c) || c == '\0' || 
               std::string(",.()+-/*=~%<>[];").find(c) != std::string::npos;
    }

    // String formatting helper
    std::string format(const char* fmt, ...);
}

#endif // HELPERS_H


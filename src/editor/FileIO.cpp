#include "FileIO.h"
#include <fstream>
#include <sstream>

bool FileIO::loadFile(const std::string& filename, std::vector<Row>& rows) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    rows.clear();
    std::string line;
    int idx = 0;
    
    while (std::getline(file, line)) {
        // Remove carriage return if present (Windows line endings)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        rows.emplace_back(idx++, line);
    }
    
    file.close();
    return true;
}

bool FileIO::saveFile(const std::string& filename, const std::vector<Row>& rows) {
    std::ofstream file(filename, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    
    for (const auto& row : rows) {
        file << row.getChars() << '\n';
    }
    
    file.close();
    return true;
}

std::string FileIO::rowsToString(const std::vector<Row>& rows) {
    std::ostringstream oss;
    for (const auto& row : rows) {
        oss << row.getChars() << '\n';
    }
    return oss.str();
}


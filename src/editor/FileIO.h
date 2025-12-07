#ifndef FILEIO_H
#define FILEIO_H

#include <string>
#include <vector>
#include "Row.h"

class FileIO {
public:
    // Load file into rows, return true on success
    static bool loadFile(const std::string& filename, std::vector<Row>& rows);
    
    // Save rows to file, return true on success
    static bool saveFile(const std::string& filename, const std::vector<Row>& rows);
    
    // Convert rows to single string with newlines
    static std::string rowsToString(const std::vector<Row>& rows);
};

#endif // FILEIO_H


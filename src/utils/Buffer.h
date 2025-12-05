#ifndef BUFFER_H
#define BUFFER_H

#include <string>

// Append buffer for efficient screen rendering
// Instead of many small write() calls, we accumulate data and write once
class Buffer {
public:
    Buffer();
    ~Buffer();

    void append(const std::string& s);
    void append(const char* s, size_t len);
    void clear();
    
    const std::string& getData() const { return data; }
    size_t getLength() const { return data.length(); }

private:
    std::string data;
};

#endif // BUFFER_H


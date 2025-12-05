#include "Buffer.h"

Buffer::Buffer() {
    data.reserve(1024);  // Pre-allocate for efficiency
}

Buffer::~Buffer() {
}

void Buffer::append(const std::string& s) {
    data += s;
}

void Buffer::append(const char* s, size_t len) {
    data.append(s, len);
}

void Buffer::clear() {
    data.clear();
}


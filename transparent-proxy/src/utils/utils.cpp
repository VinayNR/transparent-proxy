#include "utils.h"

#include <string>
#include <sstream>

void deleteAndNullifyPointer(char *& ptr, bool isArray) {
    isArray ? delete[] ptr : delete ptr;
    ptr = nullptr;
}

int searchKeyValueFromRawString(const std::string & data, const std::string & pattern, char delimiter) {
    size_t pattern_pos;
    std::stringstream ss(data);
    std::string line;
    // extract request line parameters
    while (getline(ss, line, delimiter)) {
        if (line.length() == 0) {
            // end of request
            break;
        }
        if ((pattern_pos = line.find(pattern)) != std::string::npos) {
            return stoi(line.substr(pattern_pos + pattern.length()));
        }
        if (ss.peek() == '\n') {
            ss.ignore();
        }
    }
    return 0;
}

int findPosition(const std::string s, const char *c) {
    for (int i=0; i<s.length(); i++) {
        if (s[i] == *c) {
            return i;
        }
    }
    return -1;
}
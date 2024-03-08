#pragma once

#include <string>

void deleteAndNullifyPointer(char *&, bool);
int searchKeyValueFromRawString(const std::string &, const std::string &, char);
int findPosition(const std::string, const char *);
#pragma once
#include <string>

class FileDialog {
public:
    // Ex: "*.png;*.jpg"
    static std::string Open(const char* filter);
};

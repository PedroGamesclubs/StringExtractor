#pragma once

#include <string>

class StringFileExtractor {
public:
    StringFileExtractor();
    std::wstring extract(const std::wstring& filePath);

private:
    std::wstring computeMD5(const std::wstring& filePath);
    std::wstring getTimestampString(const std::wstring& filePath);
    std::wstring getSizeOfImageString(const std::wstring& filePath);
};

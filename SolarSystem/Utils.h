#pragma once

#include <string>

namespace mc
{
    class File
    {
    public:
        File(const std::string& filepath);
        ~File();

        char* data;
        size_t size;
    };
}
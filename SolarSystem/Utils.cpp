#include "Utils.h"
#include <fstream>


namespace mc
{
    File::File(const std::string& filepath)
        : data{ nullptr }, size{0}
    {
        std::ifstream file;
        file.open(filepath);
        if (file.is_open())
        {
            file.seekg(0, std::ios::end);
            size = file.tellg();
            data = new char[size + 1];
            std::memset(data, 0, size + 1);
            file.seekg(0);
            file.read(data, size);
            file.close();
        }
        else
        {
            throw std::runtime_error("Error reading file: " + filepath);
        }
    }

    File::~File()
    {
        delete[] data;
    }
}
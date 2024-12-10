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

    float Utils::RandF()
    {
        return (float)(rand()) / (float)RAND_MAX;
    }

    float Utils::RandF(float a, float b)
    {
        return a + RandF() * (b - a);
    }

    XMVECTOR Utils::RandUnitVec3()
    {
        XMVECTOR one = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
        while (true)
        {
            XMVECTOR v = XMVectorSet(
                RandF(-1.0f, 1.0f),
                RandF(-1.0f, 1.0f),
                RandF(-1.0f, 1.0f), 0.0f);
            if (XMVector3Greater(XMVector3LengthSq(v), one))
            {
                continue;
            }
            return XMVector3Normalize(v);
        }

    }

}
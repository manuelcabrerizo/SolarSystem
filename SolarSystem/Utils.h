#pragma once

#include <DirectXMath.h>
#include <string>

using namespace DirectX;

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

    class Utils
    {
    public:
        static float RandF();
        static float RandF(float a, float b);
        static XMVECTOR RandUnitVec3();
    };
}
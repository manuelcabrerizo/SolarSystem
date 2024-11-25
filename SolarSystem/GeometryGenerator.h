#pragma once

#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

namespace mc
{
    struct Vertex
    {
        XMFLOAT3 position;
        XMFLOAT3 normal;
        XMFLOAT3 tangent;
        XMFLOAT2 uv;
    };

    struct MeshData
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
    };

    class GeometryGenerator
    {
    public:
        static void GenerateSphere(float radius, unsigned int numSubdivisions, MeshData& meshData);

    private:
        static void Subdivide(MeshData& meshData);
        static float AngleFromXY(float x, float y);
    };
}


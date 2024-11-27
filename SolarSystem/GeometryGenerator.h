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
        static void GenerateQuad(MeshData& meshData);
        static void GenerateSphere(float radius, unsigned int sliceCount, unsigned int stackCount, MeshData& meshData);
        static void GenerateGeosphere(float radius, unsigned int numSubdivisions, MeshData& meshData);

    private:
        static void Subdivide(MeshData& meshData);
        static float AngleFromXY(float x, float y);
    };
}


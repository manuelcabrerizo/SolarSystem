#pragma once

#include <DirectXMath.h>
#include <vector>
#include <string>

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

    struct CollisionQuad
    {
        XMFLOAT3 vertices[4];
        XMFLOAT3 normal;
    };

    struct CollisionData
    {
        std::vector<CollisionQuad> quads;

    };

    class GeometryGenerator
    {
    public:
        static void GenerateQuad(MeshData& meshData);
        static void GenerateSphere(float radius, unsigned int sliceCount, unsigned int stackCount, MeshData& meshData);
        static void GenerateGeosphere(float radius, unsigned int numSubdivisions, MeshData& meshData);
        static void LoadOBJFile(MeshData& meshData, const std::string& filepath);
        static void LoadCollisionDataFromOBJFile(CollisionData& collisionData, const std::string& filepath);
    private:
        static void Subdivide(MeshData& meshData);
        static float AngleFromXY(float x, float y);
    };
}


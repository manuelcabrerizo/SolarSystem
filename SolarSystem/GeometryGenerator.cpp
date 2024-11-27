#include "GeometryGenerator.h"

namespace mc
{
// PUBLICS:
    void GeometryGenerator::GenerateQuad(MeshData& meshData)
    {
        Vertex vertices[6] = {
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
            {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}
        };
        meshData.vertices.resize(6);
        memcpy(meshData.vertices.data(), vertices, sizeof(Vertex) * 6);
    }

    void GeometryGenerator::GenerateSphere(float radius, unsigned int sliceCount, unsigned int stackCount, MeshData& meshData)
    {
        meshData.vertices.clear();
        meshData.indices.clear();

        //
        // Compute the vertices stating at the top pole and moving down the stacks.
        //

        // Poles: note that there will be texture coordinate distortion as there is
        // not a unique point on the texture map to assign to the pole when mapping
        // a rectangular texture onto a sphere.
        Vertex topVertex{ XMFLOAT3(0.0f, +radius, 0.0f),XMFLOAT3(0.0f, +1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) };
        Vertex bottomVertex{ XMFLOAT3(0.0f, -radius, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) };

        meshData.vertices.push_back(topVertex);

        float phiStep = XM_PI / stackCount;
        float thetaStep = 2.0f * XM_PI / sliceCount;

        // Compute vertices for each stack ring (do not count the poles as rings).
        for (unsigned int i = 1; i <= stackCount - 1; ++i)
        {
            float phi = i * phiStep;

            // Vertices of ring.
            for (unsigned int j = 0; j <= sliceCount; ++j)
            {
                float theta = j * thetaStep;

                Vertex v;

                // spherical to cartesian
                v.position.x = radius * sinf(phi) * cosf(theta);
                v.position.y = radius * cosf(phi);
                v.position.z = radius * sinf(phi) * sinf(theta);

                // Partial derivative of P with respect to theta
                v.tangent.x = -radius * sinf(phi) * sinf(theta);
                v.tangent.y = 0.0f;
                v.tangent.z = +radius * sinf(phi) * cosf(theta);

                XMVECTOR T = XMLoadFloat3(&v.tangent);
                XMStoreFloat3(&v.tangent, XMVector3Normalize(T));

                XMVECTOR p = XMLoadFloat3(&v.position);
                XMStoreFloat3(&v.normal, XMVector3Normalize(p));

                v.uv.x = theta / XM_2PI;
                v.uv.y = phi / XM_PI;

                meshData.vertices.push_back(v);
            }
        }

        meshData.vertices.push_back(bottomVertex);

        //
        // Compute indices for top stack.  The top stack was written first to the vertex buffer
        // and connects the top pole to the first ring.
        //

        for (unsigned int i = 1; i <= sliceCount; ++i)
        {
            meshData.indices.push_back(0);
            meshData.indices.push_back(i + 1);
            meshData.indices.push_back(i);
        }

        //
        // Compute indices for inner stacks (not connected to poles).
        //

        // Offset the indices to the index of the first vertex in the first ring.
        // This is just skipping the top pole vertex.
        unsigned int baseIndex = 1;
        unsigned int ringVertexCount = sliceCount + 1;
        for (unsigned int i = 0; i < stackCount - 2; ++i)
        {
            for (unsigned int j = 0; j < sliceCount; ++j)
            {
                meshData.indices.push_back(baseIndex + i * ringVertexCount + j);
                meshData.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
                meshData.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

                meshData.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
                meshData.indices.push_back(baseIndex + i * ringVertexCount + j + 1);
                meshData.indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
            }
        }

        //
        // Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
        // and connects the bottom pole to the bottom ring.
        //

        // South pole vertex was added last.
        unsigned int southPoleIndex = (unsigned int)meshData.vertices.size() - 1;

        // Offset the indices to the index of the first vertex in the last ring.
        baseIndex = southPoleIndex - ringVertexCount;

        for (unsigned int i = 0; i < sliceCount; ++i)
        {
            meshData.indices.push_back(southPoleIndex);
            meshData.indices.push_back(baseIndex + i);
            meshData.indices.push_back(baseIndex + i + 1);
        }
    }


    void GeometryGenerator::GenerateGeosphere(float radius, unsigned int numSubdivisions, MeshData& meshData)
    {
        // Put a cap on the number of subdivisions.
        numSubdivisions = std::min(numSubdivisions, 5u);

        // Approximate a sphere by tessellating an icosahedron.

        const float X = 0.525731f;
        const float Z = 0.850651f;

        XMFLOAT3 pos[12] =
        {
            XMFLOAT3(-X, 0.0f, Z),  XMFLOAT3(X, 0.0f, Z),
            XMFLOAT3(-X, 0.0f, -Z), XMFLOAT3(X, 0.0f, -Z),
            XMFLOAT3(0.0f, Z, X),   XMFLOAT3(0.0f, Z, -X),
            XMFLOAT3(0.0f, -Z, X),  XMFLOAT3(0.0f, -Z, -X),
            XMFLOAT3(Z, X, 0.0f),   XMFLOAT3(-Z, X, 0.0f),
            XMFLOAT3(Z, -X, 0.0f),  XMFLOAT3(-Z, -X, 0.0f)
        };

        unsigned int k[60] =
        {
            1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
            1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
            3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
            10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
        };

        meshData.vertices.resize(12);
        meshData.indices.resize(60);

        for (unsigned int i = 0; i < 12; ++i)
            meshData.vertices[i].position = pos[i];

        for (unsigned int i = 0; i < 60; ++i)
            meshData.indices[i] = k[i];

        for (unsigned int i = 0; i < numSubdivisions; ++i)
            Subdivide(meshData);

        // Project vertices onto sphere and scale.
        for (unsigned int i = 0; i < meshData.vertices.size(); ++i)
        {
            // Project onto unit sphere.
            XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&meshData.vertices[i].position));

            // Project onto sphere.
            XMVECTOR p = radius * n;

            XMStoreFloat3(&meshData.vertices[i].position, p);
            XMStoreFloat3(&meshData.vertices[i].normal, n);

            // Derive texture coordinates from spherical coordinates.
            float theta = AngleFromXY(
                meshData.vertices[i].position.x,
                meshData.vertices[i].position.z);

            float phi = acosf(meshData.vertices[i].position.y / radius);

            meshData.vertices[i].uv.x = theta / XM_2PI;
            meshData.vertices[i].uv.y = phi / XM_PI;

            // Partial derivative of P with respect to theta
            meshData.vertices[i].tangent.x = -radius * sinf(phi) * sinf(theta);
            meshData.vertices[i].tangent.y = 0.0f;
            meshData.vertices[i].tangent.z = +radius * sinf(phi) * cosf(theta);

            XMVECTOR T = XMLoadFloat3(&meshData.vertices[i].tangent);
            XMStoreFloat3(&meshData.vertices[i].tangent, XMVector3Normalize(T));
        }
    }

// PRIVATES:
    void GeometryGenerator::Subdivide(MeshData& meshData)
    {
        // Save a copy of the input geometry.
        MeshData inputCopy = meshData;


        meshData.vertices.resize(0);
        meshData.indices.resize(0);

        //       v1
        //       *
        //      / \
    	//     /   \
    	//  m0*-----*m1
        //   / \   / \
    	//  /   \ /   \
    	// *-----*-----*
        // v0    m2     v2

        unsigned int numTris = inputCopy.indices.size() / 3;
        for (unsigned int i = 0; i < numTris; ++i)
        {
            Vertex v0 = inputCopy.vertices[inputCopy.indices[i * 3 + 0]];
            Vertex v1 = inputCopy.vertices[inputCopy.indices[i * 3 + 1]];
            Vertex v2 = inputCopy.vertices[inputCopy.indices[i * 3 + 2]];

            //
            // Generate the midpoints.
            //

            Vertex m0, m1, m2;

            // For subdivision, we just care about the position component.  We derive the other
            // vertex components in CreateGeosphere.

            m0.position = XMFLOAT3(
                0.5f * (v0.position.x + v1.position.x),
                0.5f * (v0.position.y + v1.position.y),
                0.5f * (v0.position.z + v1.position.z));

            m1.position = XMFLOAT3(
                0.5f * (v1.position.x + v2.position.x),
                0.5f * (v1.position.y + v2.position.y),
                0.5f * (v1.position.z + v2.position.z));

            m2.position = XMFLOAT3(
                0.5f * (v0.position.x + v2.position.x),
                0.5f * (v0.position.y + v2.position.y),
                0.5f * (v0.position.z + v2.position.z));

            //
            // Add new geometry.
            //

            meshData.vertices.push_back(v0); // 0
            meshData.vertices.push_back(v1); // 1
            meshData.vertices.push_back(v2); // 2
            meshData.vertices.push_back(m0); // 3
            meshData.vertices.push_back(m1); // 4
            meshData.vertices.push_back(m2); // 5

            meshData.indices.push_back(i * 6 + 0);
            meshData.indices.push_back(i * 6 + 3);
            meshData.indices.push_back(i * 6 + 5);

            meshData.indices.push_back(i * 6 + 3);
            meshData.indices.push_back(i * 6 + 4);
            meshData.indices.push_back(i * 6 + 5);

            meshData.indices.push_back(i * 6 + 5);
            meshData.indices.push_back(i * 6 + 4);
            meshData.indices.push_back(i * 6 + 2);

            meshData.indices.push_back(i * 6 + 3);
            meshData.indices.push_back(i * 6 + 1);
            meshData.indices.push_back(i * 6 + 4);
        }
    }

    float GeometryGenerator::AngleFromXY(float x, float y)
    {
        float theta = 0.0f;

        // Quadrant I or IV
        if (x >= 0.0f)
        {
            // If x = 0, then atanf(y/x) = +pi/2 if y > 0
            //                atanf(y/x) = -pi/2 if y < 0
            theta = atanf(y / x); // in [-pi/2, +pi/2]

            if (theta < 0.0f)
                theta += 2.0f * XM_PI; // in [0, 2*pi).
        }

        // Quadrant II or III
        else
            theta = atanf(y / x) + XM_PI; // in [0, 2*pi).

        return theta;
    }





}
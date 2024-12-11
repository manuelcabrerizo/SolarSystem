#pragma once

#include "ConstBuffer.h"
#include "GameConstBuffers.h"
#include <list>

namespace mc
{
    class Scene;
    class Mesh;
    class Texture;
    class VertexShader;
    class PixelShader;
    class GraphicsManager;

    class SceneNode
    {
    public:
        XMVECTOR GetPosition()
        {
            return position_;
        }

        void SetPosition(float x, float y, float z)
        {
            position_ = XMVectorSet(x, y, z, 1.0f);
        }
        void SetRotation(XMVECTOR rotation)
        {
            rotation_ = rotation;
        }
        void SetScale(float x, float y, float z)
        {
            scale_ = XMVectorSet(x, y, z, 1.0f);
        }
        void SetMesh(mc::Mesh* mesh) { mesh_ = mesh; }
        void SetTexture(mc::Texture* texture) { texture_ = texture; }
        void SetVertexShader(mc::VertexShader* vs) { vs_ = vs; }
        void SetPixelShader(mc::PixelShader* ps) { ps_ = ps; }

        XMVECTOR GetParentPosition();
        void SetCullBack(bool value);
        XMMATRIX GetModelMatrix();
        SceneNode& AddNode();
        void Draw(const mc::GraphicsManager& gm, const Scene& scene);

    private:
        mc::Mesh* mesh_{ nullptr };
        mc::Texture* texture_{ nullptr };
        mc::VertexShader* vs_{ nullptr };
        mc::PixelShader* ps_{ nullptr };
        bool cullBack_{ true };

        XMVECTOR position_;
        XMVECTOR rotation_;
        XMVECTOR scale_;
        std::list<SceneNode> childrens_;
        SceneNode* parent_;
    };

    class Scene
    {
    public:
        Scene(ObjectConstBuffer* objectCPUBuffer, ConstBuffer<ObjectConstBuffer>* objectGPUBuffer);
        SceneNode& AddNode();
        void Draw(const mc::GraphicsManager& gm);

        ObjectConstBuffer* objectCPUBuffer_;
        ConstBuffer<ObjectConstBuffer>* objectGPUBuffer_;
    private:
        SceneNode root_;
    };
}


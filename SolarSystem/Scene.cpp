#include "Scene.h"

#include "GraphicsManager.h"
#include "VertexShader.h"
#include "PixelShader.h"
#include "Mesh.h"
#include "Texture.h"

namespace mc
{
    XMVECTOR SceneNode::GetParentPosition()
    {
        SceneNode* parent = parent_;
        XMVECTOR position = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        while (parent)
        {
            position += parent->position_;
            parent = parent->parent_;
        }
        return position;
    }

    void SceneNode::SetCullBack(bool value)
    {
        cullBack_ = value;
    }

    XMMATRIX SceneNode::GetModelMatrix()
    {
        XMMATRIX trans = XMMatrixTranslationFromVector(GetParentPosition() + position_);
        XMMATRIX rot = XMMatrixRotationQuaternion(rotation_);
        XMMATRIX scale = XMMatrixScalingFromVector(scale_);
        return scale * rot * trans;
    }

    SceneNode& SceneNode::AddNode()
    {
        SceneNode& node = childrens_.emplace_back();
        node.parent_ = this;
        return node;
    }

    void SceneNode::Draw(const mc::GraphicsManager& gm, const Scene& scene)
    {
        if (!cullBack_)
        {
            gm.SetRasterizerStateCullNone();
        }

        XMMATRIX trans = XMMatrixTranslationFromVector(GetParentPosition() + position_);
        XMMATRIX rot = XMMatrixRotationQuaternion(rotation_);
        XMMATRIX scale = XMMatrixScalingFromVector(scale_);

        scene.objectCPUBuffer_->model = scale * rot * trans;
        scene.objectGPUBuffer_->Update(gm, *scene.objectCPUBuffer_);
        if (vs_) { vs_->Bind(gm); }
        if (ps_) { ps_->Bind(gm); }
        if (texture_) { texture_->Bind(gm, 0); }
        if (mesh_) { mesh_->Draw(gm); }
        if (texture_) { texture_->Unbind(gm, 0); }

        for (auto& child : childrens_)
        {
            child.Draw(gm, scene);
        }

        if (!cullBack_)
        {
            gm.SetRasterizerStateCullBack();
        }
    }



    Scene::Scene(ObjectConstBuffer* objectCPUBuffer, ConstBuffer<ObjectConstBuffer>* objectGPUBuffer)
        : objectCPUBuffer_(objectCPUBuffer), objectGPUBuffer_(objectGPUBuffer)
    {
    }

    SceneNode& Scene::AddNode()
    {
        return root_.AddNode();
    }


    void Scene::Draw(const mc::GraphicsManager& gm)
    {
        gm.SetRasterizerStateCullBack();
        root_.Draw(gm, *this);
    }

}
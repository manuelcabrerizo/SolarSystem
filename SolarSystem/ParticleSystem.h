#pragma once

#include "GraphicsResource.h"
#include "Texture.h"
#include "ConstBuffer.h"

#include <DirectXMath.h>

using namespace DirectX;

namespace mc
{
    class Shader;

    struct VertexParticle
    {
        XMFLOAT3 pos;
        XMFLOAT3 vel;
        XMFLOAT2 sizeW;
        float age;
        unsigned int type;
    };

    struct ParticleConstBuffer {
        XMFLOAT3 eyePosW;
        float gameTime;
        XMFLOAT3 emitPosW;
        float timeStep;
        XMFLOAT3 emitDirW;
        float pad0;
        XMFLOAT3 targetPosW;
        float pad1;
    };

    class ParticleSystem : public GraphicsResource
    {
    public:
        ParticleSystem(const ParticleSystem&) = delete;
        ParticleSystem& operator=(const ParticleSystem&) = delete;

        ~ParticleSystem();

        ParticleSystem(const GraphicsManager& gm, unsigned int maxParticles,
            Shader *soVShader, Shader *soGShader,
            Shader *dwVShader, Shader *dwPShader, Shader *dwGShader,
            Texture& texture);
        void Reset();
        void Update(XMFLOAT3 startPos, XMFLOAT3 targetPos, XMFLOAT3 cameraPos, float gameTime_, float dt);
        void Draw(const GraphicsManager& gm);
    private:
        void CreateVertexBuffer(const GraphicsManager& gm);
        void CreateRandomTexture(const GraphicsManager& gm);

        unsigned int maxParticles_{ 1000 };
        bool firstRun_{ true };

        float timeStep_{ 0.0f };
        float age_{ 0.0f };
        float gameTime_{ 0.0f };

        XMFLOAT3 eyePosW_{ 0.0f, 0.0f, 0.0f };
        XMFLOAT3 targetPosW_{ 0.0f, 0.0f, 0.0f };
        XMFLOAT3 emitPosW_{ 0.0f, 0.0f, 0.0f };
        XMFLOAT3 emitDirW_{ 0.0f, 1.0f, 0.0f };

        Microsoft::WRL::ComPtr<ID3D11Buffer> initVB_;
        ID3D11Buffer *drawVB_;
        ID3D11Buffer *streamOutVB_;

        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> randomTextureSRV_;
        Texture& texture_;

        // stream output shaders
        Shader *soVShader_;
        Shader *soGShader_;
        // draw shaders
        Shader *dwVShader_;
        Shader *dwPShader_;
        Shader *dwGShader_;

        mc::ConstBuffer<ParticleConstBuffer> constBuffer_;

        static ParticleConstBuffer particleConstBuffer;
    };
}


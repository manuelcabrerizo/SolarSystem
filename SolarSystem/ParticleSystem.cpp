#include "ParticleSystem.h"
#include <stdexcept>

#include "Shader.h"
#include "Utils.h"

namespace mc
{
    ParticleConstBuffer ParticleSystem::particleConstBuffer;

    ParticleSystem::ParticleSystem(const GraphicsManager& gm, unsigned int maxParticles,
        Shader* soVShader, Shader* soGShader,
        Shader* dwVShader, Shader* dwPShader, Shader* dwGShader,
        Texture& texture)
        : maxParticles_(maxParticles), firstRun_(true), texture_(texture),
        soVShader_(soVShader), soGShader_(soGShader),
        dwVShader_(dwVShader), dwPShader_(dwPShader), dwGShader_(dwGShader),
        constBuffer_(gm, BIND_TO_GS|BIND_TO_VS|BIND_TO_PS, particleConstBuffer, 5)
    {
        CreateVertexBuffer(gm);
        CreateRandomTexture(gm);
        constBuffer_.Bind(gm);
    }

    ParticleSystem::~ParticleSystem()
    {
        if (drawVB_)
        {
            drawVB_->Release();
        }
        if (streamOutVB_)
        {
            streamOutVB_->Release();
        }
    }

    void ParticleSystem::Reset()
    {
        firstRun_ = true;
        age_ = 0.0f;
    }

    void ParticleSystem::Update(XMFLOAT3 startPos, XMFLOAT3 startVel, XMFLOAT3 emitDir, XMFLOAT3 cameraPos, float gameTime, float dt, float thrust)
    {
        eyePosW_ = cameraPos;
        emitPosW_ = startPos;
        gameTime_ = gameTime;
        timeStep_ = dt;
        age_ += dt;
        emitDirW_ = emitDir;
        starVerlocity_ = startVel;
        thrust_ = thrust;
    }

    void ParticleSystem::Draw(const GraphicsManager& gm)
    {
        particleConstBuffer.gameTime = gameTime_;
        particleConstBuffer.timeStep = timeStep_;
        particleConstBuffer.eyePosW = eyePosW_;
        particleConstBuffer.emitPosW = emitPosW_;
        particleConstBuffer.emitDirW = emitDirW_;
        particleConstBuffer.starVelocity = starVerlocity_;
        particleConstBuffer.thrust = thrust_;
        constBuffer_.Update(gm, particleConstBuffer);

        gm.SetDepthStencilOff();
        gm.SetSamplerLinearWrap();
        GetDeviceContext(gm)->GSSetShaderResources(0, 1, randomTextureSRV_.GetAddressOf());
        GetDeviceContext(gm)->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        soVShader_->Bind(gm);
        GetDeviceContext(gm)->PSSetShader(nullptr, 0, 0);
        soGShader_->Bind(gm);

        unsigned int stride = sizeof(VertexParticle);
        unsigned int offset = 0;

        // On the first pass, use the initialization VB.  Otherwise, use
        // the VB that contains the current particle list.
        if (firstRun_) 
        {
            GetDeviceContext(gm)->IASetVertexBuffers(0, 1, initVB_.GetAddressOf(), &stride, &offset);
        }
        else 
        {
            GetDeviceContext(gm)->IASetVertexBuffers(0, 1, &drawVB_, &stride, &offset);
        }

        // Draw the current particle list using stream-out only to update them.  
        // The updated vertices are streamed-out to the target VB. 
        GetDeviceContext(gm)->SOSetTargets(1, &streamOutVB_, &offset);
        if (firstRun_)
        {
            GetDeviceContext(gm)->Draw(1, 0);
            firstRun_ = false;
        }
        else
        {
            GetDeviceContext(gm)->DrawAuto();
        }
       
        // done streaming-out--unbind the vertex buffer
        ID3D11Buffer* bufferArray[1] = { 0 };
        GetDeviceContext(gm)->SOSetTargets(1, bufferArray, &offset);

        
        // ping-pong the vertex buffers
        std::swap(drawVB_, streamOutVB_); // TODO: check that this work right

        gm.SetDepthStencilOnWriteMaskZero();
        gm.SetAdditiveBlending();

        GetDeviceContext(gm)->IASetVertexBuffers(0, 1, &drawVB_, &stride, &offset);
        texture_.Bind(gm, 0);
        dwVShader_->Bind(gm);
        dwPShader_->Bind(gm);
        dwGShader_->Bind(gm);

        GetDeviceContext(gm)->DrawAuto();
        texture_.Unbind(gm, 0);

       
        // reset the state
        gm.SetSamplerLinearClamp();
        gm.SetAlphaBlending();
        gm.SetDepthStencilOn();
        GetDeviceContext(gm)->VSSetShader(nullptr, 0, 0);
        GetDeviceContext(gm)->PSSetShader(nullptr, 0, 0);
        GetDeviceContext(gm)->GSSetShader(nullptr, 0, 0);
    }

    void ParticleSystem::CreateVertexBuffer(const GraphicsManager& gm)
    {
        D3D11_BUFFER_DESC vbd;
        vbd.Usage = D3D11_USAGE_DEFAULT;
        vbd.ByteWidth = sizeof(VertexParticle);
        vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vbd.CPUAccessFlags = 0;
        vbd.MiscFlags = 0;
        vbd.StructureByteStride = 0;

        // the initial particle emitter has type 0 and age 0. the rest
        // of the particle attribute do not apply to the emitter
        VertexParticle p;
        ZeroMemory(&p, sizeof(VertexParticle));
        p.age = 0;
        p.type = 0;

        D3D11_SUBRESOURCE_DATA vinitData;
        vinitData.pSysMem = &p;
        if (FAILED(GetDevice(gm)->CreateBuffer(&vbd, &vinitData, &initVB_))) {
            throw std::runtime_error("Error: Failed Creating Particle System Init Vertex Buffer");
        }

        // Create the ping-pong buffers for stream-out and drawing.
        vbd.ByteWidth = sizeof(VertexParticle) * maxParticles_;
        vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

        if (FAILED(GetDevice(gm)->CreateBuffer(&vbd, 0, &drawVB_))) {
            throw std::runtime_error("Error: Failed Creating Particle System Draw Vertex Buffer");
        }
        if (FAILED(GetDevice(gm)->CreateBuffer(&vbd, 0, &streamOutVB_))) {
            throw std::runtime_error("Error: Failed Creating Particle System Stream Out Vertex Buffer\n");
        }
    }

    void ParticleSystem::CreateRandomTexture(const GraphicsManager& gm)
    {
        int width = 1024;
        XMFLOAT4 *randomValues = new XMFLOAT4[width];
        for (int i = 0; i < width; i++)
        {
            randomValues[i].x = Utils::RandF(-1, 1);
            randomValues[i].y = Utils::RandF(-1, 1);
            randomValues[i].z = Utils::RandF(-1, 1);
            randomValues[i].w = Utils::RandF(-1, 1);
        }
        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = randomValues;
        initData.SysMemPitch = width * sizeof(XMFLOAT4);
        initData.SysMemSlicePitch = 0;
        // Create the texture
        D3D11_TEXTURE1D_DESC texDesc;
        texDesc.Width = width;
        texDesc.MipLevels = 1;
        texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        texDesc.Usage = D3D11_USAGE_IMMUTABLE;
        texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;
        texDesc.ArraySize = 1;
        Microsoft::WRL::ComPtr<ID3D11Texture1D> randomTex;;
        if (FAILED(GetDevice(gm)->CreateTexture1D(&texDesc, &initData, &randomTex))) {
            throw std::runtime_error("Error: Failed Creating Particle System Random Texture");
        }
        // Create the resource view
        D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
        viewDesc.Format = texDesc.Format;
        viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
        viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
        viewDesc.Texture1D.MostDetailedMip = 0;
        if (FAILED(GetDevice(gm)->CreateShaderResourceView(randomTex.Get(), &viewDesc, &randomTextureSRV_))) {
            throw std::runtime_error("Error: Failed Creating Particle System Random Texture Shader Resource View");
        }

        delete[] randomValues;
    }
}
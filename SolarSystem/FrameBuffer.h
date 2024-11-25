#pragma once

#include "Bindable.h"

namespace mc
{
    class FrameBuffer : public Bindable
    {
    public:
        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;

        FrameBuffer(const GraphicsManager& gm, 
            unsigned int x, unsigned int y,
            unsigned int w, unsigned int h);
        void Bind(const GraphicsManager& gm) override;
        void Clear(const GraphicsManager& gm, float r, float g, float b);
        void BindAsTexture(const GraphicsManager& gm, unsigned int slot);
        void UnbindAsTexture(const GraphicsManager& gm, unsigned int slot);

    private:
        unsigned int x_, y_, w_, h_;
        DXGI_FORMAT format_;
        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView_;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView_;

        void CreateTexture(const GraphicsManager& gm);
        void CreateRenderTargetView(const GraphicsManager& gm);
        void CreateShaderResourceView(const GraphicsManager& gm);
        void CreateDepthStencilView(const GraphicsManager& gm);
    };
}


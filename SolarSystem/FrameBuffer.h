#pragma once

#include "GraphicsResource.h"

namespace mc
{
    class FrameBuffer : public GraphicsResource
    {
    public:
        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator=(const FrameBuffer&) = delete;

        FrameBuffer(const GraphicsManager& gm, 
            unsigned int x, unsigned int y,
            unsigned int w, unsigned int h);
        FrameBuffer(const GraphicsManager& gm,
            unsigned int x, unsigned int y,
            unsigned int w, unsigned int h,
            DXGI_FORMAT format, bool hasMsaa = false, int msaa = 1);
        void Bind(const GraphicsManager& gm);
        void Clear(const GraphicsManager& gm, float r, float g, float b);
        void BindAsTexture(const GraphicsManager& gm, unsigned int slot);
        void UnbindAsTexture(const GraphicsManager& gm, unsigned int slot);

        void Resolve(const GraphicsManager& gm);

    private:
        unsigned int x_, y_, w_, h_;
        unsigned int msaa_{1};
        unsigned int msaaQuality_;
        DXGI_FORMAT format_;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView_;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> resolveTexture_;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView_;

        void CreateTexture(const GraphicsManager& gm);
        void CreateRenderTargetView(const GraphicsManager& gm);
        void CreateShaderResourceView(const GraphicsManager& gm);
        void CreateDepthStencilView(const GraphicsManager& gm);

        void CreateTextureMSAA(const GraphicsManager& gm);
        void CreateRenderTargetViewMSAA(const GraphicsManager& gm);
        void CreateShaderResourceViewMSAA(const GraphicsManager& gm);
        void CreateDepthStencilViewMSAA(const GraphicsManager& gm);
    };
}


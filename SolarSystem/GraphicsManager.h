#pragma once

#include "Window.h"

#include <d3d11.h>
#include <wrl.h>

#include <vector>
#include <string>

namespace mc
{
    class GraphicsManager
    {
        friend class GraphicsResource;
    public:
        GraphicsManager(const Window& window);
        ~GraphicsManager() = default;
        GraphicsManager(const GraphicsManager&) = delete;
        GraphicsManager& operator=(const GraphicsManager&) = delete;
            
        void Clear(float r, float g, float b) const;
        void Present() const;
        void SetViewport(float x, float y, float width, float height) const;
        void BindBackBuffer();

        void SetSamplerLinear();
        void SetSamplerPoint();

        void SetRasterizerStateCullBack();
        void SetRasterizerStateCullFront();
        void SetRasterizerStateCullNone();
        void SetRasterizerStateWireframe();

        void SetDepthStencilOn();
        void SetDepthStencilOff();
        void SetDepthStencilOnWriteMaskZero();

        void SetAlphaBlending();
        void SetAdditiveBlending();
        void SetBlendingOff();   

    private:
        void CreateDevice();
        void CreateSwapChain(const Window& window);
        void CreateRenderTargetView();
        void CreateDepthStencilView(const Window& window);
        void CreateSamplerStates();
        void CreateRasterizerStates();
        void CreateDepthStencilStates();
        void CreateBendingStates();

        Microsoft::WRL::ComPtr<ID3D11Device> device_;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext_;
        Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView_;

        Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerStateLinear_;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerStatePoint_;

        Microsoft::WRL::ComPtr<ID3D11RasterizerState> wireFrameRasterizer_;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> fillRasterizerCullBack_;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> fillRasterizerCullFront_;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> fillRasterizerCullNone_;

        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilOn_;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilOff_;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilOnWriteMaskZero_;

        Microsoft::WRL::ComPtr<ID3D11BlendState> alphaBlendOn_;
        Microsoft::WRL::ComPtr<ID3D11BlendState> alphaBlendOff_;
        Microsoft::WRL::ComPtr<ID3D11BlendState> additiveBlending_;

        unsigned int w_, h_;
    };

}


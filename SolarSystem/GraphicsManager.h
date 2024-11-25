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
        void SetViewport(float width, float height);
        
    private:
        void CreateDevice();
        void CreateSwapChain(const Window& window);
        void CreateRenderTargetView();
        void CreateDepthStencilView(const Window& window);

        Microsoft::WRL::ComPtr<ID3D11Device> device_;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext_;
        Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView_;
    };

}


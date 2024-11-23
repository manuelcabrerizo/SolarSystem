#pragma once

#include "Window.h"

#include <d3d11.h>
#include <vector>
#include <string>

namespace mc
{
    using Shader = size_t;

    class GraphicsManager
    {
    public:
        GraphicsManager(const Window& window);
        ~GraphicsManager();

        void Clear(float r, float g, float b) const;
        void Present() const;
        void SetViewport(float width, float height);
        
        Shader CreateVertexShader(const std::string& filepath);
        Shader CreatePixelShader(const std::string& filepath);

        void SetVertexShader(Shader shader);
        void SetPixelShader(Shader shader);

    private:
        void CreateDevice();
        void CreateSwapChain(const Window& window);
        void CreateRenderTargetView();
        void CreateDepthStencilView(const Window& window);

        ID3D11Device* device_;
        ID3D11DeviceContext* deviceContext_;
        IDXGISwapChain* swapChain_;
        ID3D11RenderTargetView* renderTargetView_;
        ID3D11DepthStencilView* depthStencilView_;

        std::vector<ID3D11VertexShader*> vertexShaders_;
        std::vector<ID3D11PixelShader*> pixelShaders_;
    };

}


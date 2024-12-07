#include "GraphicsManager.h"
#include <stdexcept>
#include <d3dcompiler.h>
#include "Utils.h"
#include <iostream>

namespace mc
{
    GraphicsManager::GraphicsManager(const Window& window)
        : w_(window.Width()), h_(window.Height())
    {
        CreateDevice();
        CreateSwapChain(window);
        CreateRenderTargetView();
        CreateDepthStencilView(window);
        CreateSamplerStates();
        CreateRasterizerStates();
        CreateDepthStencilStates();
        CreateBendingStates();
        SetViewport(
            0.0f, 0.0f,
            static_cast<float>(window.Width()),
            static_cast<float>(window.Height())
        );
        BindBackBuffer();
        SetSamplerLinear();
    }


    void GraphicsManager::Clear(float r, float g, float b) const
    {
        float clearColor[] = { r, g, b, 1.0f };
        deviceContext_->ClearRenderTargetView(renderTargetView_.Get(), clearColor);
        deviceContext_->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }

    void GraphicsManager::Present() const
    {
        swapChain_->Present(1, 0);
    }

    void  GraphicsManager::SetViewport(float x, float y, float width, float height) const
    {
        // set up the viewport
        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = x;
        viewport.TopLeftY = y;
        viewport.Width = width;
        viewport.Height = height;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        deviceContext_->RSSetViewports(1, &viewport);
    }

    void GraphicsManager::BindBackBuffer()
    {
        deviceContext_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());
        SetViewport(0.0f, 0.0f, static_cast<float>(w_), static_cast<float>(h_));
    }

    void GraphicsManager::CreateDevice()
    {
        // Create the device and deviceContext
        int deviceFlags = 0;
#ifdef _DEBUG
        deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL featureLevel;
        if (FAILED(D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, deviceFlags, 0, 0, D3D11_SDK_VERSION, &device_, &featureLevel, &deviceContext_)))
        {
            throw std::runtime_error("Error creating Directx11 device.");
        }
    }

    void GraphicsManager::CreateSwapChain(const Window& window)
    {
        // create the d3d11 device swapchain and device context
        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        swapChainDesc.BufferDesc.Width = window.Width();
        swapChainDesc.BufferDesc.Height = window.Height();
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 1;
        swapChainDesc.OutputWindow = window.Get();
        swapChainDesc.Windowed = true;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.Flags = 0;

        // Create the swap chain
        Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
        Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
        Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
        device_->QueryInterface(__uuidof(IDXGIDevice), &dxgiDevice);
        dxgiDevice->GetParent(__uuidof(IDXGIAdapter), &dxgiAdapter);
        dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &dxgiFactory);
        if (FAILED(dxgiFactory->CreateSwapChain(device_.Get(), &swapChainDesc, &swapChain_)))
        {
            throw std::runtime_error("Error creating swap chain.");
        }
    }

    void GraphicsManager::CreateRenderTargetView()
    {
        // create render target view
        Microsoft::WRL::ComPtr<ID3D11Texture2D> backBufferTexture;
        swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBufferTexture);
        if (FAILED(device_->CreateRenderTargetView(backBufferTexture.Get(), 0, &renderTargetView_)))
        {
            throw std::runtime_error("Error creating render target.");
        }
    }

    void GraphicsManager::CreateDepthStencilView(const Window& window)
    {
        // check for msaa
        UINT msaaQuality4x;
        device_->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &msaaQuality4x);
        if (msaaQuality4x <= 0)
        {
            throw std::runtime_error("Error msaa 4x not supported.");
        }
        // create the depth stencil texture
        Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilTexture;
        D3D11_TEXTURE2D_DESC depthStencilTextureDesc;
        depthStencilTextureDesc.Width = window.Width();
        depthStencilTextureDesc.Height = window.Height();
        depthStencilTextureDesc.MipLevels = 1;
        depthStencilTextureDesc.ArraySize = 1;
        depthStencilTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilTextureDesc.SampleDesc.Count = 1;
        depthStencilTextureDesc.SampleDesc.Quality = 0;
        depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilTextureDesc.CPUAccessFlags = 0;
        depthStencilTextureDesc.MiscFlags = 0;
        if (FAILED(device_->CreateTexture2D(&depthStencilTextureDesc, 0, &depthStencilTexture)))
        {
            throw std::runtime_error("Error creating depth stencil texture.");
        }
        if (FAILED(device_->CreateDepthStencilView(depthStencilTexture.Get(), 0, &depthStencilView_)))
        {
            throw std::runtime_error("Error creating depth stencil view.");
        }
    }

    void GraphicsManager::CreateSamplerStates()
    {
        // D3D11_TEXTURE_ADDRESS_CLAMP; D3D11_TEXTURE_ADDRESS_WRAP;
        D3D11_SAMPLER_DESC colorMapDesc{};
        colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(device_->CreateSamplerState(&colorMapDesc, &samplerStatePoint_)))
        {
            throw std::runtime_error("Error: Failed Creating sampler state Point\n");
        }
        colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        if (FAILED(device_->CreateSamplerState(&colorMapDesc, &samplerStateLinear_)))
        {
            throw std::runtime_error("Error: Failed Creating sampler state Linear\n");
        }
    }

    void GraphicsManager::SetSamplerLinear() const
    {
        deviceContext_->PSSetSamplers(0, 1, samplerStateLinear_.GetAddressOf());
    }

    void GraphicsManager::SetSamplerPoint() const
    {
        deviceContext_->PSSetSamplers(0, 1, samplerStatePoint_.GetAddressOf());
    }

    void GraphicsManager::CreateRasterizerStates()
    {
        D3D11_RASTERIZER_DESC fillRasterizerFrontDesc = {};
        fillRasterizerFrontDesc.FillMode = D3D11_FILL_SOLID;
        fillRasterizerFrontDesc.CullMode = D3D11_CULL_FRONT;
        fillRasterizerFrontDesc.DepthClipEnable = true;
        fillRasterizerFrontDesc.AntialiasedLineEnable = true;
        fillRasterizerFrontDesc.MultisampleEnable = true;
        device_->CreateRasterizerState(&fillRasterizerFrontDesc, &fillRasterizerCullFront_);

        D3D11_RASTERIZER_DESC fillRasterizerBackDesc = {};
        fillRasterizerBackDesc.FillMode = D3D11_FILL_SOLID;
        fillRasterizerBackDesc.CullMode = D3D11_CULL_BACK;
        fillRasterizerBackDesc.DepthClipEnable = true;
        fillRasterizerBackDesc.AntialiasedLineEnable = true;
        fillRasterizerBackDesc.MultisampleEnable = true;
        device_->CreateRasterizerState(&fillRasterizerBackDesc, &fillRasterizerCullBack_);

        D3D11_RASTERIZER_DESC fillRasterizerNoneDesc = {};
        fillRasterizerNoneDesc.FillMode = D3D11_FILL_SOLID;
        fillRasterizerNoneDesc.CullMode = D3D11_CULL_NONE;
        fillRasterizerNoneDesc.DepthClipEnable = true;
        fillRasterizerNoneDesc.AntialiasedLineEnable = true;
        fillRasterizerNoneDesc.MultisampleEnable = true;
        device_->CreateRasterizerState(&fillRasterizerNoneDesc, &fillRasterizerCullNone_);

        D3D11_RASTERIZER_DESC wireFrameRasterizerDesc = {};
        wireFrameRasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
        wireFrameRasterizerDesc.CullMode = D3D11_CULL_NONE;
        wireFrameRasterizerDesc.DepthClipEnable = true;
        wireFrameRasterizerDesc.AntialiasedLineEnable = true;
        wireFrameRasterizerDesc.MultisampleEnable = true;
        device_->CreateRasterizerState(&wireFrameRasterizerDesc, &wireFrameRasterizer_);
    }

    void GraphicsManager::SetRasterizerStateCullBack() const
    {
        deviceContext_->RSSetState(fillRasterizerCullBack_.Get());
    }

    void GraphicsManager::SetRasterizerStateCullFront() const
    {
        deviceContext_->RSSetState(fillRasterizerCullFront_.Get());
    }

    void GraphicsManager::SetRasterizerStateCullNone() const
    {
        deviceContext_->RSSetState(fillRasterizerCullNone_.Get());
    }

    void GraphicsManager::SetRasterizerStateWireframe() const
    {
        deviceContext_->RSSetState(wireFrameRasterizer_.Get());
    }


    void GraphicsManager::CreateDepthStencilStates()
    {
        // create depth stencil states
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
        // Depth test parameters
        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
        // Stencil test parameters
        depthStencilDesc.StencilEnable = true;
        depthStencilDesc.StencilReadMask = 0xFF;
        depthStencilDesc.StencilWriteMask = 0xFF;
        // Stencil operations if pixel is front-facing
        depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
        depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        // Stencil operations if pixel is back-facing
        depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
        depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        device_->CreateDepthStencilState(&depthStencilDesc, &depthStencilOn_);

        depthStencilDesc.DepthEnable = false;
        depthStencilDesc.StencilEnable = false;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        device_->CreateDepthStencilState(&depthStencilDesc, &depthStencilOff_);

        depthStencilDesc.DepthEnable = true;
        depthStencilDesc.StencilEnable = true;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
        device_->CreateDepthStencilState(&depthStencilDesc, &depthStencilOnWriteMaskZero_);
    }

    void GraphicsManager::SetDepthStencilOn() const
    {
        deviceContext_->OMSetDepthStencilState(depthStencilOn_.Get(), 1);
    }

    void GraphicsManager::SetDepthStencilOff() const
    {
        deviceContext_->OMSetDepthStencilState(depthStencilOff_.Get(), 1);
    }

    void GraphicsManager::SetDepthStencilOnWriteMaskZero() const
    {
        deviceContext_->OMSetDepthStencilState(depthStencilOnWriteMaskZero_.Get(), 1);
    }

    void GraphicsManager::CreateBendingStates()
    {
        D3D11_BLEND_DESC blendStateDesc = {};
        blendStateDesc.RenderTarget[0].BlendEnable = true;
        blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        device_->CreateBlendState(&blendStateDesc, &alphaBlendOn_);

        blendStateDesc.RenderTarget[0].BlendEnable = false;
        blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        device_->CreateBlendState(&blendStateDesc, &alphaBlendOff_);

        blendStateDesc.RenderTarget[0].BlendEnable = true;
        blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
        blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        device_->CreateBlendState(&blendStateDesc, &additiveBlending_);
    }

    void GraphicsManager::SetAlphaBlending() const
    {
        deviceContext_->OMSetBlendState(alphaBlendOn_.Get(), 0, 0xffffffff);
    }

    void GraphicsManager::SetAdditiveBlending() const
    {
        deviceContext_->OMSetBlendState(additiveBlending_.Get(), 0, 0xffffffff);

    }

    void GraphicsManager::SetBlendingOff() const
    {
        deviceContext_->OMSetBlendState(alphaBlendOff_.Get(), 0, 0xffffffff);
    }

}
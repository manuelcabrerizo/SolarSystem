#include "GraphicsManager.h"
#include <stdexcept>
#include <d3dcompiler.h>
#include "Utils.h"
#include <iostream>

namespace mc
{
    GraphicsManager::GraphicsManager(const Window& window) {

        CreateDevice();
        CreateSwapChain(window);
        CreateRenderTargetView();
        CreateDepthStencilView(window);
        SetViewport(
            static_cast<float>(window.Width()),
            static_cast<float>(window.Height())
        );

        deviceContext_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());
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

    void  GraphicsManager::SetViewport(float width, float height)
    {
        // set up the viewport
        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = 0.0f;
        viewport.TopLeftY = 0.0f;
        viewport.Width = width;
        viewport.Height = height;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        deviceContext_->RSSetViewports(1, &viewport);
    }

    void GraphicsManager::CreateDevice()
    {
        // Create the device and deviceContext
        int deviceFlags = D3D11_CREATE_DEVICE_DEBUG;
        D3D_FEATURE_LEVEL featureLevel;
        if (FAILED(D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, deviceFlags, 0, 0, D3D11_SDK_VERSION, &device_, &featureLevel, &deviceContext_)))
        {
            throw std::runtime_error("Error creating Directx11 device.");
        }
    }

    void GraphicsManager::CreateSwapChain(const Window& window)
    {
        // check for msaa
        UINT msaaQuality4x;
        device_->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &msaaQuality4x);
        if (msaaQuality4x <= 0)
        {
            throw std::runtime_error("Error msaa 4x not supported.");
        }
        // create the d3d11 device swapchain and device context
        DXGI_SWAP_CHAIN_DESC swapChainDesc;
        swapChainDesc.BufferDesc.Width = window.Width();
        swapChainDesc.BufferDesc.Height = window.Height();
        swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swapChainDesc.SampleDesc.Count = 4;
        swapChainDesc.SampleDesc.Quality = msaaQuality4x - 1;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount = 2;
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
        depthStencilTextureDesc.SampleDesc.Count = 4;
        depthStencilTextureDesc.SampleDesc.Quality = msaaQuality4x - 1; // TODO: test if this is correct
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
}
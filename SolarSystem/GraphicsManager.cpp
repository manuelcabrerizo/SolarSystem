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
    }

    GraphicsManager::~GraphicsManager()
    {
        for (auto* shader : vertexShaders_)
        {
            if (shader)
            {
                shader->Release();
            }
        }
        for (auto* shader : pixelShaders_)
        {
            if (shader)
            {
                shader->Release();
            }
        }
        if (depthStencilView_) { depthStencilView_->Release(); }
        if (renderTargetView_) { renderTargetView_->Release(); }
        if (swapChain_) { swapChain_->Release(); }
        if (deviceContext_) { deviceContext_->Release(); }
        if (device_) { device_->Release(); }
    }

    void GraphicsManager::Clear(float r, float g, float b) const
    {
        float clearColor[] = { r, g, b, 1.0f };
        deviceContext_->ClearRenderTargetView(renderTargetView_, clearColor);
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
        HRESULT result = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, deviceFlags, 0, 0, D3D11_SDK_VERSION, &device_, &featureLevel, &deviceContext_);
        if (FAILED(result))
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
        IDXGIDevice* dxgiDevice = 0;
        device_->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
        IDXGIAdapter* dxgiAdapter = 0;
        dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
        IDXGIFactory* dxgiFactory = 0;
        dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
        HRESULT result = dxgiFactory->CreateSwapChain(device_, &swapChainDesc, &swapChain_);
        if (dxgiDevice) dxgiDevice->Release();
        if (dxgiAdapter) dxgiAdapter->Release();
        if (dxgiFactory) dxgiFactory->Release();
        if (FAILED(result))
        {
            throw std::runtime_error("Error creating swap chain.");
        }
    }

    void GraphicsManager::CreateRenderTargetView()
    {
        // create render target view
        ID3D11Texture2D* backBufferTexture = 0;
        swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBufferTexture);
        HRESULT result = device_->CreateRenderTargetView(backBufferTexture, 0, &renderTargetView_);
        if (backBufferTexture)
        {
            backBufferTexture->Release();
        }
        if (FAILED(result))
        {
            throw std::runtime_error("Error creating render target.");
        }
    }

    void GraphicsManager::CreateDepthStencilView(const Window& window)
    {
        // create the depth stencil texture
        ID3D11Texture2D* depthStencilTexture = 0;
        D3D11_TEXTURE2D_DESC depthStencilTextureDesc;
        depthStencilTextureDesc.Width = window.Width();
        depthStencilTextureDesc.Height = window.Height();
        depthStencilTextureDesc.MipLevels = 1;
        depthStencilTextureDesc.ArraySize = 1;
        depthStencilTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilTextureDesc.SampleDesc.Count = 4;
        depthStencilTextureDesc.SampleDesc.Quality = 3; // TODO: test if this is correct
        depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilTextureDesc.CPUAccessFlags = 0;
        depthStencilTextureDesc.MiscFlags = 0;
        HRESULT result = device_->CreateTexture2D(&depthStencilTextureDesc, 0, &depthStencilTexture);
        if (FAILED(result))
        {
            throw std::runtime_error("Error creating depth stencil texture.");
        }
        result = device_->CreateDepthStencilView(depthStencilTexture, 0, &depthStencilView_);
        if (depthStencilTexture)
        {
            depthStencilTexture->Release();
        }
        if (FAILED(result))
        {
            throw std::runtime_error("Error creating depth stencil view.");
        }
    }


    Shader GraphicsManager::CreateVertexShader(const std::string& filepath)
    {
        ID3D11VertexShader* shader = nullptr;
        File shaderFile(filepath);
        ID3DBlob* shaderCompiled = nullptr;
        ID3DBlob* errorShader = nullptr;
        HRESULT result = D3DCompile(shaderFile.data, shaderFile.size,
            0, 0, 0, "vs_main", "vs_5_0",
            D3DCOMPILE_ENABLE_STRICTNESS, 0,
            &shaderCompiled, &errorShader);
        if (errorShader != 0)
        {
            char* errorString = (char*)errorShader->GetBufferPointer();
            std::cout << "Error conpiling VERTEX SHADER: " << filepath << "\n";
            std::cout << errorString << "\n";
            errorShader->Release();
        }
        else
        {
            result = device_->CreateVertexShader(
                shaderCompiled->GetBufferPointer(),
                shaderCompiled->GetBufferSize(), 0,
                &shader);
        }
        if (shaderCompiled)
        {
            shaderCompiled->Release();
            vertexShaders_.push_back(shader);
            return vertexShaders_.size() - 1;
        }
        return -1;
    }


    Shader GraphicsManager::CreatePixelShader(const std::string& filepath)
    {
        ID3D11PixelShader* shader = nullptr;
        File shaderFile(filepath);
        ID3DBlob* shaderCompiled = nullptr;
        ID3DBlob* errorShader = nullptr;
        HRESULT result = D3DCompile(shaderFile.data, shaderFile.size,
            0, 0, 0, "fs_main", "ps_5_0",
            D3DCOMPILE_ENABLE_STRICTNESS, 0,
            &shaderCompiled, &errorShader);
        if (errorShader != 0)
        {
            char* errorString = (char*)errorShader->GetBufferPointer();
            std::cout << "Error conpiling PIXEL SHADER: " << filepath << "\n";
            std::cout << errorString << "\n";
            errorShader->Release();
        }
        else
        {
            result = device_->CreatePixelShader(
                shaderCompiled->GetBufferPointer(),
                shaderCompiled->GetBufferSize(), 0,
                &shader);
        }
        if (shaderCompiled)
        {
            shaderCompiled->Release();
            pixelShaders_.push_back(shader);
            return pixelShaders_.size() - 1;
        }
        return -1;
    }

    void GraphicsManager::SetVertexShader(Shader shader)
    {
        deviceContext_->VSSetShader(vertexShaders_[shader], 0, 0);
    }

    void GraphicsManager::SetPixelShader(Shader shader)
    {
        deviceContext_->PSSetShader(pixelShaders_[shader], 0, 0);
    }
}
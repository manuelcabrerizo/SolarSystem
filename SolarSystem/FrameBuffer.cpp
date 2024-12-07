#include "FrameBuffer.h"
#include <stdexcept>

namespace mc
{
    FrameBuffer::FrameBuffer(const GraphicsManager& gm,
        unsigned int x, unsigned int y,
        unsigned int w, unsigned int h)
        : x_(x), y_(y), w_(w), h_(h), format_(DXGI_FORMAT_R8G8B8A8_UNORM)
    {
        CreateTexture(gm);
        CreateRenderTargetView(gm);
        CreateShaderResourceView(gm);
        CreateDepthStencilView(gm);
    }

    FrameBuffer::FrameBuffer(const GraphicsManager& gm,
        unsigned int x, unsigned int y,
        unsigned int w, unsigned int h,
        DXGI_FORMAT format, bool msaa)
        : x_(x), y_(y), w_(w), h_(h), format_(format)
    {
        if (!msaa)
        {
            CreateTexture(gm);
            CreateRenderTargetView(gm);
            CreateShaderResourceView(gm);
            CreateDepthStencilView(gm);
        }
        else
        {
            // check for msaa
            GetDevice(gm)->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 8, &msaaQuality_);
            if (msaaQuality_ <= 0)
            {
                throw std::runtime_error("Error msaa not supported.");
            }
            CreateTextureMSAA(gm);
            CreateRenderTargetViewMSAA(gm);
            CreateShaderResourceViewMSAA(gm);
            CreateDepthStencilViewMSAA(gm);
        }
    }

    void FrameBuffer::Bind(const GraphicsManager& gm)
    {
        GetDeviceContext(gm)->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());
        gm.SetViewport(static_cast<float>(x_), static_cast<float>(y_), static_cast<float>(w_), static_cast<float>(h_));
    }

    void FrameBuffer::Clear(const GraphicsManager& gm, float r, float g, float b)
    {
        float clearColor[] = { r, g, b, 1.0f };
        GetDeviceContext(gm)->ClearRenderTargetView(renderTargetView_.Get(), clearColor);
        GetDeviceContext(gm)->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }

    void FrameBuffer::BindAsTexture(const GraphicsManager& gm, unsigned int slot)
    {
        GetDeviceContext(gm)->PSSetShaderResources(slot, 1, shaderResourceView_.GetAddressOf());
    }

    void FrameBuffer::UnbindAsTexture(const GraphicsManager& gm, unsigned int slot)
    {
        ID3D11ShaderResourceView* null[] = { nullptr };
        GetDeviceContext(gm)->PSSetShaderResources(slot, 1, null);
    }


    void FrameBuffer::CreateTexture(const GraphicsManager& gm)
    {
        D3D11_TEXTURE2D_DESC texDesc{};
        texDesc.Width = w_;
        texDesc.Height = h_;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = format_;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;
        if (FAILED(GetDevice(gm)->CreateTexture2D(&texDesc, 0, &texture_)))
        {
            throw std::runtime_error("Error creating frame buffer texture");
        }
    }

    void FrameBuffer::CreateRenderTargetView(const GraphicsManager& gm)
    {
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
        rtvDesc.Format = format_;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = 0;
        if (FAILED(GetDevice(gm)->CreateRenderTargetView(texture_.Get(), &rtvDesc, &renderTargetView_))) 
        {
            throw std::runtime_error("Error creating frame buffer render target view");
        }
    }

    void FrameBuffer::CreateShaderResourceView(const GraphicsManager& gm)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = format_;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        if (FAILED(GetDevice(gm)->CreateShaderResourceView(texture_.Get(), &srvDesc, &shaderResourceView_)))
        {
            throw std::runtime_error("Error creating frame buffer shader resource view");
        }
    }

    void FrameBuffer::CreateDepthStencilView(const GraphicsManager& gm)
    {
        // create the depth stencil texture
        Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilTexture;
        D3D11_TEXTURE2D_DESC depthStencilTextureDesc{};
        depthStencilTextureDesc.Width = w_;
        depthStencilTextureDesc.Height = h_;
        depthStencilTextureDesc.MipLevels = 1;
        depthStencilTextureDesc.ArraySize = 1;
        depthStencilTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilTextureDesc.SampleDesc.Count = 1;
        depthStencilTextureDesc.SampleDesc.Quality = 0;
        depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilTextureDesc.CPUAccessFlags = 0;
        depthStencilTextureDesc.MiscFlags = 0;
        if (FAILED(GetDevice(gm)->CreateTexture2D(&depthStencilTextureDesc, NULL, &depthStencilTexture)))
        {
            throw std::runtime_error("Error creating frame buffer depth stencil texture");
        }
        // create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
        descDSV.Flags = 0;
        descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        descDSV.Texture2D.MipSlice = 0;
        if (FAILED(GetDevice(gm)->CreateDepthStencilView(depthStencilTexture.Get(), &descDSV, &depthStencilView_))) {
            throw std::runtime_error("Error creating frame buffer depth stencil view");
        }
    }


    void FrameBuffer::CreateTextureMSAA(const GraphicsManager& gm)
    {
        D3D11_TEXTURE2D_DESC texDesc{};
        texDesc.Width = w_;
        texDesc.Height = h_;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = format_;
        texDesc.SampleDesc.Count = 8;
        texDesc.SampleDesc.Quality = msaaQuality_ - 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;
        if (FAILED(GetDevice(gm)->CreateTexture2D(&texDesc, 0, &texture_)))
        {
            throw std::runtime_error("Error creating frame buffer texture");
        }

        texDesc = {};
        texDesc.Width = w_;
        texDesc.Height = h_;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = format_;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;
        if (FAILED(GetDevice(gm)->CreateTexture2D(&texDesc, 0, &resolveTexture_)))
        {
            throw std::runtime_error("Error creating frame buffer texture");
        }

    }

    void FrameBuffer::CreateRenderTargetViewMSAA(const GraphicsManager& gm)
    {
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
        rtvDesc.Format = format_;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
        rtvDesc.Texture2D.MipSlice = 0;
        if (FAILED(GetDevice(gm)->CreateRenderTargetView(texture_.Get(), &rtvDesc, &renderTargetView_)))
        {
            throw std::runtime_error("Error creating frame buffer render target view");
        }
    }

    void FrameBuffer::CreateShaderResourceViewMSAA(const GraphicsManager& gm)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = format_;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        if (FAILED(GetDevice(gm)->CreateShaderResourceView(resolveTexture_.Get(), &srvDesc, &shaderResourceView_)))
        {
            throw std::runtime_error("Error creating frame buffer shader resource view");
        }
    }

    void FrameBuffer::CreateDepthStencilViewMSAA(const GraphicsManager& gm)
    {
        // create the depth stencil texture
        Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilTexture;
        D3D11_TEXTURE2D_DESC depthStencilTextureDesc{};
        depthStencilTextureDesc.Width = w_;
        depthStencilTextureDesc.Height = h_;
        depthStencilTextureDesc.MipLevels = 1;
        depthStencilTextureDesc.ArraySize = 1;
        depthStencilTextureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilTextureDesc.SampleDesc.Count = 8;
        depthStencilTextureDesc.SampleDesc.Quality = msaaQuality_ - 1;
        depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilTextureDesc.CPUAccessFlags = 0;
        depthStencilTextureDesc.MiscFlags = 0;
        if (FAILED(GetDevice(gm)->CreateTexture2D(&depthStencilTextureDesc, NULL, &depthStencilTexture)))
        {
            throw std::runtime_error("Error creating frame buffer depth stencil texture");
        }
        // create the depth stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC descDSV{};
        descDSV.Flags = 0;
        descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
        descDSV.Texture2D.MipSlice = 0;
        if (FAILED(GetDevice(gm)->CreateDepthStencilView(depthStencilTexture.Get(), &descDSV, &depthStencilView_))) {
            throw std::runtime_error("Error creating frame buffer depth stencil view");
        }
    }

    void FrameBuffer::Resolve(const GraphicsManager& gm)
    {
        GetDeviceContext(gm)->ResolveSubresource(resolveTexture_.Get(), 0, texture_.Get(), 0, format_);
    }

}

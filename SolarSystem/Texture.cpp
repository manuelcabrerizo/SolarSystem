#include "Texture.h"
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace mc
{
    Texture::Texture(const GraphicsManager& gm, const std::string& filepath)
    {
        stbi_set_flip_vertically_on_load(false);
        int channels = 4;
        void* data = (void*)stbi_load(filepath.c_str(), &width_, &height_, &channels, 0);
        if (!data)
        {
            throw std::runtime_error("Error reading texture file");
        }


        D3D11_SUBRESOURCE_DATA subresourceData{};
        subresourceData.pSysMem = data;
        subresourceData.SysMemPitch = width_ * channels;

        D3D11_TEXTURE2D_DESC texDesc{};
        texDesc.Width = width_;
        texDesc.Height = height_;
        texDesc.MipLevels = 0;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

        if (FAILED(GetDevice(gm)->CreateTexture2D(&texDesc, nullptr, &texture_)))
        {
            throw std::runtime_error("Error creating texture");
        }

        GetDeviceContext(gm)->UpdateSubresource(texture_.Get(), 0, 0, subresourceData.pSysMem, subresourceData.SysMemPitch, 0);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
        srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = -1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        if (FAILED(GetDevice(gm)->CreateShaderResourceView(texture_.Get(), &srvDesc, &shaderResourceView_)))
        {
            throw std::runtime_error("Error creating texture shader resource view");
        }

        GetDeviceContext(gm)->GenerateMips(shaderResourceView_.Get());

        stbi_image_free(data);
    }

    void Texture::Bind(const GraphicsManager& gm, int slot)
    {
        GetDeviceContext(gm)->PSSetShaderResources(slot, 1, shaderResourceView_.GetAddressOf());
    }

    void Texture::Unbind(const GraphicsManager& gm, int slot)
    {
        ID3D11ShaderResourceView* null[] = { nullptr };
        GetDeviceContext(gm)->PSSetShaderResources(slot, 1, null);
    }
}

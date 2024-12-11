#pragma once

#include "GraphicsResource.h"
#include <string>

namespace mc
{
    class Texture : public GraphicsResource
    {
    public:
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;
        Texture(const GraphicsManager& gm, const std::string& filepath);
        void Bind(const GraphicsManager& gm, int slot);
        void Unbind(const GraphicsManager& gm, int slot);
        int GetWidth() { return width_; }
        int GetHeight() { return height_; }
    private:
        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView_;
        int width_;
        int height_;
    };
}


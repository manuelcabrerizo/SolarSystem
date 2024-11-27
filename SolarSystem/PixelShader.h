#pragma once

#include "Shader.h"

namespace mc
{
    class PixelShader : public Shader
    {
    public:
        PixelShader& operator=(const PixelShader&) = delete;
        PixelShader(const GraphicsManager& gm, const std::string& filepath);
        void Bind(const GraphicsManager& gm) override;
        void Compile(const GraphicsManager& gm) override;
    private:
        Microsoft::WRL::ComPtr<ID3D11PixelShader> shader_;
    };
}


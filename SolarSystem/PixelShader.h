#pragma once

#include "Bindable.h"
#include <string>

namespace mc
{
    class PixelShader : public Bindable
    {
    public:
        PixelShader(const GraphicsManager& gm, const std::string& filepath);
        void Bind(const GraphicsManager& gm) override;
    private:
        Microsoft::WRL::ComPtr<ID3D11PixelShader> shader;
        Microsoft::WRL::ComPtr<ID3DBlob> shaderCompiled;
    };
}


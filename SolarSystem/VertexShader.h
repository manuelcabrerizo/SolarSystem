#pragma once

#include "Bindable.h"
#include <string>

namespace mc
{
    class VertexShader : public Bindable
    {
    public:
        VertexShader(const GraphicsManager& gm, const std::string& filepath);
        void Bind(const GraphicsManager& gm) override;
    private:
        Microsoft::WRL::ComPtr<ID3D11VertexShader> shader;
        Microsoft::WRL::ComPtr<ID3DBlob> shaderCompiled;
    };
}


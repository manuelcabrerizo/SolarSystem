#pragma once

#include "Shader.h"

namespace mc
{
    class GeometryShader : public Shader
    {
    public:
        GeometryShader& operator=(const GeometryShader&) = delete;
        GeometryShader(const GraphicsManager& gm, const std::string& filepath, bool streamOutput = false);
        void Bind(const GraphicsManager& gm) override;
        void Compile(const GraphicsManager& gm) override;
    private:
        Microsoft::WRL::ComPtr<ID3D11GeometryShader> shader_;
        bool streamOutput_;
    };
}


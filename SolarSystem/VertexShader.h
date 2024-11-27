#pragma once

#include "Shader.h"



namespace mc
{
    class VertexShader : public Shader
    {
    public:
        VertexShader& operator=(const VertexShader&) = delete;
        VertexShader(const GraphicsManager& gm, const std::string& filepath);
        void Bind(const GraphicsManager& gm) override;
        void Compile(const GraphicsManager& gm) override;
    private:
        Microsoft::WRL::ComPtr<ID3D11VertexShader> shader_;
    };
}


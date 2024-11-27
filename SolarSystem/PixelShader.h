#pragma once

#include "GraphicsResource.h"
#include <string>
#include <filesystem>

namespace mc
{
    class PixelShader : public GraphicsResource
    {
    public:
        PixelShader& operator=(const PixelShader&) = delete;

        PixelShader(const GraphicsManager& gm, const std::string& filepath);
        void Bind(const GraphicsManager& gm);
        ID3DBlob* GetByteCode() const { return shaderCompiled.Get(); }
        const std::string& GetPath() const { return filepath_; }
        auto& GetLastWriteTime() const { return lastWriteTime_; }
        void Compile(const GraphicsManager& gm);
    private:
        Microsoft::WRL::ComPtr<ID3D11PixelShader> shader;
        Microsoft::WRL::ComPtr<ID3DBlob> shaderCompiled;
        std::filesystem::file_time_type lastWriteTime_;
        std::string filepath_;
    };
}


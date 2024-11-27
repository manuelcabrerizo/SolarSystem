#pragma once

#include "GraphicsResource.h"
#include <string>
#include <filesystem>

namespace mc
{
    class VertexShader : public GraphicsResource
    {
    public:
        VertexShader& operator=(const VertexShader&) = delete;

        VertexShader(const GraphicsManager& gm, const std::string& filepath);
        void Bind(const GraphicsManager& gm);
        ID3DBlob* GetByteCode() const { return shaderCompiled.Get(); }
        const std::string& GetPath() const { return filepath_; }
        auto& GetLastWriteTime() const { return lastWriteTime_; }
        void Compile(const GraphicsManager& gm);
    private:
        Microsoft::WRL::ComPtr<ID3D11VertexShader> shader;
        Microsoft::WRL::ComPtr<ID3DBlob> shaderCompiled;
        std::filesystem::file_time_type lastWriteTime_;
        std::string filepath_;
    };
}


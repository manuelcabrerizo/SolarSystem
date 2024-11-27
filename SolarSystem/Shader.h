#pragma once

#include "GraphicsResource.h"
#include <string>
#include <filesystem>

namespace mc
{
    class Shader : public GraphicsResource
    {
    public:
        virtual ~Shader() = default;

        virtual void Bind(const GraphicsManager& gm) = 0;
        virtual void Compile(const GraphicsManager& gm) = 0;
        ID3DBlob* GetByteCode() const { return shaderCompiled_.Get(); }
        const std::string& GetPath() const { return filepath_; }
        auto& GetLastWriteTime() const { return lastWriteTime_; }

    protected:
        Microsoft::WRL::ComPtr<ID3DBlob> shaderCompiled_;
        std::filesystem::file_time_type lastWriteTime_;
        std::string filepath_;
    };
}
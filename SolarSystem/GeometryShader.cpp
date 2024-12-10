#include "GeometryShader.h"
#include "Utils.h"
#include "GraphicsManager.h"

#include <d3dcompiler.h>
#include <iostream>

namespace mc
{
    GeometryShader::GeometryShader(const GraphicsManager& gm, const std::string& filepath, bool streamOutput)
    {
        streamOutput_ = streamOutput;
        filepath_ = filepath;
        Compile(gm);
    }

    void GeometryShader::Compile(const GraphicsManager& gm)
    {
        Microsoft::WRL::ComPtr<ID3DBlob> errorShader;

        File shaderFile(filepath_);

        D3DCompile(shaderFile.data, shaderFile.size,
            0, 0, 0, "gs_main", "gs_5_0",
            D3DCOMPILE_ENABLE_STRICTNESS, 0,
            &shaderCompiled_, &errorShader);
        if (errorShader != 0)
        {
            char* errorString = (char*)errorShader->GetBufferPointer();
            std::cout << "Error compiling GEOMETRY SHADER: " << filepath_ << "\n";
            std::cout << errorString << "\n";
        }
        else
        {
            if (streamOutput_)
            {
                D3D11_SO_DECLARATION_ENTRY pDecl[] = {
                    { 0, "POSITION",   0, 0, 3, 0 },
                    { 0, "TEXCOORD",   0, 0, 3, 0 },
                    { 0, "TEXCOORD",   1, 0, 2, 0 },
                    { 0, "TEXCOORD",   2, 0, 1, 0 },
                    { 0, "TEXCOORD",   3, 0, 1, 0 }
                };
                GetDevice(gm)->CreateGeometryShaderWithStreamOutput(
                    shaderCompiled_->GetBufferPointer(),
                    shaderCompiled_->GetBufferSize(),
                    pDecl, 5,
                    nullptr, 0, 0, nullptr,
                    &shader_);
            }
            else
            {
                GetDevice(gm)->CreateGeometryShader(
                    shaderCompiled_->GetBufferPointer(),
                    shaderCompiled_->GetBufferSize(), 0,
                    &shader_);
            }
        }

        // save the write time
        try
        {
            auto p = std::filesystem::current_path() / filepath_;
            lastWriteTime_ = std::filesystem::last_write_time(p);
        }
        catch (const std::exception& e)
        {
            std::cout << "Error: " << e.what() << "\n";
        }
    }

    void GeometryShader::Bind(const GraphicsManager& gm)
    {
        GetDeviceContext(gm)->GSSetShader(shader_.Get(), 0, 0);
    }
}
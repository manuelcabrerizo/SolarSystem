#include "VertexShader.h"
#include "Utils.h"
#include "GraphicsManager.h"

#include <d3dcompiler.h>
#include <iostream>

namespace mc
{
    VertexShader::VertexShader(const GraphicsManager& gm, const std::string& filepath)
    {
        filepath_ = filepath;
        Compile(gm);
    }

    void VertexShader::Compile(const GraphicsManager& gm)
    {
        Microsoft::WRL::ComPtr<ID3DBlob> errorShader;

        File shaderFile(filepath_);

        D3DCompile(shaderFile.data, shaderFile.size,
            0, 0, 0, "vs_main", "vs_5_0",
            D3DCOMPILE_ENABLE_STRICTNESS, 0,
            &shaderCompiled_, &errorShader);
        if (errorShader != 0)
        {
            char* errorString = (char*)errorShader->GetBufferPointer();
            std::cout << "Error compiling VERTEX SHADER: " << filepath_ << "\n";
            std::cout << errorString << "\n";
        }
        else
        {
            GetDevice(gm)->CreateVertexShader(
                shaderCompiled_->GetBufferPointer(),
                shaderCompiled_->GetBufferSize(), 0,
                &shader_);
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

    void VertexShader::Bind(const GraphicsManager& gm)
    {
        GetDeviceContext(gm)->VSSetShader(shader_.Get(), 0, 0);
    }
}
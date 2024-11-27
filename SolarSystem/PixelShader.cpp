#include "PixelShader.h"
#include "Utils.h"
#include "GraphicsManager.h"

#include <d3dcompiler.h>

#include <iostream>

namespace mc
{
    PixelShader::PixelShader(const GraphicsManager& gm, const std::string& filepath)
        : filepath_(filepath)
    {
        Compile(gm);
    }

    void PixelShader::Compile(const GraphicsManager& gm)
    {
        Microsoft::WRL::ComPtr<ID3DBlob> errorShader;

        File shaderFile(filepath_);

        D3DCompile(shaderFile.data, shaderFile.size,
            0, 0, 0, "fs_main", "ps_5_0",
            D3DCOMPILE_ENABLE_STRICTNESS, 0,
            &shaderCompiled, &errorShader);
        if (errorShader != 0)
        {
            char* errorString = (char*)errorShader->GetBufferPointer();
            std::cout << "Error conpiling PIXEL SHADER: " << filepath_ << "\n";
            std::cout << errorString << "\n";
        }
        else
        {
            GetDevice(gm)->CreatePixelShader(
                shaderCompiled->GetBufferPointer(),
                shaderCompiled->GetBufferSize(), 0,
                &shader);
        }

        // save the write time
        auto p = std::filesystem::current_path() / filepath_;
        lastWriteTime_ = std::filesystem::last_write_time(p);
    }

    void PixelShader::Bind(const GraphicsManager& gm)
    {
        GetDeviceContext(gm)->PSSetShader(shader.Get(), 0, 0);
    }
}

#include "VertexShader.h"
#include "Utils.h"
#include "GraphicsManager.h"

#include <d3dcompiler.h>
#include <iostream>

namespace mc
{
    VertexShader::VertexShader(const GraphicsManager& gm, const std::string& filepath)
    {
        Microsoft::WRL::ComPtr<ID3DBlob> errorShader;

        File shaderFile(filepath);

        D3DCompile(shaderFile.data, shaderFile.size,
            0, 0, 0, "vs_main", "vs_5_0",
            D3DCOMPILE_ENABLE_STRICTNESS, 0,
            &shaderCompiled, &errorShader);
        if (errorShader != 0)
        {
            char* errorString = (char*)errorShader->GetBufferPointer();
            std::cout << "Error conpiling VERTEX SHADER: " << filepath << "\n";
            std::cout << errorString << "\n";
        }
        else
        {
            GetDevice(gm)->CreateVertexShader(
                shaderCompiled->GetBufferPointer(),
                shaderCompiled->GetBufferSize(), 0,
                &shader);
        }
    }

    void VertexShader::Bind(const GraphicsManager& gm)
    {
        GetDeviceContext(gm)->VSSetShader(shader.Get(), 0, 0);
    }
}
#pragma once

#include "GraphicsResource.h"

namespace mc
{
    struct InputLayoutDesc {
        D3D11_INPUT_ELEMENT_DESC desc[16];
        int count = 0;
    };


    class VertexShader;

    class InputLayout : public GraphicsResource
    {
    public:
        InputLayout(const InputLayout&) = delete;
        InputLayout& operator=(const InputLayout&) = delete;

        InputLayout(const GraphicsManager& gm, const VertexShader& shader, InputLayoutDesc& desc);
        void Bind(const GraphicsManager& gm);
    private:
        Microsoft::WRL::ComPtr <ID3D11InputLayout> layout;
    };
}


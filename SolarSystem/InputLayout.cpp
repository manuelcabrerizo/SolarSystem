#include "InputLayout.h"
#include "VertexShader.h"
#include <stdexcept>

namespace mc
{
    InputLayout::InputLayout(const GraphicsManager& gm, const VertexShader& shader, InputLayoutDesc& desc)
    {
        if (FAILED(GetDevice(gm)->CreateInputLayout(
            desc.desc,
            desc.count,
            shader.GetByteCode()->GetBufferPointer(),
            shader.GetByteCode()->GetBufferSize(),
            &layout)))
        {
            throw std::runtime_error("Error creating input layout");
        }
    }

    void InputLayout::Bind(const GraphicsManager& gm)
    {
        GetDeviceContext(gm)->IASetInputLayout(layout.Get());
    }
}
#include "IndexBuffer.h"
#include <stdexcept>

namespace mc
{
    IndexBuffer::IndexBuffer(const GraphicsManager& gm, unsigned int* indices, unsigned int count)
        : indexCount(count), format(DXGI_FORMAT_R32_UINT)
    {
        D3D11_BUFFER_DESC indexDesc;
        ZeroMemory(&indexDesc, sizeof(indexDesc));
        indexDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexDesc.ByteWidth = sizeof(unsigned int) * count;

        D3D11_SUBRESOURCE_DATA subresourceData;
        ZeroMemory(&subresourceData, sizeof(subresourceData));
        subresourceData.pSysMem = indices;
        if (FAILED(GetDevice(gm)->CreateBuffer(&indexDesc, &subresourceData, &buffer)))
        {
            throw std::runtime_error("Error creating vertex buffer");
        }
    }

    void IndexBuffer::Bind(const GraphicsManager& gm)
    {
        GetDeviceContext(gm)->IASetIndexBuffer(buffer.Get(), format, 0);
    }
}
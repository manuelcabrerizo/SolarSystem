#include "VertexBuffer.h"
#include <stdexcept>

namespace mc
{
    VertexBuffer::VertexBuffer(const GraphicsManager& gm, void* vertices, unsigned int count, unsigned int stride)
        : verticesCount(count), stride(stride), offset(0)
    {
        D3D11_BUFFER_DESC vertexDesc;
        ZeroMemory(&vertexDesc, sizeof(vertexDesc));
        vertexDesc.Usage = D3D11_USAGE_DEFAULT;
        vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexDesc.ByteWidth = stride * count;

        D3D11_SUBRESOURCE_DATA subresourceData;
        ZeroMemory(&subresourceData, sizeof(subresourceData));
        subresourceData.pSysMem = vertices;
        if (FAILED(GetDevice(gm)->CreateBuffer(&vertexDesc, &subresourceData, &buffer)))
        {
            throw std::runtime_error("Error creating vertex buffer");
        }
    }

    void VertexBuffer::Bind(const GraphicsManager& gm)
    {
        GetDeviceContext(gm)->IASetVertexBuffers(0, 1, buffer.GetAddressOf(), &stride, &offset);
    }
}
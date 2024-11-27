#pragma once

#include "GraphicsResource.h"
#include <stdexcept>

namespace mc
{
    enum class ConstBufferBind
    {
        Vertex,
        Pixel
    };

    template<typename Type>
    class ConstBuffer : public GraphicsResource
    {
    public:
        ConstBuffer(const ConstBuffer&) = delete;
        ConstBuffer& operator=(const ConstBuffer&) = delete;

        ConstBuffer(const GraphicsManager& gm, ConstBufferBind bindTo, const Type& data, unsigned int slot)
            : slot(slot), bindTo(bindTo)
        {
            D3D11_BUFFER_DESC bufferDesc;
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
            bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            bufferDesc.MiscFlags = 0;
            bufferDesc.ByteWidth = sizeof(data);
            bufferDesc.StructureByteStride = 0;

            D3D11_SUBRESOURCE_DATA subresourceData;
            ZeroMemory(&subresourceData, sizeof(subresourceData));
            subresourceData.pSysMem = &data;
            if (FAILED(GetDevice(gm)->CreateBuffer(&bufferDesc, &subresourceData, &buffer)))
            {
                throw std::runtime_error("Error creating const buffer");
            }
        }
        void Update(const GraphicsManager& gm, const Type& data)
        {
            D3D11_MAPPED_SUBRESOURCE mappedSubResource;
            if(FAILED(GetDeviceContext(gm)->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubResource)))
            {
                throw std::runtime_error("Error updating const buffer");
            }
            memcpy(mappedSubResource.pData, &data, sizeof(data));
            GetDeviceContext(gm)->Unmap(buffer.Get(), 0);
        }
        
        void Bind(const GraphicsManager& gm)
        {
            switch (bindTo)
            {
            case ConstBufferBind::Vertex: GetDeviceContext(gm)->VSSetConstantBuffers(slot, 1, buffer.GetAddressOf()); break;
            case ConstBufferBind::Pixel:  GetDeviceContext(gm)->PSSetConstantBuffers(slot, 1, buffer.GetAddressOf()); break;
            }
        }
    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
        unsigned int  slot;
        ConstBufferBind bindTo;
    };
}


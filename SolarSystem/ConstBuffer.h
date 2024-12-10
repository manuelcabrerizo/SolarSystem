#pragma once

#include "GraphicsResource.h"
#include <stdexcept>

namespace mc
{
    enum ConstBufferBind
    {
        BIND_TO_VS = (1 << 0),
        BIND_TO_PS = (1 << 1),
        BIND_TO_GS = (1 << 2)
    };

    template<typename Type>
    class ConstBuffer : public GraphicsResource
    {
    public:
        ConstBuffer(const ConstBuffer&) = delete;
        ConstBuffer& operator=(const ConstBuffer&) = delete;

        ConstBuffer(const GraphicsManager& gm, unsigned int bindTo, const Type& data, unsigned int slot)
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
            if (bindTo & ConstBufferBind::BIND_TO_VS)
            {
                GetDeviceContext(gm)->VSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
            }
            if (bindTo & ConstBufferBind::BIND_TO_PS)
            {
                GetDeviceContext(gm)->PSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
            }
            if (bindTo & ConstBufferBind::BIND_TO_GS)
            {
                GetDeviceContext(gm)->GSSetConstantBuffers(slot, 1, buffer.GetAddressOf());
            }
        }
    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
        unsigned int  slot;
        unsigned int bindTo;
    };
}


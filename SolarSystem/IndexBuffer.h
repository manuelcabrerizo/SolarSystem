#pragma once

#include "GraphicsResource.h"

namespace mc
{
    class IndexBuffer : public GraphicsResource
    {
    public:
        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;

        IndexBuffer(const GraphicsManager& gm, unsigned int *indices, unsigned int count);
        void Bind(const GraphicsManager& gm);
    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
        unsigned int indexCount{};
        DXGI_FORMAT format{};
    };
}


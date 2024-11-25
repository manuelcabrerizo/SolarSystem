#pragma once

#include "Bindable.h"

namespace mc
{
    class VertexBuffer : public Bindable
    {
    public:
        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        VertexBuffer(const GraphicsManager& gm, void *vertices, unsigned int count, unsigned int stride);
        void Bind(const GraphicsManager& gm) override;
    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
        unsigned int verticesCount;
        unsigned int stride;
        unsigned int offset;
    };
}


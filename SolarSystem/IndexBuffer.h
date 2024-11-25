#pragma once

#include "Bindable.h"

namespace mc
{
    class IndexBuffer : public Bindable
    {
    public:
        IndexBuffer(const GraphicsManager& gm, unsigned int *indices, unsigned int count);
        void Bind(const GraphicsManager& gm) override;
    private:
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
        unsigned int indexCount{};
        DXGI_FORMAT format{};
    };
}


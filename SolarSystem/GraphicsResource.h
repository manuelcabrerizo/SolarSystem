#pragma once

#include "GraphicsManager.h"

namespace mc
{
    class GraphicsResource
    {
    protected:
        static ID3D11Device* GetDevice(const GraphicsManager& gm);
        static ID3D11DeviceContext* GetDeviceContext(const GraphicsManager& gm);
    };
}


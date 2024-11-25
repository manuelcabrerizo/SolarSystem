#include "GraphicsResource.h"

namespace mc
{
    ID3D11Device* GraphicsResource::GetDevice(const GraphicsManager& gm)
    {
        return gm.device_.Get();
    }

    ID3D11DeviceContext* GraphicsResource::GetDeviceContext(const GraphicsManager& gm)
    {
        return gm.deviceContext_.Get();
    }
}
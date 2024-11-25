#pragma once

#include "GraphicsResource.h"

namespace mc
{
    class GraphicsManager;

    class Bindable : public GraphicsResource
    {
    public:
        virtual void Bind(const GraphicsManager& gm) = 0;
        virtual ~Bindable() = default;
    };
}


#pragma once

#include "GraphicsResource.h"
#include "Bindable.h"
#include <vector>
#include <memory>

namespace mc
{
    class Drawable : public GraphicsResource
    {
    public:
        virtual void Draw(const GraphicsManager& gm) = 0;
        virtual ~Drawable() = default;
    protected:
        std::vector<std::unique_ptr<Bindable>> bindables;
    };
}


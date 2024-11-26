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
        virtual void Draw(const mc::GraphicsManager& gm, unsigned int count, bool indexed) = 0;
        virtual ~Drawable() = default;
    protected:
        std::vector<Bindable *> bindables_;
    };
}


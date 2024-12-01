#pragma once

#include "GraphicsResource.h"

namespace mc
{
    class VertexBuffer;
    class InputLayout;
    class IndexBuffer;

    class Mesh : public GraphicsResource
    {
    public:
        Mesh(const GraphicsManager& gm,
            VertexBuffer* vb = nullptr,
            InputLayout* il = nullptr,
            IndexBuffer* ib = nullptr,
            size_t count = 0, bool indexed = false);

        void Draw(const mc::GraphicsManager& gm);
    private:
        VertexBuffer* vb_{ nullptr };
        InputLayout* il_{ nullptr };
        IndexBuffer* ib_{ nullptr };
        size_t count_{0};
        bool indexed_{false};
    };
}


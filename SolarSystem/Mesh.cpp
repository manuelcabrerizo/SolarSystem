#include "Mesh.h"

#include "VertexBuffer.h"
#include "InputLayout.h"
#include "IndexBuffer.h"

namespace mc
{
    Mesh::Mesh(const GraphicsManager& gm, VertexBuffer* vb, InputLayout* il, IndexBuffer* ib,
        size_t count, bool indexed)
        : vb_(vb), il_(il), ib_(ib), count_(count), indexed_(indexed) { }

    void Mesh::Draw(const GraphicsManager& gm)
    {
        if (vb_)
        {
            vb_->Bind(gm);
        }
        if (il_)
        {
            il_->Bind(gm);
        }
        if (ib_)
        {
            ib_->Bind(gm);
        }

        GetDeviceContext(gm)->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        if (indexed_)
        {
            GetDeviceContext(gm)->DrawIndexed(static_cast<UINT>(count_), 0, 0);
        }
        else
        {
            GetDeviceContext(gm)->Draw(static_cast<UINT>(count_), 0);
        }
    }
}
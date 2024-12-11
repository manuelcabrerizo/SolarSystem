#include "Text.h"
#include <stdexcept>

#include "Texture.h"
#include "Shader.h"

namespace mc
{
    Text::Text(const GraphicsManager& gm, unsigned int size, Texture& texture, unsigned int tileW, unsigned int tileH,
        Shader *vertShader, Shader *pixelShader)
        : texture_(texture), used_(0), size_(size), tileW_(tileW), tileH_(tileH),
          vertShader_(vertShader), pixelShader_(pixelShader)
    {
        quads_.resize(size);
        GenerateUVs();

        D3D11_BUFFER_DESC vertexDesc;
        ZeroMemory(&vertexDesc, sizeof(vertexDesc));
        vertexDesc.Usage = D3D11_USAGE_DYNAMIC;
        vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        vertexDesc.ByteWidth = size * sizeof(Quad);
        if (FAILED(GetDevice(gm)->CreateBuffer(&vertexDesc, NULL, &vertexBuffer_))) {
            throw std::runtime_error("Error: cannot create Batch GPU buffer.");
        }
    }

    Text::~Text()
    {
    }

    void Text::Write(const GraphicsManager& gm, const std::string& text, float x, float y, float w, float h)
    {
        float offset = 0;
        for (char letter : text)
        {
            if (letter >= 32 && letter <= 126)
            {
                int index = letter - 32;
                UV uv = uvs_[index];
                Quad quad{};
                quad.vertices[0].position = XMFLOAT3(offset + x, y, -1);
                quad.vertices[0].uv = XMFLOAT2(uv.ux, uv.uy);
                quad.vertices[1].position = XMFLOAT3(offset + x + w, y, -1);
                quad.vertices[1].uv = XMFLOAT2(uv.vx, uv.uy);
                quad.vertices[2].position = XMFLOAT3(offset + x, y - h, -1);
                quad.vertices[2].uv = XMFLOAT2(uv.ux, uv.vy);

                quad.vertices[3].position = XMFLOAT3(offset + x, y - h, -1);
                quad.vertices[3].uv = XMFLOAT2(uv.ux, uv.vy);
                quad.vertices[4].position = XMFLOAT3(offset + x + w, y, -1);
                quad.vertices[4].uv = XMFLOAT2(uv.vx, uv.uy);
                quad.vertices[5].position = XMFLOAT3(offset + x + w, y - h, -1);
                quad.vertices[5].uv = XMFLOAT2(uv.vx, uv.vy);
                quads_[used_++] = quad;
                if (used_ == size_)
                {
                    Render(gm);
                }
            }
            offset += w;
        }
    }

    void Text::Render(const GraphicsManager& gm)
    {
        D3D11_MAPPED_SUBRESOURCE bufferData;


        ZeroMemory(&bufferData, sizeof(bufferData));
        GetDeviceContext(gm)->Map(vertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &bufferData);
        memcpy(bufferData.pData, quads_.data(), used_ * sizeof(Quad));
        GetDeviceContext(gm)->Unmap(vertexBuffer_.Get(), 0);

        texture_.Bind(gm, 0);
        vertShader_->Bind(gm);
        pixelShader_->Bind(gm);

        unsigned int stride = sizeof(Vertex);
        unsigned int offset = 0;
        GetDeviceContext(gm)->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);
        GetDeviceContext(gm)->Draw(used_ * 6, 0);
        texture_.Unbind(gm, 0);

        used_ = 0;
    }

    void Text::GenerateUVs()
    {
        int col = texture_.GetWidth() / tileW_;
        int row = texture_.GetHeight() / tileH_;
        float incX = (float)tileW_ / (float)texture_.GetWidth();
        float incY = (float)tileH_ / (float)texture_.GetHeight();

        uvs_.resize(col * row);

        float x = 0.0f;
        float y = 0.0f;

        int counter = 0;
        for (int j = 0; j < row; j++)
        {
            for (int i = 0; i < col; i++)
            {
                UV uv = { x, x + incX, y,  y + incY };
                uvs_[counter++] = uv;
                x += incX;
            }
            x = 0;
            y += incY;
        }

        int StopHere = 0;
    }
}
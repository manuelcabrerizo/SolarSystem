#pragma once

#include "GraphicsResource.h"
#include "GeometryGenerator.h"
#include <vector>

namespace mc
{
    class Texture;
    class Shader;

    struct UV
    {
        float ux, vx;
        float uy, vy;
    };

    struct Quad
    {
        Vertex vertices[6];
    };

    class Text : public GraphicsResource
    {
    public:
        Text(const Text&) = delete;
        Text& operator=(const Text&) = delete;

        Text(const GraphicsManager& gm, unsigned int maxLetters, Texture& texture, unsigned int tileW, unsigned int tileH,
            Shader* vertShader, Shader* pixelShader);
        ~Text();

        void Write(const GraphicsManager& gm, const std::string& text, float x, float y, float w, float h);
        void Render(const GraphicsManager& gm);
    private:
        void GenerateUVs();

        Texture& texture_;
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
        std::vector<Quad> quads_;
        std::vector<UV> uvs_;

        Shader* vertShader_{ nullptr };
        Shader* pixelShader_{ nullptr };

        unsigned int used_{ 0 };
        unsigned int size_{ 0 };
        unsigned int tileW_{ 0 };
        unsigned int tileH_{ 0 };
    };
}


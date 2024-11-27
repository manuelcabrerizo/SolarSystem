#pragma once

#include "VertexShader.h"
#include "PixelShader.h"

#include <vector>
#include <string>

namespace mc
{
    using Shader = unsigned int;
    class ShaderManager
    {
    public:
        ShaderManager();
        ~ShaderManager() = default;
        ShaderManager(const ShaderManager&) = delete;
        ShaderManager& operator=(const ShaderManager&) = delete;

        Shader AddVertexShader(const GraphicsManager& gm, const std::string& filepath);
        Shader AddPixelShader(const GraphicsManager& gm, const std::string& filepath);

        VertexShader& GetVertexShader(Shader shader);
        PixelShader& GetPixelShader(Shader shader);

        void HotReaload(const GraphicsManager& gm);

    private:
        // TODO: unify this vector into one
        std::vector<VertexShader> vertexShaders_;
        std::vector<PixelShader> pixelShaders_;
    };
}


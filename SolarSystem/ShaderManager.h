#pragma once

#include "VertexShader.h"
#include "PixelShader.h"
#include "GeometryShader.h"

#include <unordered_map>
#include <string>

namespace mc
{
    class ShaderManager
    {
    public:
        ShaderManager();
        ~ShaderManager() = default;
        ShaderManager(const ShaderManager&) = delete;
        ShaderManager& operator=(const ShaderManager&) = delete;

        void AddVertexShader(const std::string& name, const GraphicsManager& gm, const std::string& filepath);
        void AddPixelShader(const std::string& name, const GraphicsManager& gm, const std::string& filepath);
        void AddGeometryShader(const std::string& name, const GraphicsManager& gm, const std::string& filepath, bool streamOuput = false);
        Shader *Get(const std::string& name);

        void HotReaload(const GraphicsManager& gm);

    private:
        std::unordered_map<std::string, std::unique_ptr<Shader>> shaders_;
    };
}


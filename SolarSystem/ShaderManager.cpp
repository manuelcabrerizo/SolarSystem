#include "ShaderManager.h"

namespace mc
{
    ShaderManager::ShaderManager() {}

    Shader ShaderManager::AddVertexShader(const GraphicsManager& gm, const std::string& filepath)
    {
        vertexShaders_.emplace_back(gm, filepath);
        return vertexShaders_.size() - 1;
    }

    Shader ShaderManager::AddPixelShader(const GraphicsManager& gm, const std::string& filepath)
    {
        pixelShaders_.emplace_back(gm, filepath);
        return pixelShaders_.size() - 1;

    }
    

    VertexShader& ShaderManager::GetVertexShader(Shader shader)
    {
        return vertexShaders_[shader];
    }

    PixelShader& ShaderManager::GetPixelShader(Shader shader)
    {
        return pixelShaders_[shader];
    }
    

    void ShaderManager::HotReaload(const GraphicsManager& gm)
    {
        for (auto& shader : vertexShaders_)
        {
            auto p = std::filesystem::current_path() / shader.GetPath();
            std::filesystem::file_time_type ftime = std::filesystem::last_write_time(p);
            if (ftime != shader.GetLastWriteTime())
            {
                shader.Compile(gm);
            }
        }
        for (auto& shader : pixelShaders_)
        {
            auto p = std::filesystem::current_path() / shader.GetPath();
            std::filesystem::file_time_type ftime = std::filesystem::last_write_time(p);
            if (ftime != shader.GetLastWriteTime())
            {
                shader.Compile(gm);
            }
        }       
    }
}
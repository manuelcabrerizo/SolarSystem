#include "ShaderManager.h"
#include <iostream>

namespace mc
{
    ShaderManager::ShaderManager() {}

    void ShaderManager::AddVertexShader(const std::string& name, const GraphicsManager& gm, const std::string& filepath)
    {
        shaders_.emplace(std::make_pair(name, std::make_unique<VertexShader>(gm, filepath)));
    }

    void ShaderManager::AddPixelShader(const std::string& name, const GraphicsManager& gm, const std::string& filepath)
    {
        shaders_.emplace(std::make_pair(name, std::make_unique<PixelShader>(gm, filepath)));
    }

    void ShaderManager::AddGeometryShader(const std::string& name, const GraphicsManager& gm, const std::string& filepath, bool streamOuput)
    {
        shaders_.emplace(std::make_pair(name, std::make_unique<GeometryShader>(gm, filepath, streamOuput)));
    }

    Shader* ShaderManager::Get(const std::string& name)
    {
        return shaders_.at(name).get();
    }    

    void ShaderManager::HotReaload(const GraphicsManager& gm)
    {
        for (auto& pair : shaders_)
        {
            try
            {
                auto& shader = pair.second;
                auto p = std::filesystem::current_path() / shader->GetPath();
                std::filesystem::file_time_type ftime = std::filesystem::last_write_time(p);
                if (ftime != shader->GetLastWriteTime())
                {
                    shader->Compile(gm);
                }
            }
            catch (const std::exception& e)
            {
                std::cout << "Error: " << e.what() << "\n";
            }
        }
    }
}
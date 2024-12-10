#pragma once

#include "GraphicsManager.h"
#include "InputManager.h"
#include "ShaderManager.h"

#include "ConstBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "InputLayout.h"
#include "FrameBuffer.h"
#include "Texture.h"

#include "GeometryGenerator.h"
#include "Mesh.h"
#include "Camera.h"
#include "ParticleSystem.h"

namespace mc
{
    class Engine
    {
    public:
        Engine(const std::string& title, int width, int height);
        ~Engine();

        GraphicsManager& GetGraphicsManager();
        InputManager& GetInputManager();
        ShaderManager& GetShaderManager();
        bool IsRunning();
        static bool isRunning;
    private:
        InputManager inputManager_;
        Window window_;
        GraphicsManager graphicsManager_;
        ShaderManager shaderManager_;
    };
}


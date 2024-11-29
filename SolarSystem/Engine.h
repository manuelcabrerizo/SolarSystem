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
#include "Camera.h"

namespace mc
{
    class Engine
    {
    public:
        Engine(const std::string& title, int width, int height);
        ~Engine();
        bool IsRunning();
        GraphicsManager& GetGraphicsManager();
        InputManager& GetInputManager();
        ShaderManager& GetShaderManager();
        static bool isRunning;
    private:
        InputManager inputManager_;
        Window window_;
        GraphicsManager graphicsManager_;
        ShaderManager shaderManager_;
    };
}


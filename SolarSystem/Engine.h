#pragma once

#include "GraphicsManager.h"
#include "InputManager.h"

#include "VertexShader.h"
#include "PixelShader.h"
#include "ConstBuffer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "InputLayout.h"
#include "FrameBuffer.h"
#include "Texture.h"

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
        static bool isRunning;
    private:
        InputManager inputManager_;
        Window window_;
        GraphicsManager graphicsManager_;
    };
}


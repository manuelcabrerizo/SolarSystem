#pragma once

#include "GraphicsManager.h"
#include "InputManager.h"

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


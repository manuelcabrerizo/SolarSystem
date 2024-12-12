#include "Engine.h"

namespace mc
{
    bool Engine::isRunning = true;

    Engine::Engine(const std::string& title, int width, int height)
        : inputManager_{},
          window_{ title, width, height,  
                    reinterpret_cast<std::size_t>(&inputManager_)},
          graphicsManager_{ window_ },
        shaderManager_{},
        audioManager_{}
    {
    }

    Engine::~Engine() {}

    GraphicsManager& Engine::GetGraphicsManager() 
    {
        return graphicsManager_;
    }

    InputManager& Engine::GetInputManager()
    {
        return inputManager_;
    }

    ShaderManager& Engine::GetShaderManager()
    {
        return shaderManager_;
    }

    AudioManager& Engine::GetAudioManager()
    {
        return audioManager_;
    }

    bool Engine::IsRunning()
    {
        window_.ProcessEvents(); // TODO: update the input in here ...
        return isRunning;
    }


}
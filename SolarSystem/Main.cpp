#include <iostream>

#include "Engine.h"

void Demo()
{
    mc::Engine engine{ "SolarSystem", 800, 600 };
    mc::GraphicsManager& gm = engine.GetGraphicsManager();
    mc::InputManager& im = engine.GetInputManager();


    mc::Shader vertexShader = gm.CreateVertexShader("assets/vertex/vert.hlsl");

    while (engine.IsRunning())
    {
        if (im.KeyJustDown(mc::KEY_A))
        {
            std::cout << "A just down\n";
        }

        if (im.KeyJustUp(mc::KEY_A))
        {
            std::cout << "A just up\n";
        }

        gm.Clear(0, 1.0f, 0);

        gm.Present();
    }
}

int main()
{
    try
    {
        Demo();
    }
    catch (std::exception& e)
    {
        std::cout << "Error: " << e.what() << "\n";
    }
    return 0;
}
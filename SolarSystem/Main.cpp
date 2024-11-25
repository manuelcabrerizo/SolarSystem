#include <iostream>

#include "Engine.h"

struct Matrix
{
    float x00, y01, z02, w03;
    float x10, y11, z12, w13;
    float x20, y21, z22, w23;
    float x30, y31, z32, w33;
};

void Demo()
{
    mc::Engine engine{ "SolarSystem", 800, 600 };
    mc::GraphicsManager& gm = engine.GetGraphicsManager();
    mc::InputManager& im = engine.GetInputManager();

    mc::VertexShader vertexShader(gm, "assets/vertex/vert.hlsl");
    mc::PixelShader   pixelShader(gm, "assets/pixel/pixel.hlsl");

    Matrix mat{};
    mc::ConstBuffer<Matrix> constBuffer(gm, mc::ConstBufferBind::Vertex, mat, 0);

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
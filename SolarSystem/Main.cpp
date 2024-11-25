#include <iostream>

#include <DirectXMath.h>
#include "Engine.h"

#include "Drawable.h"


using namespace DirectX;

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT3 normal;
    XMFLOAT4 color;
};

struct PerFrameConstBuffer
{
    XMMATRIX model;
    XMMATRIX view;
    XMMATRIX proj;
};

class Quad : public mc::Drawable
{
public:
    Quad(const mc::GraphicsManager& gm, const mc::VertexShader& shader)
    {
        // create the vertex buffer
        Vertex vertices[6] = {
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
            {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
            {{ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 0.0f, 1.0f}},
            {{ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f, 1.0f, 1.0f}},
            {{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 1.0f, 1.0f}}
        };
        bindables.emplace_back(std::make_unique<mc::VertexBuffer>(gm, vertices, 6, sizeof(Vertex)));
        // create the input layout
        mc::InputLayoutDesc desc = {
            {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
            },
            3
        };
        bindables.emplace_back(std::make_unique<mc::InputLayout>(gm, shader, desc));

        int StopHere = 0;
    }

    void Draw(const mc::GraphicsManager& gm) override
    {
        for (auto& bindable : bindables)
        {
            bindable.get()->Bind(gm);
        }
        GetDeviceContext(gm)->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        GetDeviceContext(gm)->Draw(6, 0);
    }
};

void Demo()
{
    mc::Engine engine{ "SolarSystem", 800, 600 };
    mc::GraphicsManager& gm = engine.GetGraphicsManager();
    mc::InputManager& im = engine.GetInputManager();

    // Camera Variables
    XMFLOAT3 position(0.0f, 0.0f, -10.0f);
    XMFLOAT3 target(0.0f, 0.0f, 0.0f);
    XMFLOAT3 up(0.0f, 1.0f, 0.0f);

    // Init Shaders
    mc::VertexShader vertexShader(gm, "assets/vertex/vert.hlsl");
    mc::PixelShader   pixelShader(gm, "assets/pixel/pixel.hlsl");

    // Init a Const Buffer
    PerFrameConstBuffer perFrameConstBuffer{};
    perFrameConstBuffer.view = XMMatrixIdentity();// XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&target), XMLoadFloat3(&up));
    perFrameConstBuffer.model = XMMatrixIdentity();
    perFrameConstBuffer.proj = XMMatrixIdentity();// XMMatrixPerspectiveFovLH((60.0f / 180.0f) * XM_PI, 800.0f / 600.0f, 1.0f, 100.0f);
    mc::ConstBuffer<PerFrameConstBuffer> constBuffer(gm, mc::ConstBufferBind::Vertex, perFrameConstBuffer, 0);
    
    // Create a quad
    Quad quad(gm, vertexShader);

    vertexShader.Bind(gm);
    pixelShader.Bind(gm);
    constBuffer.Bind(gm);

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

        quad.Draw(gm);

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
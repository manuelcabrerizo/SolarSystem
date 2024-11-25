#include <iostream>

#include <DirectXMath.h>
#include "Engine.h"
#include "Drawable.h"

#include "GeometryGenerator.h"

using namespace DirectX;

struct PerFrameConstBuffer
{
    XMMATRIX model;
    XMMATRIX view;
    XMMATRIX proj;
};

class Mesh : public mc::Drawable
{
public:
    Mesh(const mc::GraphicsManager& gm,
        mc::VertexBuffer* vb = nullptr,
        mc::InputLayout* il = nullptr,
        mc::IndexBuffer* ib = nullptr)
    {
        if (vb != nullptr)
        {
            bindables_.push_back(vb);
        }
        if (il != nullptr)
        {
            bindables_.push_back(il);
        }
        if (ib != nullptr)
        {
            bindables_.push_back(ib);
        }
    }

    void Draw(const mc::GraphicsManager& gm, unsigned int count) override
    {
        for (mc::Bindable* bindable : bindables_)
        {
            bindable->Bind(gm);
        }
        GetDeviceContext(gm)->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        GetDeviceContext(gm)->DrawIndexed(count, 0, 0);
    }
};

void Demo()
{
    constexpr int windowWidth = 1280;
    constexpr int windowHeight = 720;

    mc::Engine engine{ "SolarSystem", windowWidth, windowHeight };
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
    perFrameConstBuffer.model = XMMatrixIdentity();
    perFrameConstBuffer.view  =  XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&target), XMLoadFloat3(&up));
    perFrameConstBuffer.proj  = XMMatrixPerspectiveFovLH((60.0f / 180.0f) * XM_PI, (float)windowWidth / (float)windowHeight, 1.0f, 100.0f);
    mc::ConstBuffer<PerFrameConstBuffer> constBuffer(gm, mc::ConstBufferBind::Vertex, perFrameConstBuffer, 0);
    
    mc::MeshData sphereData;
    mc::GeometryGenerator::GenerateSphere(1, 3, sphereData);

    mc::VertexBuffer sphereVB(gm, sphereData.vertices.data(), sphereData.vertices.size(), sizeof(mc::Vertex));
    mc::IndexBuffer  sphereIB(gm, sphereData.indices.data(), sphereData.indices.size());
    
    // create the input layout
    mc::InputLayoutDesc desc = {
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,    0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0}
        },
        4
    };
    mc::InputLayout sphereIL(gm, vertexShader, desc);

    // Create a quad
    Mesh mesh(gm, &sphereVB, &sphereIL, &sphereIB);

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

        gm.Clear(0.1f, 0.1f, 0.3f);

        mesh.Draw(gm, sphereData.indices.size());

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
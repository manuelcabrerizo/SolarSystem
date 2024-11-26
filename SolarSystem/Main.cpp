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

    void Draw(const mc::GraphicsManager& gm, unsigned int count, bool indexed) override
    {
        for (mc::Bindable* bindable : bindables_)
        {
            bindable->Bind(gm);
        }
        GetDeviceContext(gm)->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        if (indexed)
        {
            GetDeviceContext(gm)->DrawIndexed(count, 0, 0);
        }
        else
        {
            GetDeviceContext(gm)->Draw(count, 0);
        }
        
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
    mc::PixelShader texturePixelShader(gm, "assets/pixel/texturePixel.hlsl");

    // Load textures
    mc::Texture lavaTexture(gm, "assets/textures/Lava.png");

    // Init a Const Buffer
    PerFrameConstBuffer perFrameConstBuffer{};
    perFrameConstBuffer.model = XMMatrixIdentity();
    perFrameConstBuffer.view = XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&target), XMLoadFloat3(&up));
    perFrameConstBuffer.proj = XMMatrixPerspectiveFovLH((60.0f / 180.0f) * XM_PI, (float)windowWidth / (float)windowHeight, 1.0f, 100.0f);
    mc::ConstBuffer<PerFrameConstBuffer> constBuffer(gm, mc::ConstBufferBind::Vertex, perFrameConstBuffer, 0);
        
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
    mc::InputLayout IL(gm, vertexShader, desc);

    // Create a sphere
    mc::MeshData sphereData;
    mc::GeometryGenerator::GenerateSphere(1, 3, sphereData);
    mc::VertexBuffer sphereVB(gm, sphereData.vertices.data(), sphereData.vertices.size(), sizeof(mc::Vertex));
    mc::IndexBuffer  sphereIB(gm, sphereData.indices.data(), sphereData.indices.size());
    Mesh sphere(gm, &sphereVB, &IL, &sphereIB);
    
    // Create a quad
    mc::MeshData quadData;
    mc::GeometryGenerator::GenerateQuad(quadData);
    mc::VertexBuffer quadVB(gm, quadData.vertices.data(), quadData.vertices.size(), sizeof(mc::Vertex));
    Mesh quad(gm, &quadVB, &IL);

    mc::FrameBuffer frameBuffer(gm, 0, 0, windowWidth, windowHeight);


    vertexShader.Bind(gm);
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

        // draw to frame buffer
        {
            texturePixelShader.Bind(gm);

            perFrameConstBuffer.model = XMMatrixScaling(4.0f, 4.0f, 4.0f);
            perFrameConstBuffer.view = XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&target), XMLoadFloat3(&up));
            perFrameConstBuffer.proj = XMMatrixPerspectiveFovLH((60.0f / 180.0f) * XM_PI, (float)windowWidth / (float)windowHeight, 1.0f, 100.0f);
            constBuffer.Update(gm, perFrameConstBuffer);

            frameBuffer.Bind(gm);
            frameBuffer.Clear(gm, 0.1f, 0.1f, 0.3f);

            lavaTexture.Bind(gm, 0);
            sphere.Draw(gm, sphereData.indices.size(), true);
            lavaTexture.Unbind(gm, 0);
        }

        // draw to backBuffer
        {
            texturePixelShader.Bind(gm);

            perFrameConstBuffer.model = XMMatrixScaling(static_cast<float>(windowWidth), static_cast<float>(windowHeight), 1.0f);
            perFrameConstBuffer.view = XMMatrixIdentity();
            perFrameConstBuffer.proj = XMMatrixOrthographicLH(windowWidth, windowHeight, 0, 100);
            constBuffer.Update(gm, perFrameConstBuffer);

            gm.BindBackBuffer();
            gm.Clear(0.3f, 0.1f, 0.1f);

            

            frameBuffer.BindAsTexture(gm, 0);
            quad.Draw(gm, quadData.vertices.size(), false);
            frameBuffer.UnbindAsTexture(gm, 0);
        }

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
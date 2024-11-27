#include <iostream>
#include <cmath>

#include <DirectXMath.h>
#include "Engine.h"

using namespace DirectX;

struct PerFrameConstBuffer
{
    XMMATRIX model;
    XMMATRIX view;
    XMMATRIX proj;
};

class Mesh : public mc::GraphicsResource
{
public:
    Mesh(const mc::GraphicsManager& gm,
        mc::VertexBuffer* vb = nullptr,
        mc::InputLayout* il = nullptr,
        mc::IndexBuffer* ib = nullptr)
        : vb_(vb), il_(il), ib_(ib) { }

    void Draw(const mc::GraphicsManager& gm, unsigned int count, bool indexed)
    {
        if (vb_)
        {
            vb_->Bind(gm);
        }
        if (il_)
        {
            il_->Bind(gm);
        }
        if (ib_)
        {
            ib_->Bind(gm);
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
private:
    mc::VertexBuffer* vb_{ nullptr };
    mc::InputLayout* il_{ nullptr };
    mc::IndexBuffer* ib_{ nullptr };
};

void Demo()
{
    constexpr int windowWidth = 800;
    constexpr int windowHeight = 600;

    mc::Engine engine{ "SolarSystem", windowWidth, windowHeight };
    mc::GraphicsManager& gm = engine.GetGraphicsManager();
    mc::InputManager& im = engine.GetInputManager();
    mc::ShaderManager& sm = engine.GetShaderManager();

    // Camera Variables
    XMFLOAT3 position(0.0f, 0.0f, 20.0f);
    XMFLOAT3 target(0.0f, 0.0f, 0.0f);
    XMFLOAT3 up(0.0f, 1.0f, 0.0f);

    // Init Shaders
    mc::Shader vertexShaderHandle = sm.AddVertexShader(gm, "assets/vertex/vert.hlsl");
    mc::Shader postProcessShaderHandle = sm.AddPixelShader(gm, "assets/pixel/postProcess.hlsl");
    mc::Shader texturePixelShaderHandle = sm.AddPixelShader(gm, "assets/pixel/texturePixel.hlsl");
    mc::Shader skyboxShaderHandle = sm.AddPixelShader(gm, "assets/pixel/skybox.hlsl");
    mc::VertexShader& vertexShader = sm.GetVertexShader(vertexShaderHandle);
    mc::PixelShader& postProcessShader = sm.GetPixelShader(postProcessShaderHandle);
    mc::PixelShader& texturePixelShader = sm.GetPixelShader(texturePixelShaderHandle);
    mc::PixelShader& skyboxShader = sm.GetPixelShader(skyboxShaderHandle);

    // Load textures
    mc::Texture lavaTexture(gm, "assets/textures/Lava.png");
    mc::Texture skyTexture(gm, "assets/textures/sky.png");
    mc::Texture moonTexture(gm, "assets/textures/moon.png");

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
    mc::GeometryGenerator::GenerateSphere(1, 20, 20, sphereData);
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


    float time = 0;

    while (engine.IsRunning())
    {
        sm.HotReaload(gm);

        position = XMFLOAT3(20 * std::cosf(time), 0.0f, 20 * -std::sinf(time));
        time += 0.01f;

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

            perFrameConstBuffer.model = XMMatrixScaling(6.0f, 6.0f, 6.0f);
            perFrameConstBuffer.view = XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&target), XMLoadFloat3(&up));
            perFrameConstBuffer.proj = XMMatrixPerspectiveFovLH((60.0f / 180.0f) * XM_PI, (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
            constBuffer.Update(gm, perFrameConstBuffer);

            frameBuffer.Bind(gm);
            frameBuffer.Clear(gm, 0.1f, 0.1f, 0.3f);

            // Draw sky
            skyboxShader.Bind(gm);
            gm.SetRasterizerStateCullFront();
            gm.SetDepthStencilOff();
            perFrameConstBuffer.model = XMMatrixTranslation(position.x, position.y, position.z);
            constBuffer.Update(gm, perFrameConstBuffer);
            skyTexture.Bind(gm, 0);
            sphere.Draw(gm, sphereData.indices.size(), true);
            skyTexture.Unbind(gm, 0);


            gm.SetRasterizerStateCullBack();
            gm.SetDepthStencilOn();
            texturePixelShader.Bind(gm);

            // Draw planet
            perFrameConstBuffer.model = XMMatrixScaling(4.0f, 4.0f, 4.0f) * XMMatrixTranslation(0, 0, 0);
            constBuffer.Update(gm, perFrameConstBuffer);
            lavaTexture.Bind(gm, 0);
            sphere.Draw(gm, sphereData.indices.size(), true);
            lavaTexture.Unbind(gm, 0);

            // Draw moon
            perFrameConstBuffer.model = XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(5.0f, 0.0f, 0.0f);
            constBuffer.Update(gm, perFrameConstBuffer);
            moonTexture.Bind(gm, 0);
            sphere.Draw(gm, sphereData.indices.size(), true);
            moonTexture.Unbind(gm, 0);
        }

        // draw to backBuffer
        {
            gm.SetRasterizerStateCullBack();

            postProcessShader.Bind(gm);

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
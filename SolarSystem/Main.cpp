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

struct CameraConstBuffer
{
    XMFLOAT3 viewPos;
    float pad;
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
    constexpr int windowWidth = 1280;
    constexpr int windowHeight = 720;

    // Camera variables
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    float fov = (45.0f / 180.0f) * XM_PI;
    float aspectRation = (float)windowWidth / (float)windowHeight;

    mc::Engine engine{ "SolarSystem", windowWidth, windowHeight };
    mc::GraphicsManager& gm = engine.GetGraphicsManager();
    mc::InputManager& im = engine.GetInputManager();
    mc::ShaderManager& sm = engine.GetShaderManager();

    // Camera Variables
    mc::Camera camera(XMFLOAT3(0.0f, 0.0f, -20.0f));

    // Init Shaders
    sm.AddVertexShader("vert", gm, "assets/vertex/vert.hlsl");
    sm.AddPixelShader("postProcess", gm, "assets/pixel/postProcess.hlsl");
    sm.AddPixelShader("texturePixel", gm, "assets/pixel/texturePixel.hlsl");
    sm.AddPixelShader("skybox", gm, "assets/pixel/skybox.hlsl");

    // Load textures
    mc::Texture lavaTexture(gm, "assets/textures/Lava.png");
    mc::Texture skyTexture(gm, "assets/textures/sky3.png");
    mc::Texture moonTexture(gm, "assets/textures/moon.png");

    // Init a Const Buffer
    PerFrameConstBuffer perFrameConstBuffer{};
    perFrameConstBuffer.model = XMMatrixIdentity();
    perFrameConstBuffer.view = camera.GetViewMat();
    perFrameConstBuffer.proj = XMMatrixPerspectiveFovLH(fov, aspectRation, nearPlane, farPlane);
    mc::ConstBuffer<PerFrameConstBuffer> constBuffer(gm, mc::ConstBufferBind::Vertex, perFrameConstBuffer, 0);

    CameraConstBuffer cameraConstBuffer{};
    //XMStoreFloat3(&cameraConstBuffer.viewDir, XMVector3Normalize(XMLoadFloat3(&target) - XMLoadFloat3(&position)));
    cameraConstBuffer.viewPos = camera.GetPosition();
    mc::ConstBuffer<CameraConstBuffer> cameraCB(gm, mc::ConstBufferBind::Vertex, cameraConstBuffer, 1);
        
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
    mc::InputLayout IL(gm, *(mc::VertexShader *)sm.Get("vert"), desc);

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

    sm.Get("vert")->Bind(gm);
    constBuffer.Bind(gm);
    cameraCB.Bind(gm);

    while (engine.IsRunning())
    {
        sm.HotReaload(gm);

        camera.Update(im, 0.016f);

        gm.SetRasterizerStateCullBack();

        frameBuffer.Bind(gm);
        frameBuffer.Clear(gm, 0.1f, 0.1f, 0.3f);
        perFrameConstBuffer.view = camera.GetViewMat();
        perFrameConstBuffer.proj = XMMatrixPerspectiveFovLH(fov, aspectRation, nearPlane, farPlane);
        // Draw sky
        {
            // Compute the sky coord system so is always facing the camera
            XMFLOAT3 target = camera.GetTarget();
            XMVECTOR front = XMVector3Normalize(XMLoadFloat3(&camera.GetPosition()) - XMLoadFloat3(&target));
            XMVECTOR right = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), front));
            XMVECTOR up = XMVector3Cross(front, right);

            XMVECTOR viewDir =  XMVector3Normalize(XMLoadFloat3(&target) - XMLoadFloat3(&camera.GetPosition()));
            cameraConstBuffer.viewPos = camera.GetPosition();
            cameraCB.Update(gm, cameraConstBuffer);

            // Compute the scale so the quad fill the screen 
            float a = (farPlane * 0.5f);
            float ys = (std::tanf(fov * 0.5f) * a) * 2.0f;
            float xs = ys * aspectRation;

            XMMATRIX translationMat = XMMatrixTranslationFromVector(XMLoadFloat3(&camera.GetPosition()) + (viewDir * a));
            XMMATRIX rotationMat = XMMatrixRotationY(XM_PI) * XMMATRIX(right, up, front, XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));
            XMMATRIX scaleMat = XMMatrixScaling(xs, ys, 1.0f);

            perFrameConstBuffer.model = scaleMat * rotationMat * translationMat;
            constBuffer.Update(gm, perFrameConstBuffer);

            sm.Get("skybox")->Bind(gm);
            gm.SetDepthStencilOff();

            skyTexture.Bind(gm, 0);
            quad.Draw(gm, quadData.vertices.size(), false);
            skyTexture.Unbind(gm, 0);
        }
        
        // Draw Planets
        {
            gm.SetDepthStencilOn();
            sm.Get("texturePixel")->Bind(gm);
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
        

        gm.BindBackBuffer();
        gm.Clear(0.3f, 0.1f, 0.1f);
        // Draw to backBuffer
        {
            sm.Get("postProcess")->Bind(gm);
            perFrameConstBuffer.model = XMMatrixScaling(static_cast<float>(windowWidth), static_cast<float>(windowHeight), 1.0f);
            perFrameConstBuffer.view = XMMatrixIdentity();
            perFrameConstBuffer.proj = XMMatrixOrthographicLH(windowWidth, windowHeight, 0, 100);
            constBuffer.Update(gm, perFrameConstBuffer);

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
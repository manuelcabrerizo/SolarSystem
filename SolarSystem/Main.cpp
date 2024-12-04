#include <iostream>
#include <cmath>
#include <list>

#include <DirectXMath.h>
#include "Engine.h"

#include "Ship.h"

using namespace DirectX;

struct ObjectConstBuffer
{
    XMMATRIX model;

};
struct CameraConstBuffer
{
    XMMATRIX view;
    XMMATRIX proj;
    XMFLOAT3 viewPos;
    float pad;
};

struct PointLight
{
    XMFLOAT3 position;
    float constant;
    XMFLOAT3 ambient;
    float linear;
    XMFLOAT3 diffuse;
    float quadratic;
    XMFLOAT3 specular;
    float pad0;
};

struct LightConstBuffer
{
    PointLight lights[4];
    int count;
    XMFLOAT3 viewPos;
};

struct CommonConstBuffer
{
    XMFLOAT2 resolution;
    float time;
    float pad0;
};

class Scene;

class SceneNode
{
public:
    void SetPosition(float x, float y, float z)
    {
        position_ = XMVectorSet(x, y, z, 1.0f);
    }
    void SetRotation(XMVECTOR rotation)
    {
        rotation_ = rotation;
    }
    void SetScale(float x, float y, float z)
    {
        scale_ = XMVectorSet(x, y, z, 1.0f);
    }
    void SetMesh(mc::Mesh* mesh) { mesh_ = mesh; }
    void SetTexture(mc::Texture* texture) { texture_ = texture; }
    void SetVertexShader(mc::VertexShader* vs) { vs_ = vs; }
    void SetPixelShader(mc::PixelShader* ps) { ps_ = ps; }

    XMVECTOR GetPosition()
    {
        return position_;
    }

    XMVECTOR GetParentPosition()
    {
        SceneNode* parent = parent_;
        XMVECTOR position = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
        while (parent)
        {
            position += parent->position_;
            parent = parent->parent_;
        }
        return position;
    }

    SceneNode& AddNode()
    {
        SceneNode& node = childrens_.emplace_back();
        node.parent_ = this;
        return node;
    }

    void Draw(const mc::GraphicsManager& gm, const Scene& scene);

private:
    mc::Mesh* mesh_{nullptr};
    mc::Texture* texture_{ nullptr };
    mc::VertexShader* vs_{ nullptr };
    mc::PixelShader* ps_{ nullptr };

    XMVECTOR position_;
    XMVECTOR rotation_;
    XMVECTOR scale_;
    std::list<SceneNode> childrens_;
    SceneNode* parent_;
};

class Scene
{
public:
    Scene(ObjectConstBuffer* objectCPUBuffer, mc::ConstBuffer<ObjectConstBuffer>* objectGPUBuffer)
        : objectCPUBuffer_(objectCPUBuffer), objectGPUBuffer_(objectGPUBuffer)
    {
    }

    SceneNode& AddNode()
    {
        return root_.AddNode();
    }


    void Draw(const mc::GraphicsManager& gm)
    {
        root_.Draw(gm, *this);
    }

    ObjectConstBuffer* objectCPUBuffer_;
    mc::ConstBuffer<ObjectConstBuffer>* objectGPUBuffer_;
private:
    SceneNode root_;
};

void SceneNode::Draw(const mc::GraphicsManager& gm, const Scene& scene)
{
    XMMATRIX trans = XMMatrixTranslationFromVector(GetParentPosition() + position_);
    XMMATRIX rot = XMMatrixRotationQuaternion(rotation_);
    XMMATRIX scale = XMMatrixScalingFromVector(scale_);

    scene.objectCPUBuffer_->model = scale * rot * trans;
    scene.objectGPUBuffer_->Update(gm, *scene.objectCPUBuffer_);
    if (vs_) { vs_->Bind(gm); }
    if (ps_) { ps_->Bind(gm); }
    if (texture_) { texture_->Bind(gm, 0); }
    if (mesh_) { mesh_->Draw(gm); }
    if (texture_) { texture_->Unbind(gm, 0); }

    for (auto& child : childrens_)
    {
        child.Draw(gm, scene);
    }
}

constexpr int windowWidth = 1920;
constexpr int windowHeight = 1080;

float Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

void Demo()
{
    // Camera variables
    float nearPlane = 0.01f;
    float farPlane = 100.0f;
    float fovMin = (60.0f / 180.0f) * XM_PI;
    float fovMax = (90.0f / 180.0f) * XM_PI;
    float aspectRation = (float)windowWidth / (float)windowHeight;

    // Quary the clock frequency
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    mc::Engine engine("Solar System", windowWidth, windowHeight);
    mc::GraphicsManager& gm = engine.GetGraphicsManager();
    mc::InputManager& im = engine.GetInputManager();
    mc::ShaderManager& sm = engine.GetShaderManager();

    // Camera Variables
    mc::Camera camera(XMFLOAT3(0, 0, 0));

    // Init Shaders
    sm.AddVertexShader("vert", gm, "assets/vertex/vert.hlsl");
    sm.AddPixelShader("postProcess", gm, "assets/pixel/postProcess.hlsl");
    sm.AddPixelShader("texturePixel", gm, "assets/pixel/texturePixel.hlsl");
    sm.AddPixelShader("skybox", gm, "assets/pixel/skybox.hlsl");
    sm.AddPixelShader("sun", gm, "assets/pixel/sunPixel.hlsl");
    sm.AddPixelShader("trackBase", gm, "assets/pixel/trackBase.hlsl");
    sm.AddPixelShader("trackRail", gm, "assets/pixel/trackRail.hlsl");
    sm.AddPixelShader("ship", gm, "assets/pixel/ship.hlsl");


    // Load textures
    mc::Texture lavaTexture(gm, "assets/textures/Lava.png");
    mc::Texture skyTexture(gm, "assets/textures/sky3.png");
    mc::Texture moonTexture(gm, "assets/textures/moon.png");
    mc::Texture planet1Texture(gm, "assets/textures/planet.png");
    mc::Texture planet2Texture(gm, "assets/textures/planet2.png");
    mc::Texture sunTexture(gm, "assets/textures/sun.png");
    mc::Texture shipTexture(gm, "assets/textures/Overtone_Default_Diffuse.png");

    // Init a Const Buffer
    ObjectConstBuffer objectCPUBuffer{};
    objectCPUBuffer.model = XMMatrixIdentity();
    mc::ConstBuffer<ObjectConstBuffer> objectGPUBuffer(gm, mc::ConstBufferBind::Vertex, objectCPUBuffer, 0);

    CameraConstBuffer cameraCPUBuffer{};
    cameraCPUBuffer.view = camera.GetViewMat();
    cameraCPUBuffer.proj = XMMatrixPerspectiveFovLH(fovMin, aspectRation, nearPlane, farPlane);
    cameraCPUBuffer.viewPos = camera.GetPosition();
    mc::ConstBuffer<CameraConstBuffer> cameraGPUBuffer(gm, mc::ConstBufferBind::Vertex, cameraCPUBuffer, 1);

    LightConstBuffer lightCPUBuffer{};
    lightCPUBuffer.count = 1;
    lightCPUBuffer.lights[0].position = XMFLOAT3(0, 1, 0);
    lightCPUBuffer.lights[0].constant = 1.0f;
    lightCPUBuffer.lights[0].linear = 0.0014f;
    lightCPUBuffer.lights[0].quadratic = 0.000007;
    lightCPUBuffer.lights[0].ambient = XMFLOAT3(0.2f, 0.2f, 0.2f);
    lightCPUBuffer.lights[0].diffuse = XMFLOAT3(1.5f, 1.5f, 1.5f);
    lightCPUBuffer.lights[0].specular = XMFLOAT3(2.0f, 2.0f,2.0f);
    lightCPUBuffer.viewPos = camera.GetPosition();
    mc::ConstBuffer<LightConstBuffer> lightGPUBuffer(gm, mc::ConstBufferBind::Pixel, lightCPUBuffer, 2);

    CommonConstBuffer commonCPUBuffer{};
    commonCPUBuffer.resolution = XMFLOAT2(windowWidth, windowHeight);
    commonCPUBuffer.time = 0.0f;
    mc::ConstBuffer<CommonConstBuffer> commonGPUBuffer(gm, mc::ConstBufferBind::Pixel, commonCPUBuffer, 3);
        
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
    mc::GeometryGenerator::GenerateSphere(1, 40, 40, sphereData);
    mc::VertexBuffer sphereVB(gm, sphereData.vertices.data(), sphereData.vertices.size(), sizeof(mc::Vertex));
    mc::IndexBuffer  sphereIB(gm, sphereData.indices.data(), sphereData.indices.size());
    mc::Mesh sphere(gm, &sphereVB, &IL, &sphereIB, sphereData.indices.size(), true);
    
    // Create a quad
    mc::MeshData quadData;
    mc::GeometryGenerator::GenerateQuad(quadData);
    mc::VertexBuffer quadVB(gm, quadData.vertices.data(), quadData.vertices.size(), sizeof(mc::Vertex));
    mc::Mesh quad(gm, &quadVB, &IL, nullptr, quadData.vertices.size(), false);

    // Create track base
    mc::MeshData trackBaseData;
    mc::GeometryGenerator::LoadOBJFile(trackBaseData, "assets/mesh/track_base_tri.obj");
    mc::VertexBuffer trackBaseVB(gm, trackBaseData.vertices.data(), trackBaseData.vertices.size(), sizeof(mc::Vertex));
    mc::Mesh trackBaseMesh(gm, &trackBaseVB, &IL, nullptr, trackBaseData.vertices.size(), false);

    // Create track inner
    mc::MeshData trackInnerData;
    mc::GeometryGenerator::LoadOBJFile(trackInnerData, "assets/mesh/track_inner_tri.obj");
    mc::VertexBuffer trackInnerVB(gm, trackInnerData.vertices.data(), trackInnerData.vertices.size(), sizeof(mc::Vertex));
    mc::Mesh trackInnerMesh(gm, &trackInnerVB, &IL, nullptr, trackInnerData.vertices.size(), false);

    // Create track outer
    mc::MeshData trackOuterData;
    mc::GeometryGenerator::LoadOBJFile(trackOuterData, "assets/mesh/track_outer_tri.obj");
    mc::VertexBuffer trackOuterVB(gm, trackOuterData.vertices.data(), trackOuterData.vertices.size(), sizeof(mc::Vertex));
    mc::Mesh trackOuterMesh(gm, &trackOuterVB, &IL, nullptr, trackOuterData.vertices.size(), false);

    // Create ship
    mc::MeshData shipData;
    mc::GeometryGenerator::LoadOBJFile(shipData, "assets/mesh/ship.obj");
    mc::VertexBuffer shipDataVB(gm, shipData.vertices.data(), shipData.vertices.size(), sizeof(mc::Vertex));
    mc::Mesh shipMesh(gm, &shipDataVB, &IL, nullptr, shipData.vertices.size(), false);
    
    // Load collision data
    mc::CollisionData collisionDataOuter;
    mc::GeometryGenerator::LoadCollisionDataFromOBJFile(collisionDataOuter, "assets/mesh/track_outer_col.obj");
    mc::CollisionData collisionDataInner;
    mc::GeometryGenerator::LoadCollisionDataFromOBJFile(collisionDataInner, "assets/mesh/track_inner_col.obj");

    mc::FrameBuffer frameBuffer(gm, 0, 0, windowWidth, windowHeight);

    XMFLOAT3 position = XMFLOAT3(-2.5, 0.0125f, 0);
    mc::Ship shipBody(position, 2.0f, 0.04f);

    //Scene test
    Scene scene(&objectCPUBuffer, &objectGPUBuffer);

    // Create Ship
    SceneNode& ship = scene.AddNode();
    ship.SetMesh(&shipMesh);
    ship.SetTexture(&shipTexture);
    ship.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    ship.SetPixelShader((mc::PixelShader*)sm.Get("ship"));
    ship.SetPosition(position.x, position.y, position.z);
    ship.SetScale(0.0125f*0.5f, 0.0125f*0.5f, 0.0125f*0.5f);

    // Create sun
    SceneNode& sun = scene.AddNode();
    sun.SetMesh(&sphere);
    sun.SetTexture(&sunTexture);
    sun.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    sun.SetPixelShader((mc::PixelShader*)sm.Get("sun"));
    sun.SetPosition(0, 4, 0);
    sun.SetScale(1, 1, 1);

    // Create the planet
    SceneNode& planet = scene.AddNode();
    planet.SetMesh(&sphere);
    planet.SetTexture(&lavaTexture);
    planet.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    planet.SetPixelShader((mc::PixelShader*)sm.Get("texturePixel"));
    planet.SetPosition(0, -20.5, 0);
    planet.SetScale(20, 20, 20);
    planet.SetRotation(XMQuaternionRotationRollPitchYaw(XM_PI / 2, 0.0f, 0.0f));

    // Create the moon
    SceneNode& moon = planet.AddNode();
    moon.SetMesh(&sphere);
    moon.SetTexture(&moonTexture);
    moon.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    moon.SetPixelShader((mc::PixelShader*)sm.Get("texturePixel"));
    moon.SetPosition(0, 1, 2);
    moon.SetScale(1, 1, 1);

    SceneNode& jupiter = sun.AddNode();
    jupiter.SetMesh(&sphere);
    jupiter.SetTexture(&planet1Texture);
    jupiter.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    jupiter.SetPixelShader((mc::PixelShader*)sm.Get("texturePixel"));
    jupiter.SetPosition(4, 1, 0);
    jupiter.SetScale(1, 1, 1);

    SceneNode& saturn = sun.AddNode();
    saturn.SetMesh(&sphere);
    saturn.SetTexture(&planet2Texture);
    saturn.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    saturn.SetPixelShader((mc::PixelShader*)sm.Get("texturePixel"));
    saturn.SetPosition(6, 1, 0);
    saturn.SetScale(1, 1, 1);

    //////////////////////////////////////
    // Add translucid objects
    //////////////////////////////////////
    // Create track base
    SceneNode& trackBase = scene.AddNode();
    trackBase.SetMesh(&trackBaseMesh);
    trackBase.SetTexture(&planet2Texture);
    trackBase.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    trackBase.SetPixelShader((mc::PixelShader*)sm.Get("trackBase"));
    trackBase.SetPosition(0, 0, 0);
    trackBase.SetScale(1, 1, 1);

    // Create track inner
    SceneNode& trackInner = scene.AddNode();
    trackInner.SetMesh(&trackInnerMesh);
    trackInner.SetTexture(&planet1Texture);
    trackInner.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    trackInner.SetPixelShader((mc::PixelShader*)sm.Get("trackRail"));
    trackInner.SetPosition(0, 0, 0);
    trackInner.SetScale(1, 1, 1);

    // Create track inner
    SceneNode& trackOuter = scene.AddNode();
    trackOuter.SetMesh(&trackOuterMesh);
    trackOuter.SetTexture(&planet1Texture);
    trackOuter.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    trackOuter.SetPixelShader((mc::PixelShader*)sm.Get("trackRail"));
    trackOuter.SetPosition(0, 0, 0);
    trackOuter.SetScale(1, 1, 1);

    // Set Alpha blending
    gm.SetAlphaBlending();

    sm.Get("vert")->Bind(gm);
    objectGPUBuffer.Bind(gm);
    cameraGPUBuffer.Bind(gm);
    lightGPUBuffer.Bind(gm);
    commonGPUBuffer.Bind(gm);

    LARGE_INTEGER lastCounter;
    QueryPerformanceCounter(&lastCounter); 

    while (engine.IsRunning())
    {
        LARGE_INTEGER currentCounter;
        QueryPerformanceCounter(&currentCounter);
        double lastTime = (double)lastCounter.QuadPart / frequency.QuadPart;
        double currentTime = (double)currentCounter.QuadPart / frequency.QuadPart;
        float dt = static_cast<float>(currentTime - lastTime);

        commonCPUBuffer.time += dt;
        commonGPUBuffer.Update(gm, commonCPUBuffer);

        sm.HotReaload(gm);

        mc::CollisionData* collisionDataArray[] = {
            &collisionDataOuter,
            &collisionDataInner
        };

        shipBody.Update(im, dt, collisionDataArray, 2);
        ship.SetRotation(shipBody.GetOrientation());
          
        XMFLOAT3 shipPos = shipBody.GetPosition();
        ship.SetPosition(shipPos.x, shipPos.y, shipPos.z);

        //camera.Update(im, dt);
        camera.FollowShip(shipBody);

        lightCPUBuffer.viewPos = camera.GetPosition();
        lightGPUBuffer.Update(gm, lightCPUBuffer);

        // Update planets position and rotation
        /*
        sun.SerRotation(XMQuaternionRotationRollPitchYaw(0, -time*0.1f, 0));
        planet.SetPosition(100.0f * std::cosf(time*0.2f + 10), 0.0f, 100.0f * std::sinf(time*0.2f + 10));
        planet.SerRotation(XMQuaternionRotationRollPitchYaw(0, -time*2.0f, 0));
        moon.SetPosition(6.0f * std::cosf(time), 0.0f, 6.0f * std::sinf(time));
        moon.SerRotation(XMQuaternionRotationRollPitchYaw(0, -time * 3.0f, 0));
        jupiter.SetPosition(300.0f * std::cosf(time*0.05f + 300), 0.0f, 300.0f * std::sinf(time*0.05f + 300.0f));
        jupiter.SerRotation(XMQuaternionRotationRollPitchYaw(0, -time, 0));
        saturn.SetPosition(150.0f * std::cosf(time * 0.1f + 50), 0.0f, 150.0f * std::sinf(time * 0.1f + 50.0f));
        saturn.SerRotation(XMQuaternionRotationRollPitchYaw(0, -time * 0.5f, 0));
        */
        gm.SetRasterizerStateCullNone();

        float shipVel = XMVectorGetX(XMVector3Length(shipBody.GetVelocity()));
        float fov = Lerp(fovMin, fovMax, std::clamp(shipVel * shipVel, 0.0f, 1.0f));

        // Update the camera const buffer
        cameraCPUBuffer.view = camera.GetViewMat();
        cameraCPUBuffer.proj = XMMatrixPerspectiveFovLH(fov, aspectRation, nearPlane, farPlane);
        cameraCPUBuffer.viewPos = camera.GetPosition();
        cameraGPUBuffer.Update(gm, cameraCPUBuffer);

        frameBuffer.Bind(gm);
        frameBuffer.Clear(gm, 0.1f, 0.1f, 0.3f);
        // Draw sky
        {
            // Compute the sky coord system so is always facing the camera
            // Compute the scale so the quad fill the screen 
            float a = (farPlane * 0.5f);
            float ys = (std::tanf(fov * 0.5f) * a) * 2.0f;
            float xs = ys * aspectRation;

            XMVECTOR viewDir = camera.GetFront();
            XMMATRIX translationMat = XMMatrixTranslationFromVector(XMLoadFloat3(&camera.GetPosition()) + (viewDir * a));
            XMMATRIX rotationMat = 
                XMMATRIX(camera.GetRight(), camera.GetUp(), camera.GetFront(), XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));
            XMMATRIX scaleMat = XMMatrixScaling(xs, ys, 1.0f);

            objectCPUBuffer.model = scaleMat * rotationMat * translationMat;
            objectGPUBuffer.Update(gm, objectCPUBuffer);

            sm.Get("skybox")->Bind(gm);
            gm.SetDepthStencilOff();

            skyTexture.Bind(gm, 0);
            quad.Draw(gm);
            skyTexture.Unbind(gm, 0);
        }
        
        gm.SetDepthStencilOn();
        scene.Draw(gm);
        
        // Draw to backBuffer
        {
            gm.SetRasterizerStateCullBack();
            gm.BindBackBuffer();
            gm.Clear(0.3f, 0.1f, 0.1f);
            sm.Get("postProcess")->Bind(gm);
            objectCPUBuffer.model = XMMatrixScaling(static_cast<float>(windowWidth), static_cast<float>(windowHeight), 1.0f);
            cameraCPUBuffer.view = XMMatrixIdentity();
            cameraCPUBuffer.proj = XMMatrixOrthographicLH(windowWidth, windowHeight, 0, 100);
            objectGPUBuffer.Update(gm, objectCPUBuffer);
            cameraGPUBuffer.Update(gm, cameraCPUBuffer);
            frameBuffer.BindAsTexture(gm, 0);
            quad.Draw(gm);
            frameBuffer.UnbindAsTexture(gm, 0);
        }

        gm.Present();

        lastCounter = currentCounter;
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
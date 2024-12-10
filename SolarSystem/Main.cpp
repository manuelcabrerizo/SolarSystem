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
    XMFLOAT2 screenPos;
    XMFLOAT2 flerActive;
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

    void SetCullBack(bool value)
    {
        cullBack_ = value;
    }

    XMMATRIX GetModelMatrix()
    {
        XMMATRIX trans = XMMatrixTranslationFromVector(GetParentPosition() + position_);
        XMMATRIX rot = XMMatrixRotationQuaternion(rotation_);
        XMMATRIX scale = XMMatrixScalingFromVector(scale_);
        return scale * rot * trans;
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
    bool cullBack_{ true };

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
        gm.SetRasterizerStateCullBack();
        root_.Draw(gm, *this);
    }

    ObjectConstBuffer* objectCPUBuffer_;
    mc::ConstBuffer<ObjectConstBuffer>* objectGPUBuffer_;
private:
    SceneNode root_;
};

void SceneNode::Draw(const mc::GraphicsManager& gm, const Scene& scene)
{
    if (!cullBack_)
    {
        gm.SetRasterizerStateCullNone();
    }

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

    if (!cullBack_)
    {
        gm.SetRasterizerStateCullBack();
    }
}

constexpr int windowWidth = 1366;
constexpr int windowHeight = 768;

float Lerp(float a, float b, float t)
{
    return (1.0f - t) * a + b * t;
}

float InverseLerp(float a, float b, float v)
{
    return (v - a) / (b - a);
}

float Remap(float v, float inMin, float inMax, float outMin, float outMax)
{
    float t = InverseLerp(inMin, inMax, v);
    return Lerp(outMin, outMax, t);

}

void Demo()
{
    bool freeMode = true;

    // Camera variables
    float nearPlane = 0.01f;
    float farPlane = 100.0f;
    float fovMin = (60.0f / 180.0f) * XM_PI;
    float fovMax = (90.0f / 180.0f) * XM_PI;
    float aspectRation = (float)windowWidth / (float)windowHeight;

    // Quary the clock frequency
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    mc::Engine engine("Solar System Racing", windowWidth, windowHeight);
    mc::GraphicsManager& gm = engine.GetGraphicsManager();
    mc::InputManager& im = engine.GetInputManager();
    mc::ShaderManager& sm = engine.GetShaderManager();

    // Camera Variables
    mc::Camera camera(XMFLOAT3(0, 0, -2));

    // Init Shaders
    sm.AddVertexShader("vert", gm, "assets/vertex/vert.hlsl");
    sm.AddPixelShader("postProcess", gm, "assets/pixel/postProcess.hlsl");
    sm.AddPixelShader("earth", gm, "assets/pixel/earth.hlsl");
    sm.AddPixelShader("mars", gm, "assets/pixel/mars.hlsl");
    sm.AddPixelShader("skybox", gm, "assets/pixel/skybox.hlsl");
    sm.AddPixelShader("sun", gm, "assets/pixel/sun.hlsl");
    sm.AddPixelShader("trackBase", gm, "assets/pixel/trackBase.hlsl");
    sm.AddPixelShader("trackRail", gm, "assets/pixel/trackRail.hlsl");
    sm.AddPixelShader("ship", gm, "assets/pixel/ship.hlsl");
    sm.AddPixelShader("meta", gm, "assets/pixel/meta.hlsl");
    sm.AddPixelShader("postes", gm, "assets/pixel/postes.hlsl");
    sm.AddPixelShader("bloomSelector", gm, "assets/pixel/bloomSelector.hlsl");
    sm.AddPixelShader("bloom", gm, "assets/pixel/bloom.hlsl");
    sm.AddPixelShader("jupiter", gm, "assets/pixel/jupiter.hlsl");
    sm.AddPixelShader("saturn", gm, "assets/pixel/saturn.hlsl");

    // Load Shaders for the particle system
    sm.AddVertexShader("soFireVer", gm, "assets/vertex/soFireVer.hlsl");
    sm.AddGeometryShader("soFireGeo", gm, "assets/geometry/soFireGeo.hlsl", true);
    sm.AddVertexShader("dwFireVer", gm, "assets/vertex/dwFireVer.hlsl");
    sm.AddPixelShader("dwFirePix", gm, "assets/pixel/dwFirePix.hlsl");
    sm.AddGeometryShader("dwFireGeo", gm, "assets/geometry/dwFireGeo.hlsl");

    // Load textures
    mc::Texture shipTexture(gm, "assets/textures/Overtone_Default_Diffuse.png");
    mc::Texture jupiterTexture(gm, "assets/textures/planet.png");
    mc::Texture saturnTexture(gm, "assets/textures/planet2.png");
    mc::Texture lavaTexture(gm, "assets/textures/Lava.png");

    // Create the particle system
    mc::ParticleSystem particleSystem(gm, 2000, sm.Get("soFireVer"), sm.Get("soFireGeo"),
        sm.Get("dwFireVer"), sm.Get("dwFirePix"), sm.Get("dwFireGeo"), lavaTexture);


    // Init a Const Buffer
    ObjectConstBuffer objectCPUBuffer{};
    objectCPUBuffer.model = XMMatrixIdentity();
    mc::ConstBuffer<ObjectConstBuffer> objectGPUBuffer(gm, mc::BIND_TO_VS, objectCPUBuffer, 0);

    CameraConstBuffer cameraCPUBuffer{};
    cameraCPUBuffer.view = camera.GetViewMat();
    cameraCPUBuffer.proj = XMMatrixPerspectiveFovLH(fovMin, aspectRation, nearPlane, farPlane);
    cameraCPUBuffer.viewPos = camera.GetPosition();
    mc::ConstBuffer<CameraConstBuffer> cameraGPUBuffer(gm, mc::BIND_TO_VS|mc::BIND_TO_GS, cameraCPUBuffer, 1);

    LightConstBuffer lightCPUBuffer{};
    lightCPUBuffer.count = 1;
    lightCPUBuffer.lights[0].position = XMFLOAT3(0, 10, 10);
    lightCPUBuffer.lights[0].constant = 1.0f;
    lightCPUBuffer.lights[0].linear = 0.0014f;
    lightCPUBuffer.lights[0].quadratic = 0.000007;
    lightCPUBuffer.lights[0].ambient = XMFLOAT3(0.025f, 0.025f, 0.025f);
    lightCPUBuffer.lights[0].diffuse = XMFLOAT3(100.0f, 100.0f, 100.0f);
    lightCPUBuffer.lights[0].specular = XMFLOAT3(140.0f, 140.0f,140.0f);
    lightCPUBuffer.viewPos = camera.GetPosition();
    mc::ConstBuffer<LightConstBuffer> lightGPUBuffer(gm, mc::BIND_TO_PS, lightCPUBuffer, 2);

    CommonConstBuffer commonCPUBuffer{};
    commonCPUBuffer.resolution = XMFLOAT2(windowWidth, windowHeight);
    commonCPUBuffer.time = 0.0f;
    mc::ConstBuffer<CommonConstBuffer> commonGPUBuffer(gm, mc::BIND_TO_PS, commonCPUBuffer, 3);
        
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

    // create the input layout for the particle system
    mc::InputLayoutDesc particleILDesc = {
        {
            {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 3, DXGI_FORMAT_R32_UINT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0}
        },
        5
    };
    mc::InputLayout particleIL(gm, *(mc::VertexShader*)sm.Get("soFireVer"), particleILDesc);


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

    // Create planets
    mc::MeshData planetData;
    mc::GeometryGenerator::LoadOBJFile(planetData, "assets/mesh/planet.obj");
    mc::VertexBuffer planetDataVB(gm, planetData.vertices.data(), planetData.vertices.size(), sizeof(mc::Vertex));
    mc::Mesh planetMesh(gm, &planetDataVB, &IL, nullptr, planetData.vertices.size(), false);

    // Create meta
    mc::MeshData metaData;
    mc::GeometryGenerator::LoadOBJFile(metaData, "assets/mesh/meta.obj");
    mc::VertexBuffer metaDataVB(gm, metaData.vertices.data(), metaData.vertices.size(), sizeof(mc::Vertex));
    mc::Mesh metaMesh(gm, &metaDataVB, &IL, nullptr, metaData.vertices.size(), false);

    // Create postes
    mc::MeshData postesData;
    mc::GeometryGenerator::LoadOBJFile(postesData, "assets/mesh/postes.obj");
    mc::VertexBuffer postesDataVB(gm, postesData.vertices.data(), postesData.vertices.size(), sizeof(mc::Vertex));
    mc::Mesh postesMesh(gm, &postesDataVB, &IL, nullptr, postesData.vertices.size(), false);


    // Load collision data
    mc::CollisionData collisionDataOuter;
    mc::GeometryGenerator::LoadCollisionDataFromOBJFile(collisionDataOuter, "assets/mesh/track_outer_col.obj");
    mc::CollisionData collisionDataInner;
    mc::GeometryGenerator::LoadCollisionDataFromOBJFile(collisionDataInner, "assets/mesh/track_inner_col.obj");

    mc::FrameBuffer msaaBuffer(gm, 0, 0, windowWidth, windowHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, true, 8);
    mc::FrameBuffer bloom0Buffer(gm, 0, 0, windowWidth, windowHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);
    mc::FrameBuffer bloom1Buffer(gm, 0, 0, windowWidth, windowHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);

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

    // Create meta
    SceneNode& meta = scene.AddNode();
    meta.SetMesh(&metaMesh);
    meta.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    meta.SetPixelShader((mc::PixelShader*)sm.Get("meta"));
    meta.SetPosition(0, 0, 0);
    meta.SetScale(1.0f, 1.0f, 1.0f);

    // Create postes
    SceneNode& postes = scene.AddNode();
    postes.SetMesh(&postesMesh);
    postes.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    postes.SetPixelShader((mc::PixelShader*)sm.Get("postes"));
    postes.SetPosition(0, 0, 0);
    postes.SetScale(1.0f, 1.0f, 1.0f);

    // Create sun
    SceneNode& sun = scene.AddNode();
    sun.SetMesh(&planetMesh);
    sun.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    sun.SetPixelShader((mc::PixelShader*)sm.Get("sun"));
    sun.SetPosition(0, 10, 40);
    sun.SetScale(10, 10, 10);

    // Create the earth
    SceneNode& earth = scene.AddNode();
    earth.SetMesh(&planetMesh);
    earth.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    earth.SetPixelShader((mc::PixelShader*)sm.Get("earth"));
    earth.SetPosition(0, -20.5, 0);
    earth.SetScale(20, 20, 20);

    // Create the mars
    SceneNode& mars = scene.AddNode();
    mars.SetMesh(&planetMesh);
    mars.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    mars.SetPixelShader((mc::PixelShader*)sm.Get("mars"));
    mars.SetPosition(40, 0, 30);
    mars.SetScale(10, 10, 10);

    // Create the jupiter
    SceneNode& jupiter = scene.AddNode();
    jupiter.SetMesh(&planetMesh);
    jupiter.SetTexture(&jupiterTexture);
    jupiter.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    jupiter.SetPixelShader((mc::PixelShader*)sm.Get("jupiter"));
    jupiter.SetPosition(-80, 0, 0);
    jupiter.SetScale(20, 20, 20);

    // create the saturn
    SceneNode& saturn = jupiter.AddNode();
    saturn.SetMesh(&planetMesh);
    saturn.SetTexture(&saturnTexture);
    saturn.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    saturn.SetPixelShader((mc::PixelShader*)sm.Get("saturn"));
    saturn.SetPosition(-20, 15, -20);
    saturn.SetScale(10, 10, 10);


    //////////////////////////////////////
    // Add translucid objects
    //////////////////////////////////////
    // Create track base
    SceneNode& trackBase = scene.AddNode();
    trackBase.SetMesh(&trackBaseMesh);
    trackBase.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    trackBase.SetPixelShader((mc::PixelShader*)sm.Get("trackBase"));
    trackBase.SetPosition(0, 0, 0);
    trackBase.SetScale(1, 1, 1);

    // Create track inner
    SceneNode& trackInner = scene.AddNode();
    trackInner.SetMesh(&trackInnerMesh);
    trackInner.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    trackInner.SetPixelShader((mc::PixelShader*)sm.Get("trackRail"));
    trackInner.SetPosition(0, 0, 0);
    trackInner.SetScale(1, 1, 1);
    trackInner.SetCullBack(false);

    // Create track outer
    SceneNode& trackOuter = scene.AddNode();
    trackOuter.SetMesh(&trackOuterMesh);
    trackOuter.SetVertexShader((mc::VertexShader*)sm.Get("vert"));
    trackOuter.SetPixelShader((mc::PixelShader*)sm.Get("trackRail"));
    trackOuter.SetPosition(0, 0, 0);
    trackOuter.SetScale(1, 1, 1);
    trackOuter.SetCullBack(false);

    // Set Alpha blending
    gm.SetAlphaBlending();
    sm.Get("vert")->Bind(gm);
    objectGPUBuffer.Bind(gm);
    cameraGPUBuffer.Bind(gm);
    lightGPUBuffer.Bind(gm);
    commonGPUBuffer.Bind(gm);

    LARGE_INTEGER lastCounter;
    QueryPerformanceCounter(&lastCounter); 

    float time = 0.0f;

    int frameCount = 0.0f;

    double gameTime = 0.0f;

    while (engine.IsRunning())
    {
        if (im.KeyJustDown(mc::KEY_1))
        {
            freeMode = !freeMode;
        }

        LARGE_INTEGER currentCounter;
        QueryPerformanceCounter(&currentCounter);
        double lastTime = (double)lastCounter.QuadPart / frequency.QuadPart;
        double currentTime = (double)currentCounter.QuadPart / frequency.QuadPart;
        float dt = static_cast<float>(currentTime - lastTime);

        
        if (time >= 1.0f)
        {
            std::cout << "FPS: " << frameCount << "\n";
            frameCount = 0;
            time = 0;
        }
        time += dt;
        frameCount++;
        

        sm.HotReaload(gm);

        mc::CollisionData* collisionDataArray[] = {
            &collisionDataOuter,
            &collisionDataInner
        };

        if (freeMode == false)
        {
            shipBody.Update(im, dt, collisionDataArray, 2);
        }
        ship.SetRotation(shipBody.GetOrientation());
          
        XMFLOAT3 shipPos = shipBody.GetPosition();
        ship.SetPosition(shipPos.x, shipPos.y, shipPos.z);

        if (freeMode)
        {
            camera.Update(im, dt);
        }
        else
        {
            camera.FollowShip(shipBody);
        }

        XMFLOAT3 sunPosition;
        XMStoreFloat3(&sunPosition, sun.GetPosition());
        lightCPUBuffer.lights[0].position = sunPosition;
        lightCPUBuffer.viewPos = camera.GetPosition();
        lightGPUBuffer.Update(gm, lightCPUBuffer);

        float shipVel = XMVectorGetX(XMVector3Length(shipBody.GetVelocity()));
        float fov = Lerp(fovMin, fovMax, std::clamp(shipVel * shipVel, 0.0f, 1.0f));

        // Update the camera const buffer
        cameraCPUBuffer.view = camera.GetViewMat();
        cameraCPUBuffer.proj = XMMatrixPerspectiveFovLH(fov, aspectRation, nearPlane, farPlane);
        cameraCPUBuffer.viewPos = camera.GetPosition();
        cameraGPUBuffer.Update(gm, cameraCPUBuffer);
        
        XMVECTOR sunWorld = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), sun.GetModelMatrix());
        XMVECTOR sunView = XMVector4Transform(sunWorld, cameraCPUBuffer.view);
        XMVECTOR sunScreen = XMVector4Transform(sunView, cameraCPUBuffer.proj);
        XMFLOAT4 screenPos;
        XMStoreFloat4(&screenPos, sunScreen);
        if (screenPos.x >= -screenPos.w && screenPos.x <= screenPos.w &&
            screenPos.y >= -screenPos.w && screenPos.y <= screenPos.w &&
            screenPos.z >= -screenPos.w && screenPos.z <= screenPos.w)
        {
            float hWindowWidth = windowWidth * 0.5f;
            float hWindowHeight = windowHeight * 0.5f;
            screenPos.x /= screenPos.w;
            screenPos.y /= screenPos.w;
            screenPos.x *= hWindowWidth;
            screenPos.y *= hWindowHeight;
            screenPos.x += hWindowWidth;
            screenPos.y += hWindowHeight;

            float nx = screenPos.x / windowWidth;
            float x0 = std::clamp(Remap(nx, 0.0f, 0.2f, 0.0f, 1.0f), 0.0f, 1.0f);
            float x1 = std::clamp(Remap(1.0f - nx, 0.0f, 0.2f, 0.0f, 1.0f), 0.0f, 1.0f);
            float x = x0 * x1;
            float ny = screenPos.y / windowHeight;
            float y0 = std::clamp(Remap(ny, 0.0f, 0.1f, 0.0f, 1.0f), 0.0f, 1.0f);
            float y1 = std::clamp(Remap(1.0f - ny, 0.0f, 0.1f, 0.0f, 1.0f), 0.0f, 1.0f);
            float y = y0 * y1;
            commonCPUBuffer.screenPos = XMFLOAT2(screenPos.x, screenPos.y);
            commonCPUBuffer.flerActive.x = x*y;
        }
        else
        {
            commonCPUBuffer.flerActive.x = 0.0f;
        }

        commonCPUBuffer.time += dt;
        commonGPUBuffer.Update(gm, commonCPUBuffer);

        // Update the particle system
        particleSystem.Update(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), camera.GetPosition(), gameTime, dt);


        msaaBuffer.Bind(gm);
        msaaBuffer.Clear(gm, 0.1f, 0.1f, 0.3f);
       
        gm.SetRasterizerStateCullBack();
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
            quad.Draw(gm);
        }
        
        
        gm.SetDepthStencilOn();
        scene.Draw(gm);

        particleIL.Bind(gm);
        particleSystem.Draw(gm);
        sm.Get("vert")->Bind(gm);


        msaaBuffer.Resolve(gm);

        gm.SetRasterizerStateCullBack();
        objectCPUBuffer.model = XMMatrixScaling(static_cast<float>(windowWidth), static_cast<float>(windowHeight), 1.0f);
        cameraCPUBuffer.view = XMMatrixIdentity();
        cameraCPUBuffer.proj = XMMatrixOrthographicLH(windowWidth, windowHeight, 0, 100);
        objectGPUBuffer.Update(gm, objectCPUBuffer);
        cameraGPUBuffer.Update(gm, cameraCPUBuffer);

        // Draw to bloom selector buffer
        {
            bloom0Buffer.Bind(gm);
            bloom0Buffer.Clear(gm, 0.0f, 0.0f, 0.0f);
            sm.Get("bloomSelector")->Bind(gm);
            msaaBuffer.BindAsTexture(gm, 0);
            quad.Draw(gm);
            msaaBuffer.UnbindAsTexture(gm, 0);
        }

        // Bloom
        for (int i = 0; i < 5; i++)
        {
            // make horizontal the bloom
            {
                commonCPUBuffer.pad0 = 1.0f;
                commonGPUBuffer.Update(gm, commonCPUBuffer);

                bloom1Buffer.Bind(gm);
                bloom1Buffer.Clear(gm, 0.0f, 0.0f, 0.0f);
                sm.Get("bloom")->Bind(gm);
                bloom0Buffer.BindAsTexture(gm, 0);
                quad.Draw(gm);
                bloom0Buffer.UnbindAsTexture(gm, 0);
            }
            // make vertical the bloom
            {
                commonCPUBuffer.pad0 = 0.0f;
                commonGPUBuffer.Update(gm, commonCPUBuffer);

                bloom0Buffer.Bind(gm);
                bloom0Buffer.Clear(gm, 0.0f, 0.0f, 0.0f);
                sm.Get("bloom")->Bind(gm);
                bloom1Buffer.BindAsTexture(gm, 0);
                quad.Draw(gm);
                bloom1Buffer.UnbindAsTexture(gm, 0);
            }
        }

        
        // Draw to backBuffer
        {
            gm.BindBackBuffer();
            gm.Clear(0.3f, 0.1f, 0.1f);
            sm.Get("postProcess")->Bind(gm);
            msaaBuffer.BindAsTexture(gm, 0);
            bloom0Buffer.BindAsTexture(gm, 1);
            quad.Draw(gm);
            bloom0Buffer.UnbindAsTexture(gm, 1);
            msaaBuffer.UnbindAsTexture(gm, 0);

        }

        gm.Present();

        lastCounter = currentCounter;
        gameTime += dt;
    }
}

int main()
{
    try
    {
        srand(time(0));
        Demo();
    }
    catch (std::exception& e)
    {
        std::cout << "Error: " << e.what() << "\n";
    }
    return 0;
}
#include "Game.h"

using namespace DirectX;

namespace mc
{
    Game::Game()
        : ship(XMFLOAT3(-2.5, 0.0125f, 0), 2.0f, 0.04f)
    {
        // Initialize the engine and get pointer to the main systems
        engine = std::make_unique<Engine>("Solar System Racing", windowWidth, windowHeight);
        gm = &engine->GetGraphicsManager();
        im = &engine->GetInputManager();
        sm = &engine->GetShaderManager();

        LoadShaders();
        LoadTextures();

        // Create a camera
        camera = std::make_unique<Camera>(XMFLOAT3(0, 0, -2), 0.01f, 100.0f,
            (60.0f / 180.0f) * XM_PI, (90.0f / 180.0f) * XM_PI,
            (float)windowWidth / (float)windowHeight);

        // Create particle system
        particleSystem = std::make_unique<ParticleSystem>(*gm, 1000, sm->Get("soFireVer"), sm->Get("soFireGeo"),
            sm->Get("dwFireVer"), sm->Get("dwFirePix"), sm->Get("dwFireGeo"), *thrustTexture.get());

        // create the text renderer
        text = std::make_unique<Text>(*gm, 1000, *fontTexture.get(), 7, 9, sm->Get("fontVert"), sm->Get("fontPixel"));
    
        LoadConstBuffers();
        LoadInputLayouts();
        LoadGeometry();
        LoadCollisionGeometry();
        LoadFrameBuffers();
        LoadScene();

        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&lastCounter);
        gameTime = 0.0;

        // Set initial state
        gm->SetAlphaBlending();
        sm->Get("vert")->Bind(*gm);
        objectGPUBuffer->Bind(*gm);
        cameraGPUBuffer->Bind(*gm);
        lightGPUBuffer->Bind(*gm);
        commonGPUBuffer->Bind(*gm);
    }

    void Game::Run()
    {
        bool freeMode = false;
        while (engine->IsRunning())
        {
            if (im->KeyJustDown(mc::KEY_1))
            {
                freeMode = !freeMode;
            }

            LARGE_INTEGER currentCounter;
            QueryPerformanceCounter(&currentCounter);
            double lastTime = (double)lastCounter.QuadPart / frequency.QuadPart;
            double currentTime = (double)currentCounter.QuadPart / frequency.QuadPart;
            float dt = static_cast<float>(currentTime - lastTime);

            sm->HotReaload(*gm);

            mc::CollisionData* collisionDataArray[] = {
                &collisionDataOuter,
                &collisionDataInner
            };

            if (freeMode == false)
            {
                ship.Update(*im, dt, collisionDataArray, 2);
            }
            shipNode->SetRotation(ship.GetOrientation());

            XMFLOAT3 shipPos = ship.GetPosition();
            shipNode->SetPosition(shipPos.x, shipPos.y, shipPos.z);

            if (freeMode)
            {
                camera->Update(*im, dt);
            }
            else
            {
                camera->FollowShip(ship);
            }

            XMFLOAT3 sunPosition;
            XMStoreFloat3(&sunPosition, sun->GetPosition());
            lightCPUBuffer.lights[0].position = sunPosition;
            lightCPUBuffer.viewPos = camera->GetPosition();
            lightGPUBuffer->Update(*gm, lightCPUBuffer);

            float shipVel = XMVectorGetX(XMVector3Length(ship.GetVelocity()));
            float fov = mc::Utils::Lerp(camera->GetFovMin(), camera->GetFovMax(), std::clamp(shipVel * shipVel, 0.0f, 1.0f));

            // Update the camera const buffer
            cameraCPUBuffer.view = camera->GetViewMat();
            cameraCPUBuffer.proj = XMMatrixPerspectiveFovLH(fov, camera->GetAspectRatio(), camera->GetNearPlane(), camera->GetFarPlane());
            cameraCPUBuffer.viewPos = camera->GetPosition();
            cameraGPUBuffer->Update(*gm, cameraCPUBuffer);

            XMVECTOR sunWorld = XMVector4Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), sun->GetModelMatrix());
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
                float x0 = std::clamp(mc::Utils::Remap(nx, 0.0f, 0.2f, 0.0f, 1.0f), 0.0f, 1.0f);
                float x1 = std::clamp(mc::Utils::Remap(1.0f - nx, 0.0f, 0.2f, 0.0f, 1.0f), 0.0f, 1.0f);
                float x = x0 * x1;
                float ny = screenPos.y / windowHeight;
                float y0 = std::clamp(mc::Utils::Remap(ny, 0.0f, 0.1f, 0.0f, 1.0f), 0.0f, 1.0f);
                float y1 = std::clamp(mc::Utils::Remap(1.0f - ny, 0.0f, 0.1f, 0.0f, 1.0f), 0.0f, 1.0f);
                float y = y0 * y1;
                commonCPUBuffer.screenPos = XMFLOAT2(screenPos.x, screenPos.y);
                commonCPUBuffer.flerActive.x = x * y;
            }
            else
            {
                commonCPUBuffer.flerActive.x = 0.0f;
            }

            commonCPUBuffer.time += dt;
            commonGPUBuffer->Update(*gm, commonCPUBuffer);

            // Update the particle system
            XMFLOAT3 emitDir;
            XMStoreFloat3(&emitDir, ship.GetForward() * -1.0);
            XMFLOAT3 startVel;
            XMStoreFloat3(&startVel, ship.GetVelocity());
            particleSystem->Update(ship.GetPosition(), startVel, emitDir, camera->GetPosition(), gameTime, dt, ship.GetThrust() / ship.GetThrustMax());

            msaaBuffer->Bind(*gm);
            msaaBuffer->Clear(*gm, 0.1f, 0.1f, 0.3f);

            gm->SetRasterizerStateCullBack();
            // Draw sky
            {
                // Compute the sky coord system so is always facing the camera
                // Compute the scale so the quad fill the screen 
                float a = (camera->GetFarPlane() * 0.5f);
                float ys = (std::tanf(fov * 0.5f) * a) * 2.0f;
                float xs = ys * camera->GetAspectRatio();

                XMVECTOR viewDir = camera->GetFront();
                XMMATRIX translationMat = XMMatrixTranslationFromVector(XMLoadFloat3(&camera->GetPosition()) + (viewDir * a));
                XMMATRIX rotationMat =
                    XMMATRIX(camera->GetRight(), camera->GetUp(), camera->GetFront(), XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f));
                XMMATRIX scaleMat = XMMatrixScaling(xs, ys, 1.0f);

                objectCPUBuffer.model = scaleMat * rotationMat * translationMat;
                objectGPUBuffer->Update(*gm, objectCPUBuffer);

                sm->Get("skybox")->Bind(*gm);
                gm->SetDepthStencilOff();
                quadMesh->Draw(*gm);
            }


            gm->SetDepthStencilOn();
            scene->Draw(*gm);

            particleIL->Bind(*gm);
            particleSystem->Draw(*gm);
            sm->Get("vert")->Bind(*gm);


            msaaBuffer->Resolve(*gm);

            gm->SetRasterizerStateCullBack();
            objectCPUBuffer.model = XMMatrixScaling(static_cast<float>(windowWidth), static_cast<float>(windowHeight), 1.0f);
            cameraCPUBuffer.view = XMMatrixIdentity();
            cameraCPUBuffer.proj = XMMatrixOrthographicLH(windowWidth, windowHeight, -1, 100);
            objectGPUBuffer->Update(*gm, objectCPUBuffer);
            cameraGPUBuffer->Update(*gm, cameraCPUBuffer);

            // Draw to bloom selector buffer
            {
                bloom0Buffer->Bind(*gm);
                bloom0Buffer->Clear(*gm, 0.0f, 0.0f, 0.0f);
                sm->Get("bloomSelector")->Bind(*gm);
                msaaBuffer->BindAsTexture(*gm, 0);
                quadMesh->Draw(*gm);
                msaaBuffer->UnbindAsTexture(*gm, 0);
            }

            // Bloom
            for (int i = 0; i < 5; i++)
            {
                // make horizontal the bloom
                {
                    commonCPUBuffer.pad0 = 1.0f;
                    commonGPUBuffer->Update(*gm, commonCPUBuffer);

                    bloom1Buffer->Bind(*gm);
                    bloom1Buffer->Clear(*gm, 0.0f, 0.0f, 0.0f);
                    sm->Get("bloom")->Bind(*gm);
                    bloom0Buffer->BindAsTexture(*gm, 0);
                    quadMesh->Draw(*gm);
                    bloom0Buffer->UnbindAsTexture(*gm, 0);
                }
                // make vertical the bloom
                {
                    commonCPUBuffer.pad0 = 0.0f;
                    commonGPUBuffer->Update(*gm, commonCPUBuffer);

                    bloom0Buffer->Bind(*gm);
                    bloom0Buffer->Clear(*gm, 0.0f, 0.0f, 0.0f);
                    sm->Get("bloom")->Bind(*gm);
                    bloom1Buffer->BindAsTexture(*gm, 0);
                    quadMesh->Draw(*gm);
                    bloom1Buffer->UnbindAsTexture(*gm, 0);
                }
            }


            // Draw to backBuffer
            {
                gm->BindBackBuffer();
                gm->Clear(0.3f, 0.1f, 0.1f);
                sm->Get("postProcess")->Bind(*gm);
                msaaBuffer->BindAsTexture(*gm, 0);
                bloom0Buffer->BindAsTexture(*gm, 1);
                quadMesh->Draw(*gm);
                bloom0Buffer->UnbindAsTexture(*gm, 1);
                msaaBuffer->UnbindAsTexture(*gm, 0);

            }

            // Draw text
            {

                text->Write(*gm, "FPS: " + std::to_string((int)(1.0f / dt)), -windowWidth * 0.5f, windowHeight * 0.5, 7 * 2, 9 * 2);

                static float seconds = 0;
                static int minutes = 0;
                static int hours = 0;
                if (seconds >= 60.0f)
                {
                    minutes++;
                    seconds = 0.0f;
                }
                if (minutes >= 60.0f)
                {
                    hours++;
                    minutes = 0;
                }
                std::string time = std::to_string(hours) + ":" + std::to_string(minutes) + ":" + std::to_string(seconds);
                seconds += dt;
                text->Write(*gm, "Time: " + time, -windowWidth * 0.5f, (windowHeight * 0.5) - 9 * 2, 7 * 2, 9 * 2);


                text->Render(*gm);
                sm->Get("vert")->Bind(*gm);
            }

            gm->Present();

            lastCounter = currentCounter;
            gameTime += dt;
        }
    }

    void Game::LoadShaders()
    {
        // Init Shaders
        sm->AddVertexShader("vert", *gm, "assets/vertex/vert.hlsl");
        sm->AddVertexShader("fontVert", *gm, "assets/vertex/fontVert.hlsl");
        sm->AddPixelShader("fontPixel", *gm, "assets/pixel/fontPixel.hlsl");
        sm->AddPixelShader("postProcess", *gm, "assets/pixel/postProcess.hlsl");
        sm->AddPixelShader("earth", *gm, "assets/pixel/earth.hlsl");
        sm->AddPixelShader("mars", *gm, "assets/pixel/mars.hlsl");
        sm->AddPixelShader("skybox", *gm, "assets/pixel/skybox.hlsl");
        sm->AddPixelShader("sun", *gm, "assets/pixel/sun.hlsl");
        sm->AddPixelShader("trackBase", *gm, "assets/pixel/trackBase.hlsl");
        sm->AddPixelShader("trackRail", *gm, "assets/pixel/trackRail.hlsl");
        sm->AddPixelShader("ship", *gm, "assets/pixel/ship.hlsl");
        sm->AddPixelShader("meta", *gm, "assets/pixel/meta.hlsl");
        sm->AddPixelShader("postes", *gm, "assets/pixel/postes.hlsl");
        sm->AddPixelShader("bloomSelector", *gm, "assets/pixel/bloomSelector.hlsl");
        sm->AddPixelShader("bloom", *gm, "assets/pixel/bloom.hlsl");
        sm->AddPixelShader("jupiter", *gm, "assets/pixel/jupiter.hlsl");
        sm->AddPixelShader("saturn", *gm, "assets/pixel/saturn.hlsl");

        // Load Shaders for the particle system
        sm->AddVertexShader("soFireVer", *gm, "assets/vertex/soFireVer.hlsl");
        sm->AddGeometryShader("soFireGeo", *gm, "assets/geometry/soFireGeo.hlsl", true);
        sm->AddVertexShader("dwFireVer", *gm, "assets/vertex/dwFireVer.hlsl");
        sm->AddPixelShader("dwFirePix", *gm, "assets/pixel/dwFirePix.hlsl");
        sm->AddGeometryShader("dwFireGeo", *gm, "assets/geometry/dwFireGeo.hlsl");

    }

    void Game::LoadTextures()
    {
        shipTexture = std::make_unique<Texture>(*gm, "assets/textures/Overtone_Default_Diffuse.png");
        shipTexture = std::make_unique<Texture>(*gm, "assets/textures/Overtone_Default_Diffuse.png");
        jupiterTexture = std::make_unique<Texture>(*gm, "assets/textures/planet.png");
        saturnTexture = std::make_unique<Texture>(*gm, "assets/textures/planet2.png");
        thrustTexture = std::make_unique<Texture>(*gm, "assets/textures/neon.png");
        fontTexture = std::make_unique<Texture>(*gm, "assets/textures/font.png");
    }

    void Game::LoadConstBuffers()
    {
        objectCPUBuffer.model = XMMatrixIdentity();
        objectGPUBuffer = std::make_unique<ConstBuffer<ObjectConstBuffer>>(*gm, mc::BIND_TO_VS, objectCPUBuffer, 0);

        cameraCPUBuffer.view = camera->GetViewMat();
        cameraCPUBuffer.proj = XMMatrixPerspectiveFovLH(camera->GetFovMin(), camera->GetAspectRatio(), camera->GetNearPlane(), camera->GetFarPlane());
        cameraCPUBuffer.viewPos = camera->GetPosition();
        cameraGPUBuffer = std::make_unique<ConstBuffer<CameraConstBuffer>>(*gm, mc::BIND_TO_VS | mc::BIND_TO_GS, cameraCPUBuffer, 1);

        lightCPUBuffer.count = 1;
        lightCPUBuffer.lights[0].position = XMFLOAT3(0, 10, 10);
        lightCPUBuffer.lights[0].constant = 1.0f;
        lightCPUBuffer.lights[0].linear = 0.0014f;
        lightCPUBuffer.lights[0].quadratic = 0.000007;
        lightCPUBuffer.lights[0].ambient = XMFLOAT3(0.025f, 0.025f, 0.025f);
        lightCPUBuffer.lights[0].diffuse = XMFLOAT3(100.0f, 100.0f, 100.0f);
        lightCPUBuffer.lights[0].specular = XMFLOAT3(60.0f, 60.0f, 60.0f);
        lightCPUBuffer.viewPos = camera->GetPosition();
        lightGPUBuffer = std::make_unique<ConstBuffer<LightConstBuffer>>(*gm, mc::BIND_TO_PS, lightCPUBuffer, 2);

        commonCPUBuffer.resolution = XMFLOAT2(windowWidth, windowHeight);
        commonCPUBuffer.time = 0.0f;
        commonGPUBuffer = std::make_unique<ConstBuffer<CommonConstBuffer>>(*gm, mc::BIND_TO_PS, commonCPUBuffer, 3);
    }

    void Game::LoadInputLayouts()
    {
        mc::InputLayoutDesc desc = {
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,    0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0}
    },
    4
        };
        IL = std::make_unique<InputLayout>(*gm, *(mc::VertexShader*)sm->Get("vert"), desc);

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
        particleIL = std::make_unique<InputLayout>(*gm, *(mc::VertexShader*)sm->Get("soFireVer"), particleILDesc);
    }

    void Game::LoadGeometry()
    {
        MeshData quadData;
        GeometryGenerator::GenerateQuad(quadData);
        quadVB = std::make_unique<VertexBuffer>(*gm, quadData.vertices.data(), quadData.vertices.size(), sizeof(mc::Vertex));
        quadMesh = std::make_unique<Mesh>(*gm, quadVB.get(), IL.get(), nullptr, quadData.vertices.size(), false);

        // Create track base
        MeshData trackBaseData;
        GeometryGenerator::LoadOBJFile(trackBaseData, "assets/mesh/track_base_tri.obj");
        trackBaseVB = std::make_unique<VertexBuffer>(*gm, trackBaseData.vertices.data(), trackBaseData.vertices.size(), sizeof(mc::Vertex));
        trackBaseMesh = std::make_unique<Mesh>(*gm, trackBaseVB.get(), IL.get(), nullptr, trackBaseData.vertices.size(), false);

        // Create track inner
        MeshData trackInnerData;
        GeometryGenerator::LoadOBJFile(trackInnerData, "assets/mesh/track_inner_tri.obj");
        trackInnerVB = std::make_unique<VertexBuffer>(*gm, trackInnerData.vertices.data(), trackInnerData.vertices.size(), sizeof(mc::Vertex));
        trackInnerMesh = std::make_unique<Mesh>(*gm, trackInnerVB.get(), IL.get(), nullptr, trackInnerData.vertices.size(), false);

        // Create track outer
        MeshData trackOuterData;
        GeometryGenerator::LoadOBJFile(trackOuterData, "assets/mesh/track_outer_tri.obj");
        trackOuterVB = std::make_unique<VertexBuffer>(*gm, trackOuterData.vertices.data(), trackOuterData.vertices.size(), sizeof(mc::Vertex));
        trackOuterMesh = std::make_unique<Mesh>(*gm, trackOuterVB.get(), IL.get(), nullptr, trackOuterData.vertices.size(), false);

        // Create ship
        MeshData shipData;
        GeometryGenerator::LoadOBJFile(shipData, "assets/mesh/ship.obj");
        shipVB = std::make_unique<VertexBuffer>(*gm, shipData.vertices.data(), shipData.vertices.size(), sizeof(mc::Vertex));
        shipMesh = std::make_unique<Mesh>(*gm, shipVB.get(), IL.get(), nullptr, shipData.vertices.size(), false);

        // Create planets
        MeshData planetData;
        GeometryGenerator::LoadOBJFile(planetData, "assets/mesh/planet.obj");
        planetVB = std::make_unique<VertexBuffer>(*gm, planetData.vertices.data(), planetData.vertices.size(), sizeof(mc::Vertex));
        planetMesh = std::make_unique<Mesh>(*gm, planetVB.get(), IL.get(), nullptr, planetData.vertices.size(), false);

        // Create meta
        MeshData metaData;
        GeometryGenerator::LoadOBJFile(metaData, "assets/mesh/meta.obj");
        metaVB = std::make_unique<VertexBuffer>(*gm, metaData.vertices.data(), metaData.vertices.size(), sizeof(mc::Vertex));
        metaMesh = std::make_unique<Mesh>(*gm, metaVB.get(), IL.get(), nullptr, metaData.vertices.size(), false);

        // Create postes
        MeshData postesData;
        GeometryGenerator::LoadOBJFile(postesData, "assets/mesh/postes.obj");
        postesVB = std::make_unique<VertexBuffer>(*gm, postesData.vertices.data(), postesData.vertices.size(), sizeof(mc::Vertex));
        postesMesh = std::make_unique<Mesh>(*gm, postesVB.get(), IL.get(), nullptr, postesData.vertices.size(), false);

    }

    void Game::LoadCollisionGeometry()
    {
        GeometryGenerator::LoadCollisionDataFromOBJFile(collisionDataOuter, "assets/mesh/track_outer_col.obj");
        GeometryGenerator::LoadCollisionDataFromOBJFile(collisionDataInner, "assets/mesh/track_inner_col.obj");
    }

    void Game::LoadFrameBuffers()
    {
        msaaBuffer = std::make_unique<FrameBuffer>(*gm, 0, 0, windowWidth, windowHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, true, 8);
        bloom0Buffer = std::make_unique<FrameBuffer>(*gm, 0, 0, windowWidth, windowHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);
        bloom1Buffer = std::make_unique<FrameBuffer>(*gm, 0, 0, windowWidth, windowHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);
    }

    void Game::LoadScene()
    {
        // Initlializa the scene
        scene = std::make_unique<Scene>(&objectCPUBuffer, objectGPUBuffer.get());

        // Create Ship
        shipNode = &scene->AddNode();
        shipNode->SetMesh(shipMesh.get());
        shipNode->SetTexture(shipTexture.get());
        shipNode->SetVertexShader((VertexShader*)sm->Get("vert"));
        shipNode->SetPixelShader((PixelShader*)sm->Get("ship"));
        shipNode->SetPosition(ship.GetPosition().x, ship.GetPosition().y, ship.GetPosition().z);
        shipNode->SetScale(0.0125f * 0.5f, 0.0125f * 0.5f, 0.0125f * 0.5f);

        // Create meta
        mc::SceneNode& meta = scene->AddNode();
        meta.SetMesh(metaMesh.get());
        meta.SetVertexShader((VertexShader*)sm->Get("vert"));
        meta.SetPixelShader((PixelShader*)sm->Get("meta"));
        meta.SetPosition(0, 0, 0);
        meta.SetScale(1.0f, 1.0f, 1.0f);

        // Create postes
        mc::SceneNode& postes = scene->AddNode();
        postes.SetMesh(postesMesh.get());
        postes.SetVertexShader((VertexShader*)sm->Get("vert"));
        postes.SetPixelShader((PixelShader*)sm->Get("postes"));
        postes.SetPosition(0, 0, 0);
        postes.SetScale(1.0f, 1.0f, 1.0f);

        // Create sun
        sun = &scene->AddNode();
        sun->SetMesh(planetMesh.get());
        sun->SetVertexShader((VertexShader*)sm->Get("vert"));
        sun->SetPixelShader((PixelShader*)sm->Get("sun"));
        sun->SetPosition(0, 10, 40);
        sun->SetScale(10, 10, 10);

        // Create the earth
        mc::SceneNode& earth = scene->AddNode();
        earth.SetMesh(planetMesh.get());
        earth.SetVertexShader((VertexShader*)sm->Get("vert"));
        earth.SetPixelShader((PixelShader*)sm->Get("earth"));
        earth.SetPosition(0, -20.5, 0);
        earth.SetScale(20, 20, 20);

        // Create the mars
        mc::SceneNode& mars = scene->AddNode();
        mars.SetMesh(planetMesh.get());
        mars.SetVertexShader((VertexShader*)sm->Get("vert"));
        mars.SetPixelShader((PixelShader*)sm->Get("mars"));
        mars.SetPosition(40, 0, 30);
        mars.SetScale(10, 10, 10);

        // Create the jupiter
        mc::SceneNode& jupiter = scene->AddNode();
        jupiter.SetMesh(planetMesh.get());
        jupiter.SetTexture(jupiterTexture.get());
        jupiter.SetVertexShader((VertexShader*)sm->Get("vert"));
        jupiter.SetPixelShader((PixelShader*)sm->Get("jupiter"));
        jupiter.SetPosition(-80, 0, 0);
        jupiter.SetScale(20, 20, 20);

        // create the saturn
        mc::SceneNode& saturn = jupiter.AddNode();
        saturn.SetMesh(planetMesh.get());
        saturn.SetTexture(saturnTexture.get());
        saturn.SetVertexShader((VertexShader*)sm->Get("vert"));
        saturn.SetPixelShader((PixelShader*)sm->Get("saturn"));
        saturn.SetPosition(-20, 15, -20);
        saturn.SetScale(10, 10, 10);


        //////////////////////////////////////
        // Add translucid objects
        //////////////////////////////////////
        // Create track base
        mc::SceneNode& trackBase = scene->AddNode();
        trackBase.SetMesh(trackBaseMesh.get());
        trackBase.SetVertexShader((VertexShader*)sm->Get("vert"));
        trackBase.SetPixelShader((PixelShader*)sm->Get("trackBase"));
        trackBase.SetPosition(0, 0, 0);
        trackBase.SetScale(1, 1, 1);

        // Create track inner
        mc::SceneNode& trackInner = scene->AddNode();
        trackInner.SetMesh(trackInnerMesh.get());
        trackInner.SetVertexShader((VertexShader*)sm->Get("vert"));
        trackInner.SetPixelShader((PixelShader*)sm->Get("trackRail"));
        trackInner.SetPosition(0, 0, 0);
        trackInner.SetScale(1, 1, 1);
        trackInner.SetCullBack(false);

        // Create track outer
        mc::SceneNode& trackOuter = scene->AddNode();
        trackOuter.SetMesh(trackOuterMesh.get());
        trackOuter.SetVertexShader((VertexShader*)sm->Get("vert"));
        trackOuter.SetPixelShader((PixelShader*)sm->Get("trackRail"));
        trackOuter.SetPosition(0, 0, 0);
        trackOuter.SetScale(1, 1, 1);
        trackOuter.SetCullBack(false);
    }

}

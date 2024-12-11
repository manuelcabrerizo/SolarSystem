#pragma once

#include <iostream>
#include <cmath>
#include <list>

#include <DirectXMath.h>
#include "Engine.h"

#include "Ship.h"
#include "Scene.h"

namespace mc
{
    class Game
    {
    public:
        Game();
        void Run();
    private:
        void LoadShaders();
        void LoadTextures();
        void LoadConstBuffers();
        void LoadInputLayouts();
        void LoadGeometry();
        void LoadCollisionGeometry();
        void LoadFrameBuffers();
        void LoadScene();

        void UpdateShip(float dt);
        void UpdateShipLapsAndTimes(float dt);
        void UpdateConstBuffers(float dt, float fov);
        void UpdateParticleSystem(float dt);

        void Draw3DScene(float fov);

        void Begin2DMode();
        void DrawBloom();
        void DrawPostProcess();
        void DrawUI(float dt);

        const int windowWidth{ 1366 };
        const int windowHeight{ 768 };

        // Systems
        std::unique_ptr<Engine> engine;
        GraphicsManager *gm;
        InputManager *im;
        ShaderManager *sm;

        // Game specific systems
        std::unique_ptr<Camera> camera;
        std::unique_ptr<ParticleSystem> particleSystem;
        std::unique_ptr<Text> text;

        // Textures
        std::unique_ptr<Texture> shipTexture;
        std::unique_ptr<Texture> jupiterTexture;
        std::unique_ptr<Texture> saturnTexture;
        std::unique_ptr<Texture> thrustTexture;
        std::unique_ptr<Texture> fontTexture;

        // Const Buffers - Uniforms
        ObjectConstBuffer objectCPUBuffer;
        std::unique_ptr<ConstBuffer<ObjectConstBuffer>> objectGPUBuffer;
        CameraConstBuffer cameraCPUBuffer;
        std::unique_ptr<ConstBuffer<CameraConstBuffer>> cameraGPUBuffer;
        LightConstBuffer lightCPUBuffer;
        std::unique_ptr<ConstBuffer<LightConstBuffer>> lightGPUBuffer;
        CommonConstBuffer commonCPUBuffer;
        std::unique_ptr<ConstBuffer<CommonConstBuffer>> commonGPUBuffer;

        // Input layouts
        std::unique_ptr<InputLayout> IL;
        std::unique_ptr<InputLayout> particleIL;

        // Geometry
        std::unique_ptr<VertexBuffer> quadVB;
        std::unique_ptr<Mesh> quadMesh;
        std::unique_ptr<VertexBuffer> trackBaseVB;
        std::unique_ptr<Mesh> trackBaseMesh;
        std::unique_ptr<VertexBuffer> trackInnerVB;
        std::unique_ptr<Mesh> trackInnerMesh;
        std::unique_ptr<VertexBuffer> trackOuterVB;
        std::unique_ptr<Mesh> trackOuterMesh;
        std::unique_ptr<VertexBuffer> shipVB;
        std::unique_ptr<Mesh> shipMesh;
        std::unique_ptr<VertexBuffer> planetVB;
        std::unique_ptr<Mesh> planetMesh;
        std::unique_ptr<VertexBuffer> metaVB;
        std::unique_ptr<Mesh> metaMesh;
        std::unique_ptr<VertexBuffer> postesVB;
        std::unique_ptr<Mesh> postesMesh;

        // Collision Geometry
        CollisionData collisionDataOuter;
        CollisionData collisionDataInner;

        // Frame buffers
        std::unique_ptr<FrameBuffer> msaaBuffer;
        std::unique_ptr<FrameBuffer> bloom0Buffer;
        std::unique_ptr<FrameBuffer> bloom1Buffer;

        // Scene
        std::unique_ptr<Scene> scene;

        // entities
        Ship ship;
        SceneNode* shipNode;
        SceneNode* sun;

        // Clock
        LARGE_INTEGER lastCounter;
        LARGE_INTEGER frequency;
        double gameTime{ 0.0 };
        float timeScale{ 1.0f };

        // Gameplay
        unsigned int currentCheckPoint{ 0 };
        float lastFrameAngle{-1.0f};

        double currentLapTime{0};
        double lastLapTime{0};
        double bestLapTime{0};

        bool lapsStart{ false };
        bool firstLap{ true };
    };
}


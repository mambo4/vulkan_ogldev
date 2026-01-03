#pragma once

//lib
#include <glfw3.h>
#include <glm.hpp>
#include <ext.hpp>

// std
#include <iostream>
#include <algorithm>

static bool constexpr CAMERA_LEFT_HANDED =true;

struct PerspectiveProjectionInfo
{
    float FOV;
    float width;
    float height;
    float zNear;
    float zFar;
};

struct CameraMovement 
{
    bool forward = false;
    bool back  = false;
    bool left  = false;
    bool right  = false;
    bool up  = false;
    bool down = false;
    bool fast = false;
};

struct MouseState
{
    glm::vec2 m_pos = glm::vec2(0.0f);
    bool m_buttonPressed=false;
};

class GLMCameraFirstPerson{

    public:
        CameraMovement m_movement;
        MouseState m_mouseState;
        float m_acceleration=150.0f;
        float m_damping=5.0f;
        float m_maxSpeed=10.0f;
        float m_fastCoefficient=10.f;
        float m_mouseSpeed =4.0f;

        GLMCameraFirstPerson(){}

        GLMCameraFirstPerson(
            const glm::vec3& pos,
            const glm::vec3& target,
            const glm::vec3& up,
            PerspectiveProjectionInfo& perspectiveProjectionInfo
        );

        void Init(
            const glm::vec3& pos,
            const glm::vec3& target,
            const glm::vec3& up,
            PerspectiveProjectionInfo& perspectiveProjectionInfo
        );

        void Update(float dt);
        void SetMousePos( float xpos, float ypos );
        void HandleMouseButton(int button, int action, int mods);
        const glm::mat4& GetProjectionMatrix() const {return m_perspectiveProjection;}

        glm::vec3 GetPosition() const {return m_cameraPos;}
        glm::vec3 GetTarget() const;
        glm::vec3 GetUp() const { return m_up; }
        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetVPMatrix() const;
        glm::mat4 GetVPMatrixNoTranslate() const;
        const PerspectiveProjectionInfo& GetPerspectiveProjectionInfo() const { return m_perspectiveProjectionInfo; }

        void SetPos(const glm::vec3& Pos) { m_cameraPos = Pos; }
        void SetUp(const glm::vec3& Up) { m_up = Up; }
        void SetTarget(const glm::vec3& Target);
        void SetAbsTarget(const glm::vec3& Target);

    private:
        void UpdateVelocity(float dt);
        glm::vec3 GetAcceleration();
        void UpdateCameraOrientation();
        void SetUpVector();

        glm::mat4 m_perspectiveProjection = glm::mat4(0.0);
        glm::vec3 m_cameraPos=glm::vec3(0.0f);
        glm::quat m_cameraRot=glm::quat(glm::vec3(0.0f));
        glm::vec3 m_velocity=glm::vec3(0.0f);
        glm::vec2 m_oldMousePos =glm::vec2(0.0f);
        glm::vec3 m_up=glm::vec3(0.0f);
        
        PerspectiveProjectionInfo m_perspectiveProjectionInfo;
};

bool GLFWCameraHandler(CameraMovement& movement, int key, int action, int mods);
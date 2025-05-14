// #pragma once

// #include "Core/Config.h"
// #include "Core/Timestep.h"

// namespace UE {
//     // Default camera values
//     // const float Default_YAW         = -90.0f;
//     // const float Default_PITCH       =  0.0f;
//     // const float Default_SPEED       =  2.5f;
//     // const float Default_SENSITIVITY =  0.1f;
//     // const float Default_ZOOM        =  45.0f;

//     class  Camera3D {
//     public:
//         // constructor with vectors
//         Camera3D(const glm::vec3& position = glm::vec3(0), const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
//                  const float& yaw = -90.0f, const float& pitch = 0.0f);
//         // constructor with scalar values   
//         Camera3D(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);
//         // returns the view matrix calculated using Euler Angles and the LookAt Matrix
//         const glm::mat4& GetViewMatrix() const {
//             return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
//         }

//         const glm::mat4& GetProjectionMatrix() const{
//             return glm::perspective(glm::radians(m_Zoom), m_Aspect, 0.1f, 100.0f);
//         }

//         void SetAspect(float aspect){
//             m_Aspect = aspect;
//         }

//         const glm::vec3& GetPosition() const {
//             return m_Position;
//         }
    
//         // processes input received from any keyboard-like input system.         
//         void ProcessInputAndMouse(Timestep ts, float xoffset, float yoffset, bool constrainPitch = true);

//     private:
//         void updateCameraVectors();

//     private:
//         // camera Attributes
//         glm::vec3 m_Position;
//         glm::vec3 m_Front;
//         glm::vec3 m_Up;
//         glm::vec3 m_Right;
//         glm::vec3 m_WorldUp;
//         // euler Angles
//         float m_Yaw;
//         float m_Pitch;
//         // camera options
//         float m_MovementSpeed;
//         float m_MouseSensitivity;
//         float m_Zoom;
//         float m_Aspect;
//     };
// }
// #include "Renderer/Camera3D.h"
// #include "Core/Input.h"

// namespace UE {

//     Camera3D::Camera3D(const glm::vec3& position, const glm::vec3& up,
//                         const float& yaw, const float& pitch)
//         :m_Front(glm::vec3(0.0f, 0.0f, -1.0f)), m_MovementSpeed(2.5f), 
//          m_MouseSensitivity(0.1f), m_Zoom(45.0f)
//     {
//         m_Position = position;
//         m_WorldUp = m_Up;
//         m_Yaw = yaw;
//         m_Pitch = pitch;

//         updateCameraVectors();
//     }

//     Camera3D::Camera3D(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
//         :m_Front(glm::vec3(0.0f, 0.0f, -1.0f)), m_MovementSpeed(2.5f), 
//         m_MouseSensitivity(0.1f), m_Zoom(45.0f)
//     {
//         m_Position = glm::vec3(posX, posY, posZ);
//         m_WorldUp = glm::vec3(upX, upY, upZ);
//         m_Yaw = yaw;
//         m_Pitch = pitch;
//         updateCameraVectors();
//     }

//     void Camera3D::ProcessInputAndMouse(Timestep ts, float xoffset, float yoffset, bool constrainPitch){
//         float velocity = m_MovementSpeed * ts;
//         if(Input::IsKeyPressed(Key::W)){
//             m_Position += m_Front * velocity;
//         }
//         if(Input::IsKeyPressed(Key::S)){
//             m_Position -= m_Front * velocity;       
//         }
//         if(Input::IsKeyPressed(Key::A)){
//             m_Position -= m_Right * velocity;        
//         }
//         if(Input::IsKeyPressed(Key::D)){
//             m_Position += m_Right * velocity;     
//         }
//         if(Input::IsKeyPressed(Key::Space)){
//             m_Position += m_Up * velocity;     
//         }
//         if(Input::IsKeyPressed(Key::LeftControl)){
//             m_Position -= m_Up * velocity;          
//         }


//         // xoffset *= m_MouseSensitivity;
//         // yoffset *= m_MouseSensitivity;

//         // m_Yaw   += xoffset;
//         // m_Pitch += yoffset;

//         // // make sure that when pitch is out of bounds, screen doesn't get flipped
//         // if (constrainPitch)
//         // {
//         //     if (m_Pitch > 89.0f)
//         //         m_Pitch = 89.0f;
//         //     if (m_Pitch < -89.0f)
//         //         m_Pitch = -89.0f;
//         // }

//         // // update Front, Right and Up Vectors using the updated Euler angles
//         updateCameraVectors();
//     }

//     void Camera3D::updateCameraVectors()
//     {
//         // calculate the new Front vector
//         glm::vec3 front;
//         front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
//         front.y = sin(glm::radians(m_Pitch));
//         front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
//         m_Front = glm::normalize(front);
//         // also re-calculate the Right and Up vector
//         m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
//         m_Up    = glm::normalize(glm::cross(m_Right, m_Front));
//     }
// }
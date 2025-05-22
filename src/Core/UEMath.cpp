#include "uepch.h"
#include "Core/UEMath.h"
#include "raymath.h"
#include "Scene/Components.h"

namespace UE::Math {
    // Decomposes a transformation matrix into translation, rotation (in radians), and scale.
    // Assumes the matrix does not contain shear or perspective.
    bool DecomposeTransform(Matrix mat, Vector3* translation, Vector3* rotation, Vector3* scale)
    {
        if (!translation || !rotation || !scale) return false;

        // Extract translation
        translation->x = mat.m12;
        translation->y = mat.m13;
        translation->z = mat.m14;

        // Extract scale from basis vectors
        scale->x = sqrtf(mat.m0 * mat.m0 + mat.m1 * mat.m1 + mat.m2 * mat.m2);
        scale->y = sqrtf(mat.m4 * mat.m4 + mat.m5 * mat.m5 + mat.m6 * mat.m6);
        scale->z = sqrtf(mat.m8 * mat.m8 + mat.m9 * mat.m9 + mat.m10 * mat.m10);

        // Normalize the rotation matrix
        Matrix rotMat = mat;
        if (scale->x != 0.0f) { rotMat.m0 /= scale->x; rotMat.m1 /= scale->x; rotMat.m2 /= scale->x; }
        if (scale->y != 0.0f) { rotMat.m4 /= scale->y; rotMat.m5 /= scale->y; rotMat.m6 /= scale->y; }
        if (scale->z != 0.0f) { rotMat.m8 /= scale->z; rotMat.m9 /= scale->z; rotMat.m10 /= scale->z; }

        // Extract rotation assuming XYZ (pitch-roll-yaw) order
        if (rotation)
        {
            // Safe guard: clamp for asin
            float sy = -rotMat.m2;
            sy = fminf(fmaxf(sy, -1.0f), 1.0f);

            rotation->x = atan2f(rotMat.m6, rotMat.m10); // X (pitch)
            rotation->y = asinf(sy);                    // Y (yaw)
            rotation->z = atan2f(rotMat.m1, rotMat.m0); // Z (roll)

            // Convert to degrees if needed
            rotation->x *= RAD2DEG;
            rotation->y *= RAD2DEG;
            rotation->z *= RAD2DEG;
        }

        return true;
    }

    Matrix GetTransformMatrix(Vector3 position, Quaternion rotation, Vector3 scale) {
		Matrix translation = MatrixTranslate(position.x, position.y, position.z);
		Matrix rotationMat = QuaternionToMatrix(rotation);
		Matrix scaleMat = MatrixScale(scale.x, scale.y, scale.z);
		return MatrixMultiply(MatrixMultiply(translation, rotationMat), scaleMat);
	}

    Matrix GetTransformMatrix(const TransformComponent& tc)
    {
        Matrix t = MatrixTranslate(tc.Translation.x, tc.Translation.y, tc.Translation.z);
        Matrix r = MatrixRotateXYZ(Vector3{ DEG2RAD * tc.Rotation.x, DEG2RAD * tc.Rotation.y, DEG2RAD * tc.Rotation.z });
        Matrix s = MatrixScale(tc.Scale.x, tc.Scale.y, tc.Scale.z);
        return MatrixMultiply(MatrixMultiply(t, r), s);
    }

    Matrix GetCameraViewMatrix(Camera3D camera)
    {
        return MatrixLookAt(camera.position, camera.target, camera.up);
    }

    Matrix GetCameraProjectionMatrix(Camera3D camera, float viewportWidth, float viewportHeight)
    {
        float aspect = viewportWidth / viewportHeight;
        return MatrixPerspective(DEG2RAD * camera.fovy, aspect, 0.1f, 1000.0f);
    }
}
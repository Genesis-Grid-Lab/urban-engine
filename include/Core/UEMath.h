#pragma once

#include <Core/Config.h>
namespace UE {

    struct TransformComponent;
}

namespace UE::Math {

	bool DecomposeTransform(Matrix mat, Vector3* translation, Vector3* rotation, Vector3* scale);
	Matrix GetTransformMatrix(Vector3 position, Quaternion rotation, Vector3 scale);
    Matrix GetTransformMatrix(const TransformComponent& tc);
    Matrix GetCameraViewMatrix(Camera3D camera);
    Matrix GetCameraProjectionMatrix(Camera3D camera, float viewportWidth, float viewportHeight);
	// void MatrixDecompose(Matrix mat, Vector3 *outTranslation, Quaternion *outRotation, Vector3 *outScale);

}
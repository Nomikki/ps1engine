#include "camera.hpp"

Camera::Camera(/* args */)
{
  vUp = {0, 1, 0};
}

Camera::~Camera()
{
}

void Camera::Update(float dt)
{
  vForward = Vector_Mul(lookDir, dt);
  vRight = Vector_Mul(lookDirRight, dt);

  vTarget = {0, 0, 1};

  mat4x4 matCameraRot = Matrix_MakeRotationY(yaw);
  mat4x4 matCameraRotRight = Matrix_MakeRotationY(yaw + (90 / 180.0 * 3.1415f));

  lookDir = Matrix_MultiplyVector(matCameraRot, vTarget);
  lookDirRight = Matrix_MultiplyVector(matCameraRotRight, vTarget);

  vTarget = Vector_Add(pos, lookDir);
}

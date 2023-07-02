#include "transform.hpp"

Transform::Transform() {
    calculateAngles(0, 0, 0);
}
Transform::~Transform() {}

void Transform::setupMatrix(mat4x4 &m3)
{

  matWorld = Matrix_MakeIdentity();
  matWorld = Matrix_MultiplyMatrix(matRotY, matRotX);
  matWorld = Matrix_MultiplyMatrix(matWorld, m3);
}

void Transform::calculateAngles(float x, float y, float z)
{
  matRotZ = Matrix_MakeRotationZ(x);
  matRotY = Matrix_MakeRotationY(y);
  matRotX = Matrix_MakeRotationX(z);
}

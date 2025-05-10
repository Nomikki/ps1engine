#include "transform.hpp"
#include <iostream>

Transform::Transform()
{
  calculateAngles(0, 0, 0);
}
Transform::~Transform() {}

void Transform::setupMatrix(mat4x4 &m3)
{

  matWorld = Matrix_MakeIdentity();
  matWorld = Matrix_MultiplyMatrix(matRotY, matRotX);
  matWorld = Matrix_MultiplyMatrix(matWorld, m3);
}

void Transform::calculateAngles(float angleX, float angleY, float angleZ)
{
  // std::cout << "Calculating angles: angleX=" << angleX << " angleY=" << angleY << " angleZ=" << angleZ << std::endl;
  matRotX = Matrix_MakeRotationX(angleX);
  matRotY = Matrix_MakeRotationY(angleY);
  matRotZ = Matrix_MakeRotationZ(angleZ);

  // Debug output for rotation matrices
  // std::cout << "Rotation Matrix X (from angleX input):\\n";
  // for (int i = 0; i < 4; ++i) {
  //   for (int j = 0; j < 4; ++j) {
  //     std::cout << matRotX.m[i][j] << " ";
  //   }
  //   std::cout << std::endl;
  // }

  // std::cout << "Rotation Matrix Y (from angleY input):\\n";
  // for (int i = 0; i < 4; ++i) {
  //   for (int j = 0; j < 4; ++j) {
  //     std::cout << matRotY.m[i][j] << " ";
  //   }
  //   std::cout << std::endl;
  // }

  // std::cout << "Rotation Matrix Z (from angleZ input):\\n";
  // for (int i = 0; i < 4; ++i) {
  //   for (int j = 0; j < 4; ++j) {
  //     std::cout << matRotZ.m[i][j] << " ";
  //   }
  //   std::cout << std::endl;
  // }
}

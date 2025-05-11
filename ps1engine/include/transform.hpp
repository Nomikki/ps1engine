#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include "utility.hpp"

class Transform
{
public:
  Vec3 pos;
  Vec3 rot;

  Transform();
  ~Transform();

  void setupMatrix(mat4x4 &m3);
  void calculateAngles(float x, float y, float z);

  mat4x4 matWorld;

private:
  mat4x4 matRotZ, matRotX, matRotY;
};

#endif // __TRANSFORM_H__
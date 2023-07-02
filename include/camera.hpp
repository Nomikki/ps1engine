#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "utility.hpp"

class Camera
{

public:
  Camera();
  ~Camera();

  Vec3 pos = {0, 0, 0};
  Vec3 lookDir;
  Vec3 lookDirRight;

  Vec3 vUp = {0, 1, 0};
  Vec3 vTarget = {0, 0, 1};

  float yaw = 0;

  Vec3 vForward;
  Vec3 vRight;

  void Update(float dt);

  
  

private:
};

#endif // __CAMERA_H__
#ifndef __COMPONENTMANAGER_H__
#define __COMPONENTMANAGER_H__

#include <vector>
#include "component.hpp"

struct Components
{
  std::vector<Component> components;
  bool createFromFile(std::string filename, int textureID = 0, Vec3 pos = {0, 0, 0});
};

#endif // __COMPONENTMANAGER_H__
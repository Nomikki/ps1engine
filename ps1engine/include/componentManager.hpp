#ifndef __COMPONENTMANAGER_H__
#define __COMPONENTMANAGER_H__

#include <vector>
#include "component.hpp"

struct Components
{
  std::vector<Component> components;
  bool createFromFile(std::string filename, int textureID, Vec3 pos, bool centerComponent = true);
  Component &getOrCreate(uint32_t ID);
};

#endif // __COMPONENTMANAGER_H__
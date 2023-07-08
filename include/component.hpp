#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include <string>
#include "mesh.hpp"
#include "meshManager.hpp"
#include "transform.hpp"

class Component;

class Component
{
public:
  Component();
  ~Component();

  bool createMeshFromFile(std::string filename, int textureID);
  Meshes meshes;
  Transform transform;

private:
  Component *child;
};

#endif // __COMPONENT_H__
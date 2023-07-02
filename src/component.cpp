#include "component.hpp"

Component::Component()
{
  transform.pos = {0, 0, 0};
  transform.rot = {0, 0, 0};
}

Component::~Component()
{
}

bool Component::createMeshFromFile(std::string filename) {
  Mesh m;
  meshes.meshes.push_back(m);
  return meshes.meshes[0].LoadObjFromFile(filename);
}
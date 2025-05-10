#ifndef __MESH_H__
#define __MESH_H__

/*
  Todo:
  - precalculate normals (for flat and gouraud)
*/

#include <vector>
#include "utility.hpp"


struct Mesh
{
  std::vector<Triangle> tris;
  AABB aabb;
  bool LoadObjFromFile(std::string filename, bool centerModel = true);
  int textureID;
};

#endif // __MESH_H__
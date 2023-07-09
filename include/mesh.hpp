#ifndef __MESH_H__
#define __MESH_H__

/*
  Todo:
  - create AABB boxes
  - precalculate normals (for flat and gouraud)
*/

#include <vector>
#include "utility.hpp"

struct Mesh
{
  std::vector<Triangle> tris;
  bool LoadObjFromFile(std::string filename);
  int textureID;
};

#endif // __MESH_H__
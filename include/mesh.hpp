#ifndef __MESH_H__
#define __MESH_H__

#include <vector>
#include "utility.hpp"

struct Mesh
{
  std::vector<Triangle> tris;
  bool LoadObjFromFile(std::string filename);
};

#endif // __MESH_H__
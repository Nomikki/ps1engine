#include <mesh.hpp>

bool Mesh::LoadObjFromFile(std::string filename)
{
  std::ifstream f(filename);
  if (!f.is_open())
    return false;

  // local cache of vertices
  std::vector<Vec3> verts;

  // local cache of uv vectors
  std::vector<UV> uvs;

  while (!f.eof())
  {
    char line[128];
    f.getline(line, 128);

    std::strstream s;
    s << line;

    char junk;

    // vertex
    if (line[0] == 'v' && line[1] == ' ')
    {
      Vec3 v;
      s >> junk >> v.x >> v.y >> v.z;
      verts.push_back(v);
    }

    if (line[0] == 'v' && line[1] == 't')
    {
      UV uv;
      std::string junkWord;
      s >> junkWord >> uv.u >> uv.v;
      uvs.push_back(uv);
    }

    // face
    if (line[0] == 'f')
    {
      int f[3];
      int f2[3];
      std::string tmp[3];

      s >> junk >> tmp[0] >> tmp[1] >> tmp[2];

      std::size_t found = tmp[0].find('/', 0);

      if (found != tmp[0].npos)
      {
        for (int o = 0; o < 3; o++)
        {
          std::size_t foundPos = tmp[o].find('/', 0);
          f[o] = std::stoi(tmp[o].substr(0, foundPos));
          f2[o] = std::stoi(tmp[o].substr(foundPos + 1));
        }
      }
      else
      {
        for (int o = 0; o < 3; o++)
        {
          f[o] = std::stoi(tmp[o]);
        }
      }

      tris.push_back({verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1], uvs[f2[0] - 1], uvs[f2[1] - 1], uvs[f2[2] - 1], {255, 255, 255}});
      // printf("f");
    }
  }
  // printf("obj file loaded!\n");
  return true;
}
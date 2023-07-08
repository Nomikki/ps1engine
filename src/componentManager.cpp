#include "componentManager.hpp"

bool Components::createFromFile(std::string filename, int textureID, Vec3 pos)
{
  
  Component c;
  c.transform.pos = pos;
  components.push_back(c);
  if (!components[components.size()-1].createMeshFromFile(filename, textureID))
  {
    printf("file '%s' not found.\n", (const char*)filename.c_str());
  }


}
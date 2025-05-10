#include "componentManager.hpp"
#include <iostream>

bool Components::createFromFile(std::string filename, int textureID, Vec3 pos, bool centerComponent)
{
  Component c;
  c.transform.pos = pos;
  components.push_back(c);
  
  if (!components.back().createMeshFromFile(filename, textureID, centerComponent))
  {
    // std::cerr << "Error: Failed to load mesh from file '" << filename << "'" << std::endl;
    components.pop_back(); // Remove the failed component
    return false;
  }
  return true;
}

Component& Components::getOrCreate(uint32_t ID)
{
    if (ID < components.size()) {
        return components[ID];
    } else {
        // ID is out of bounds, create new components up to ID
        // This part might need more robust handling or design choices
        // depending on expected usage (e.g., error if non-sequential IDs are bad)
        components.resize(ID + 1); // Resize and default-construct new components
        // std::cout << "Warning: Accessed out-of-bounds component ID: " << ID 
        //           << ". Resized components vector to " << components.size() 
        //           << ". Consider pre-allocating or sequential ID usage." << std::endl;
        return components[ID];
    }
}
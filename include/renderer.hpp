/*
  todo:
    - meta data for textures (w, h, data, dithered, size etc)
    - 2d arrays for pixels, update render only once per frame
    - refactoring whole code base
    - point lights
    - gouraud shading
    - when loading image data, quantise it before rendering
      -> when rendering, apply only dither, not quantise

    - check AABB boxes
    - shared resources
    - z buffer


*/
#include <SFML/Graphics.hpp>
#include <cstdio>
#include <vector>
#include <list>
#include <cmath>

#include <algorithm>
#include "utility.hpp"
// #include "meshManager.hpp"
#include "componentManager.hpp"
#include "camera.hpp"

class Renderer
{
public:
  Renderer(int targetFPS = 60, float scale = 1, const char *title = "Unknow app");
  ~Renderer();

  inline void setPixel(int x, int y, Color &color);
  inline Color getPixelFrom(int x, int y, uint8_t *buffer);
  inline void setPixelTo(int x, int y, Color &color, uint8_t *buffer);

  void drawLine(int sx, int sy, int ex, int ey, Color color);
  void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color);
  void fillTriangle(Vec3 &t1, Vec3 &t2, Vec3 &t3, Color color);
  void texturedTriangle(Vec3 &t1, UV &uv1,
                        Vec3 &t2, UV &uv2,
                        Vec3 &t3, UV &uv3,
                        sf::Image &img, Color &color);

  void renderTriangle(Triangle &triangle);

  bool isOpen();
  void checkEvents();
  void render(int debugMode);
  void copyVideoBuffer(uint8_t *buffer);
  void clear();
  void Dither_FloydSteinberg();
  void rasterize();
  void renderAll();
  void calculateTriangles(Vec3 &camera, Vec3 &vTarget, Vec3 &vUp);
  void setSort(bool b);
  float getClock();

  // Mesh meshCube;
  // Meshes meshes;
  Components components;

  mat4x4 matProj;

  int width;
  int height;

  sf::Image textureImage;

  std::vector<Triangle> vecTrianglesToRaster;

private:
  void renderDebugData();

  enum RenderMode
  {
    textured,
    filled,
    wireframe,
  };

  RenderMode rMode;
  StatisticData stData;
  sf::Clock clock;
  float deltaTime;
  float dt;

  int fpsCounter;
  int fpsCounterMax;
  int fpsLimit;

  float scale;

  bool useSort;

  uint8_t *videoBuffer;
  uint8_t *videoBufferBack;
  sf::Image screenBuffer;
  // sf::Image screenBuffer2;

  sf::Texture screenTexture;
  sf::Sprite sprite;

  sf::RenderWindow window;
  sf::Uint8 *px0;
  // sf::Uint8 *px1;
  sf::Uint8 *clearScreenPtr;

  
};

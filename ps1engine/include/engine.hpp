#ifndef __ENGINE_H__
#define __ENGINE_H__

/*
  todo:
    - point lights
    - gouraud shading
    - when loading image data, quantise it before rendering
      -> when rendering, apply only dither, not quantise
    - shared resources
*/

#include <cstdio>
#include <cstring>
#include <vector>
#include <list>
#include <cmath>
#include <algorithm>


#include <SFML/Graphics.hpp>

#include "utility.hpp"
#include "componentManager.hpp"
#include "camera.hpp"

struct TextureMetadata {
    int width;
    int height;
    size_t size;
    bool isDithered;
    bool isQuantized;
    std::string filename;
};

class Engine
{
public:
  Engine(int targetFPS = 60, float scale = 1, const char *title = "Unknow app");
  ~Engine();

  inline void setPixel(int x, int y, Color &color);
  inline Color getPixelFrom(int x, int y, uint8_t *buffer);
  inline void setPixelTo(int x, int y, Color &color, uint8_t *buffer);

  int LoadTexture(std::string filename);
  void QuantizeImage(sf::Image &img);

  void drawLine(int sx, int sy, int ex, int ey, Color color);
  void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color);
  void fillTriangle(Vec3 &t1, Vec3 &t2, Vec3 &t3, Color color);
  void texturedTriangle(Vec3 &t1, UV &uv1, float w1,
                        Vec3 &t2, UV &uv2, float w2,
                        Vec3 &t3, UV &uv3, float w3,
                        sf::Image &img, Color &color);

  void renderTriangle(Triangle &triangle, int textureID = 0);

  bool isOpen();
  void checkEvents();
  void render(int debugMode);
  void copyVideoBuffer(uint8_t *buffer);
  void ClearDepthBufferWithSIMD(float* pDepthBuffer, size_t size);
  void clear();
  void Dither_FloydSteinberg();
  void rasterize();
  void renderAll();
  void calculateTriangles(Vec3 &camera, Vec3 &vTarget, Vec3 &vUp);
  void setSort(bool b);
  void setDither(bool v);
  void setFogColor(const Color& new_color);
  void setVertexSnapping(bool enabled);

  bool checkIfAABBisOnScreen(AABB &aabb, mat4x4 &matWorld, mat4x4 &matView);

  float getClock();

  Components components;

  mat4x4 matProj;

  int width;
  int height;

  std::vector<sf::Image> textureImage;
  std::vector<TextureMetadata> textureMetadata;

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

  bool useDither;
  bool useSort;
  Color fogColor;
  float fogW;
  float clipEnd;
  bool enableVertexSnapping;

  __m128 zero;
  size_t depthBufferSize;

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

  float *pDepthBuffer = nullptr;

  // Helper function for texturedTriangle
  static void SortVerticesByY(Vec3 &p1, Vec3 &p2, Vec3 &p3, 
                              UV &tex1, UV &tex2, UV &tex3, 
                              float &w_val1, float &w_val2, float &w_val3);

  void ScanlineFillTexturedPart(int y_start, int y_end,
                                  float x_edge1_start, const UV& uv_edge1_start, float w_edge1_start,
                                  float x_edge2_start, const UV& uv_edge2_start, float w_edge2_start,
                                  float dx_edge1_step, float du_edge1_step, float dv_edge1_step, float dw_edge1_step,
                                  float dx_edge2_step, float du_edge2_step, float dv_edge2_step, float dw_edge2_step,
                                  sf::Image &texture, const Color &base_color);
};

#endif // __ENGINE_H__
#include "engine.hpp"
#include <iostream>

Engine::Engine(int targetFPS, float scale, const char *title)
{
  width = 256;
  height = 224;

  fogColor = {0, 0, 0};
  fogW = 20;
  clipEnd = 0.0f;

  fpsCounter = 0;
  fpsCounterMax = 10;
  fpsLimit = targetFPS;
  this->scale = scale;
  setDither(false);
  setSort(false);
  enableVertexSnapping = true;
  generate_sincos_lookupTables();

  window.create(sf::VideoMode(width * scale, height * scale), title, sf::Style::Default);
  window.setFramerateLimit(fpsLimit);

  screenBuffer.create(width, height, sf::Color::Black);

  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      screenBuffer.setPixel(x, y, sf::Color::Black);
    }
  }

  pDepthBuffer = new float[width * height];
  if (!pDepthBuffer) {
    throw std::runtime_error("Failed to allocate depth buffer");
  }

  videoBuffer = new uint8_t[width * height * 3];
  if (!videoBuffer) {
    delete[] pDepthBuffer;
    throw std::runtime_error("Failed to allocate video buffer");
  }

  videoBufferBack = new uint8_t[width * height * 3];
  if (!videoBufferBack) {
    delete[] pDepthBuffer;
    delete[] videoBuffer;
    throw std::runtime_error("Failed to allocate video back buffer");
  }

  screenTexture.create(width, height);
  screenTexture.update(screenBuffer);
  sprite.setTexture(screenTexture);
  sprite.setScale(scale, scale);

  dt = getClock();

  px0 = const_cast<sf::Uint8 *>(screenBuffer.getPixelsPtr());

  clearScreenPtr = new uint8_t[width * height * 3];
  if (!clearScreenPtr) {
    delete[] pDepthBuffer;
    delete[] videoBuffer;
    delete[] videoBufferBack;
    throw std::runtime_error("Failed to allocate clear screen buffer");
  }

  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      int i = (y * width + x) * 3;
      clearScreenPtr[i] = fogColor.r;
      clearScreenPtr[i + 1] = fogColor.g;
      clearScreenPtr[i + 2] = fogColor.b;
    }
  }

  float fNear = 0.01f;
  float fFar = 100.0f;
  float fFov = 45.0f;
  float fAspectRatio = (float)height / (float)width;

  matProj = Matrix_MakeProjection(fFov, fAspectRatio, fNear, fFar);
  rMode = RenderMode::textured;

  zero = _mm_setzero_ps();
  depthBufferSize = width * height;
}

void Engine::QuantizeImage(sf::Image &img)
{
    unsigned int width = img.getSize().x;
    unsigned int height = img.getSize().y;

    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            sf::Color pixel = img.getPixel(x, y);
            Color color = {pixel.r, pixel.g, pixel.b};
            Color quantized = quantise(color);
            img.setPixel(x, y, sf::Color(quantized.r, quantized.g, quantized.b, pixel.a));
        }
    }
}

int Engine::LoadTexture(std::string filename)
{
  sf::Image img;
  if (!img.loadFromFile(filename))
  {
    printf("Failed to load texture: %s\n", filename.c_str());
    return -1;
  }

  QuantizeImage(img);

  TextureMetadata meta;
  meta.width = img.getSize().x;
  meta.height = img.getSize().y;
  meta.size = img.getSize().x * img.getSize().y * 4;
  meta.isDithered = false;
  meta.isQuantized = true;
  meta.filename = filename;

  textureImage.push_back(img);
  textureMetadata.push_back(meta);

  return textureImage.size() - 1;
}

Engine::~Engine()
{
  if (videoBuffer != nullptr)
    delete videoBuffer;
  videoBuffer = nullptr;

  if (videoBufferBack != nullptr)
    delete videoBufferBack;
  videoBufferBack = nullptr;

  if (pDepthBuffer != nullptr)
    delete pDepthBuffer;
  pDepthBuffer = nullptr;
}

void Engine::ClearDepthBufferWithSIMD(float *pDepthBuffer, size_t size)
{
  if (size % 4 != 0)
  {
    for (size_t i = size - (size % 4); i < size; ++i)
    {
      pDepthBuffer[i] = 0.0f;
    }
  }

  for (size_t i = 0; i < size / 4; ++i)
  {
    _mm_store_ps(&pDepthBuffer[i * 4], zero);
  }
}

void Engine::clear()
{
  memcpy(videoBuffer, clearScreenPtr, width * height * 3);
  memcpy(videoBufferBack, clearScreenPtr, width * height * 3);
 
  for (int i = 0; i < width * height; i++)
  {
    pDepthBuffer[i] = 0;
  }
}

inline void Engine::setPixel(int x, int y, Color &color)
{
  uint32_t offset = (y * width + x) * 3;

  videoBuffer[offset] = color.r;
  videoBuffer[offset + 1] = color.g;
  videoBuffer[offset + 2] = color.b;
}

inline void Engine::setPixelTo(int x, int y, Color &color, uint8_t *buffer)
{
  uint32_t offset = (y * width + x) * 3;

  buffer[offset] = color.r;
  buffer[offset + 1] = color.g;
  buffer[offset + 2] = color.b;
}

inline Color Engine::getPixelFrom(int x, int y, uint8_t *buffer)
{
  uint32_t offset = (y * width + x) * 3;
  return {buffer[offset], buffer[offset + 1], buffer[offset + 2]};
}

void Engine::SortVerticesByY(Vec3 &p1, Vec3 &p2, Vec3 &p3, 
                             UV &tex1, UV &tex2, UV &tex3, 
                             float &w_val1, float &w_val2, float &w_val3)
{
    if (p2.y < p1.y) {
        std::swap(p1, p2); std::swap(tex1, tex2); std::swap(w_val1, w_val2);
    }
    if (p3.y < p1.y) {
        std::swap(p1, p3); std::swap(tex1, tex3); std::swap(w_val1, w_val3);
    }
    if (p3.y < p2.y) {
        std::swap(p2, p3); std::swap(tex2, tex3); std::swap(w_val2, w_val3);
    }
}

void Engine::Dither_FloydSteinberg()
{
  int32_t w = width;
  int32_t h = height;

  memcpy(videoBufferBack, videoBuffer, w * h * 3);

  constexpr float step = 16.0;
  constexpr float step1 = 7.0f / step;
  constexpr float step2 = 3.0f / step;
  constexpr float step3 = 5.0f / step;
  constexpr float step4 = 1.0f / step;

  Color pixel;
  int32_t error[3];
  int32_t k[3];
  Color p;
  int px, py;

  for (int y = 1; y < h - 1; y++)
  {
    for (int x = 1; x < w - 1; x++)
    {
      pixel = getPixelFrom(x, y, videoBufferBack);

      Color qPixel = pixel;

      error[0] = pixel.r - qPixel.r;
      error[1] = pixel.g - qPixel.g;
      error[2] = pixel.b - qPixel.b;

      setPixelTo(x, y, qPixel, videoBufferBack);

      auto updatePixel = [&x, &y, &error, &w, &h, &k, &p, &px, &py, this](const Vec2 &vOffset, const float fErrorBias)
      {
        px = x + vOffset.x;
        py = y + vOffset.y;
        if (px < 0)
          px = 0;
        if (py < 0)
          py = 0;
        if (px > w)
          px = w;
        if (py > h)
          py = h;

        p = this->getPixelFrom(px, py, videoBufferBack);

        k[0] = p.r + int32_t(float(error[0]) * fErrorBias);
        k[1] = p.g + int32_t(float(error[1]) * fErrorBias);
        k[2] = p.b + int32_t(float(error[2]) * fErrorBias);

        p.r = std::clamp(k[0], 0, 255);
        p.g = std::clamp(k[1], 0, 255);
        p.b = std::clamp(k[2], 0, 255);
        setPixelTo(px, py, p, videoBufferBack);
      };

      updatePixel({1, 0}, step1);
      updatePixel({-1, 1}, step2);
      updatePixel({0, 1}, step3);
      updatePixel({1, 1}, step4);
    }
  }
}

void Engine::drawLine(int sx, int sy, int ex, int ey, Color color)
{
  float x{static_cast<float>(ex - sx)}, y{static_cast<float>(ey - sy)};
  const float max{std::max(std::fabs(x), std::fabs(y))};
  x /= max;
  y /= max;
  float lx = (float)sx;
  float ly = (float)sy;
  for (int i = 0; i < (int)max; i++)
  {
    setPixel((int)(lx), (int)(ly), color);
    lx += x;
    ly += y;
  }
}

// Implementation of ScanlineFillTexturedPart
void Engine::ScanlineFillTexturedPart(int y_start, int y_end,
                                    float x_edge1_current, const UV& uv_edge1_start_ref, float w_edge1_start_val,
                                    float x_edge2_current, const UV& uv_edge2_start_ref, float w_edge2_start_val,
                                    float dx_edge1_step, float du_edge1_step, float dv_edge1_step, float dw_edge1_step,
                                    float dx_edge2_step, float du_edge2_step, float dv_edge2_step, float dw_edge2_step,
                                    sf::Image &texture, const Color &base_color)
{
    float tex_ww = texture.getSize().x;
    float tex_hh = texture.getSize().y;

    // Create local copies for iterative updates of UVs and Ws along the edges
    UV uv_edge1_current = uv_edge1_start_ref;
    float w_edge1_current = w_edge1_start_val;
    UV uv_edge2_current = uv_edge2_start_ref;
    float w_edge2_current = w_edge2_start_val;

    for (int i = y_start; i <= y_end; i++)
    {
        int ax = static_cast<int>(x_edge1_current);
        int bx = static_cast<int>(x_edge2_current);

        // Perspective-correct UVs and W for start and end of scanline
        // These are already effectively calculated by the stepping from the main function
        // u_s, v_s, w_s correspond to edge1; u_e, v_e, w_e correspond to edge2
        float u_s_persp = uv_edge1_current.u / w_edge1_current;
        float v_s_persp = uv_edge1_current.v / w_edge1_current;
        float u_e_persp = uv_edge2_current.u / w_edge2_current;
        float v_e_persp = uv_edge2_current.v / w_edge2_current;

        // Local copies for swapping if ax > bx
        float u_s = uv_edge1_current.u;
        float v_s = uv_edge1_current.v;
        float w_s = w_edge1_current;
        float u_e = uv_edge2_current.u;
        float v_e = uv_edge2_current.v;
        float w_e = w_edge2_current;

        if (ax > bx)
        {
            std::swap(ax, bx);
            std::swap(u_s, u_e);
            std::swap(v_s, v_e);
            std::swap(w_s, w_e);
        }

        if (ax == bx) { // Avoid division by zero for tstep if scanline is a point
             x_edge1_current += dx_edge1_step;
             uv_edge1_current.u += du_edge1_step;
             uv_edge1_current.v += dv_edge1_step;
             w_edge1_current += dw_edge1_step;

             x_edge2_current += dx_edge2_step;
             uv_edge2_current.u += du_edge2_step;
             uv_edge2_current.v += dv_edge2_step;
             w_edge2_current += dw_edge2_step;
            continue;
        }

        float tstep = 1.0f / static_cast<float>(bx - ax);
        float t = 0.0f;

        for (int j = ax; j < bx; j++)
        {
            float w = (1.0f - t) * w_s + t * w_e;
            if (w == 0) { t += tstep; continue; } // Avoid division by zero

            // Perspective-correct interpolation for u and v across the scanline
            float u_interp = ((1.0f - t) * u_s / w_s + t * u_e / w_e) * w; 
            float v_interp = ((1.0f - t) * v_s / w_s + t * v_e / w_e) * w;
            v_interp = 1.0f - v_interp; // Flip v for texture coordinate system

            if (i >= 0 && i < height && j >= 0 && j < width) { // Bounds check for screen
                if (w > pDepthBuffer[i * width + j] || useSort)
                {
                    Color col;
                    int tex_x = static_cast<int>(u_interp * tex_ww);
                    int tex_y = static_cast<int>(v_interp * tex_hh);

                    // Texture bounds check
                    if (tex_x < 0) tex_x = 0; if (tex_x >= tex_ww) tex_x = tex_ww - 1;
                    if (tex_y < 0) tex_y = 0; if (tex_y >= tex_hh) tex_y = tex_hh - 1;

                    sf::Color c = texture.getPixel(tex_x, tex_y);

                    float cr = base_color.r / 255.0f;
                    float cg = base_color.g / 255.0f;
                    float cb = base_color.b / 255.0f;

                    col.r = static_cast<uint8_t>(c.r * cr);
                    col.g = static_cast<uint8_t>(c.g * cg);
                    col.b = static_cast<uint8_t>(c.b * cb);

                    float w_fog = std::clamp((w / 0.5f) * fogW, 0.0f, 1.0f);
                    col = mixRGB(col.r, col.g, col.b, fogColor.r, fogColor.g, fogColor.b, w_fog);

                    setPixel(j, i, col);

                    if (!useSort)
                        pDepthBuffer[i * width + j] = w;
                }
            }
            t += tstep;
        }
        // Step along the edges for the next scanline
        x_edge1_current += dx_edge1_step;
        uv_edge1_current.u += du_edge1_step;
        uv_edge1_current.v += dv_edge1_step;
        w_edge1_current += dw_edge1_step;

        x_edge2_current += dx_edge2_step;
        uv_edge2_current.u += du_edge2_step;
        uv_edge2_current.v += dv_edge2_step;
        w_edge2_current += dw_edge2_step;
    }
}

void Engine::texturedTriangle(Vec3 &t1_in, UV &uv1_in, float w1_in,
                              Vec3 &t2_in, UV &uv2_in, float w2_in,
                              Vec3 &t3_in, UV &uv3_in, float w3_in,
                              sf::Image &img, Color &color)
{
  Vec3 p1 = t1_in; Vec3 p2 = t2_in; Vec3 p3 = t3_in;
  UV tex1 = uv1_in; UV tex2 = uv2_in; UV tex3 = uv3_in;
  float w_val1 = w1_in; float w_val2 = w2_in; float w_val3 = w3_in;

  SortVerticesByY(p1, p2, p3, tex1, tex2, tex3, w_val1, w_val2, w_val3);

  int y1 = static_cast<int>(p1.y); int y2 = static_cast<int>(p2.y); int y3 = static_cast<int>(p3.y);

  // Deltas for the major triangle edges (p1-p2 and p1-p3)
  float dx12_step = 0, du12_step = 0, dv12_step = 0, dw12_step = 0; // Edge p1-p2
  float dx13_step = 0, du13_step = 0, dv13_step = 0, dw13_step = 0; // Edge p1-p3
  float dx23_step = 0, du23_step = 0, dv23_step = 0, dw23_step = 0; // Edge p2-p3

  if (y2 - y1 > 0) {
      float inv_dy12 = 1.0f / (p2.y - p1.y);
      dx12_step = (p2.x - p1.x) * inv_dy12;
      du12_step = (tex2.u - tex1.u) * inv_dy12;
      dv12_step = (tex2.v - tex1.v) * inv_dy12;
      dw12_step = (w_val2 - w_val1) * inv_dy12;
  }

  if (y3 - y1 > 0) {
      float inv_dy13 = 1.0f / (p3.y - p1.y);
      dx13_step = (p3.x - p1.x) * inv_dy13;
      du13_step = (tex3.u - tex1.u) * inv_dy13;
      dv13_step = (tex3.v - tex1.v) * inv_dy13;
      dw13_step = (w_val3 - w_val1) * inv_dy13;
  }

  if (y3 - y2 > 0) {
      float inv_dy23 = 1.0f / (p3.y - p2.y);
      dx23_step = (p3.x - p2.x) * inv_dy23;
      du23_step = (tex3.u - tex2.u) * inv_dy23;
      dv23_step = (tex3.v - tex2.v) * inv_dy23;
      dw23_step = (w_val3 - w_val2) * inv_dy23;
  }

  // Top part of the triangle (p1 to p2)
  if (y2 - y1 > 0) {
      ScanlineFillTexturedPart(y1, y2 -1, // Iterate up to y2-1 because y2 is the start of the next part
                               p1.x, tex1, w_val1,
                               p1.x, tex1, w_val1,
                               dx12_step, du12_step, dv12_step, dw12_step,
                               dx13_step, du13_step, dv13_step, dw13_step,
                               img, color);
  }

  // Bottom part of the triangle (p2 to p3)
  if (y3 - y2 > 0) {
      // Calculate the intersection point M on edge p1-p3 at height y2
      float My = static_cast<float>(y2);
      float Mx, Mu, Mv, Mw;

      if (y3 - y1 == 0) { // Avoid division by zero if p1 and p3 have same y (should not happen if p1,p2,p3 distinct and sorted)
          Mx = p1.x; // Or some other sensible default / error handling
          Mw = w_val1;
          Mu = tex1.u;
          Mv = tex1.v;
      } else {
          float t_intersect = (My - p1.y) / (p3.y - p1.y);
          Mx = p1.x + t_intersect * (p3.x - p1.x);
          Mw = w_val1 + t_intersect * (w_val3 - w_val1);
          if (Mw == 0) Mw = 1e-6f; // Avoid division by zero for UV calculation
          
          float u1_over_w1 = (w_val1 == 0) ? 0 : tex1.u / w_val1;
          float v1_over_w1 = (w_val1 == 0) ? 0 : tex1.v / w_val1;
          float u3_over_w3 = (w_val3 == 0) ? 0 : tex3.u / w_val3;
          float v3_over_w3 = (w_val3 == 0) ? 0 : tex3.v / w_val3;

          Mu = ((1.0f - t_intersect) * u1_over_w1 + t_intersect * u3_over_w3) * Mw;
          Mv = ((1.0f - t_intersect) * v1_over_w1 + t_intersect * v3_over_w3) * Mw;
      }
      UV texM = {Mu, Mv, Mw}; // Mw is also stored in texM.w for consistency if needed by ScanlineFill

      ScanlineFillTexturedPart(y2, y3,
                               p2.x, tex2, w_val2,                     // Start of edge 1 (p2)
                               Mx, texM, Mw,                           // Start of edge 2 (Point M on p1-p3)
                               dx23_step, du23_step, dv23_step, dw23_step, // Steps for edge 1 (p2-p3)
                               dx13_step, du13_step, dv13_step, dw13_step, // Steps for edge 2 (p1-p3, continued from M)
                               img, color);
  }

  stData.numOfTrianglesPerFrame++;
  stData.numOfTrianglesPerSecond++;
}

void Engine::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color)
{
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x3, y3, color);
  drawLine(x1, y1, x3, y3, color);
}

void Engine::fillTriangle(Vec3 &t1, Vec3 &t2, Vec3 &t3, Color color)
{
  Vec3 AUX;
  if (t1.y > t2.y)
  {
    AUX = t1;
    t1 = t2;
    t2 = AUX;
  }
  if (t1.y > t3.y)
  {
    AUX = t1;
    t1 = t3;
    t3 = AUX;
  }
  if (t2.y > t3.y)
  {
    AUX = t2;
    t2 = t3;
    t3 = AUX;
  }

  int p0x = t1.x;
  int p0y = t1.y;
  int p1x = t2.x;
  int p1y = t2.y;
  int p2x = t3.x;
  int p2y = t3.y;

  int x1 = 0;
  int x2 = 0;
  double slope1 = 0;
  double slope2 = 0;
  int y = 0;
  int x = 0;

  double sx;

  if (p0y < p1y)
  {
    slope1 = ((double)p1x - p0x) / (p1y - p0y);
    slope2 = ((double)p2x - p0x) / (p2y - p0y);
    for (int i = 0; i < p1y - p0y; i++)
    {
      x1 = p0x + i * slope1;
      x2 = p0x + i * slope2;
      y = p0y + i;

      if (x1 > x2)
      {
        int aux = x1;
        x1 = x2;
        x2 = aux;
      }

      if (x2 > x1)
      {
        for (int j = 0; j <= x2 - x1; j++)
        {
          x = x1 + j;
          setPixel(x, y, color);
        }
      }
    }
  }

  if (p1y < p2y)
  {
    slope1 = ((double)p2x - p1x) / (p2y - p1y);
    slope2 = ((double)p2x - p0x) / (p2y - p0y);
    sx = p2x - (p2y - p1y) * slope2;
    for (int i = 0; i < p2y - p1y; i++)
    {
      x1 = p1x + i * slope1;
      x2 = sx + i * slope2;
      y = p1y + i;

      if (x1 > x2)
      {
        int aux = x1;
        x1 = x2;
        x2 = aux;
      }

      if (x2 > x1)
      {
        for (int x = x1 + 1; x <= x2; x++)
        {
          setPixel(x, y, color);
        }
      }
    }
  }

  stData.numOfTrianglesPerFrame++;
  stData.numOfTrianglesPerSecond++;
}

void Engine::renderTriangle(Triangle &triangle, int textureID)
{
  if (enableVertexSnapping)
  {
    for (int i = 0; i < 3; ++i)
    {
      triangle.p[i].x = floorf(triangle.p[i].x);
      triangle.p[i].y = floorf(triangle.p[i].y);
      // triangle.p[i].z = floorf(triangle.p[i].z); // Optional: snapping Z can affect depth precision
    }
  }

  if (rMode == RenderMode::textured)
  {
    if (textureID >= 0 && textureID < textureImage.size()) {
      texturedTriangle(triangle.p[0], triangle.t[0], triangle.t[0].w,
                      triangle.p[1], triangle.t[1], triangle.t[1].w,
                      triangle.p[2], triangle.t[2], triangle.t[2].w,
                      textureImage[textureID], triangle.color);
    } else {
      fillTriangle(triangle.p[0], triangle.p[1], triangle.p[2], triangle.color);
    }
  }
  else if (rMode == RenderMode::filled)
  {
    fillTriangle(triangle.p[0], triangle.p[1], triangle.p[2], triangle.color);
  }
  else if (rMode == RenderMode::wireframe)
  {
    drawTriangle(triangle.p[0].x, triangle.p[0].y,
                triangle.p[1].x, triangle.p[1].y,
                triangle.p[2].x, triangle.p[2].y, triangle.color);
  }
}

float Engine::getClock()
{
  return deltaTime;
}

bool Engine::isOpen()
{
  return window.isOpen();
}

void Engine::checkEvents()
{
  sf::Event event{};
  while (window.pollEvent(event))
  {
    if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
      window.close();
  }
}

void Engine::copyVideoBuffer(uint8_t *buffer)
{
  Color c;
  uint32_t offset;

  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      offset = (y * width + x) * 3;

      c.r = buffer[offset];
      c.g = buffer[offset + 1];
      c.b = buffer[offset + 2];

      screenBuffer.setPixel(x, y, sf::Color{c.r, c.g, c.b, 255});
    }
  }
}

void Engine::renderDebugData()
{
  if (stData.fps_graph.size() > 1)
  {
    float max_fps = 0;
    float min_fps = 10000;

    for (int i = 0; i < stData.fps_graph.size() - 1; i++)
    {
      if (stData.fps_graph[i] > max_fps)
        max_fps = stData.fps_graph[i];

      if (stData.fps_graph[i] < min_fps)
        min_fps = stData.fps_graph[i];
    }

    float normalized_fps_y = 0;
    float normalized_fps_y_old = 0;

    Color color_fps = {255, 255, 255};
    Color color_borders = {128, 128, 128};

    for (int i = 1; i < stData.fps_graph.size(); i++)
    {
      normalized_fps_y_old = normalized_fps_y;

      normalized_fps_y = 25 - (stData.fps_graph[i] / (max_fps + min_fps)) * 25;
      normalized_fps_y = clamp2(normalized_fps_y, 0.0f, 25.0f);

      if (i == 1)
        normalized_fps_y_old = normalized_fps_y;

      drawLine(i - 1, (int)normalized_fps_y_old, i, (int)normalized_fps_y, color_fps);
    }

    drawLine(0, 24, stData.graphSize, 24, color_borders);
    drawLine(stData.graphSize, 0, stData.graphSize, 24, color_borders);
  }
}

void Engine::render(int debugMode)
{
  rasterize();

  if (debugMode) {
    renderDebugData();
  }

  if (useDither) {
    Dither_FloydSteinberg();
    copyVideoBuffer(videoBufferBack);
  } else {
    copyVideoBuffer(videoBuffer);
  }

  screenTexture.update(screenBuffer);
  window.draw(sprite);
  sprite.setPosition(0, 0);
  window.display();

  clear();

  deltaTime = clock.restart().asSeconds();

  stData.numOfTrianglesPerFrame = vecTrianglesToRaster.size();
  stData.totalTrianglesRendered += stData.numOfTrianglesPerFrame;
  
  if (debugMode)
  {
    stData.graphSize = 48;
    dt += deltaTime;
    fpsCounter++;
    if (fpsCounter >= fpsCounterMax && dt >= 1.0f)
    {
      char titleText[32] = {0};
      float fps = 1.0 / (dt / (float)fpsCounter);
      sprintf(titleText, "avg. FPS: %0.2f", fps);

      stData.fps_graph.push_back(fps);
      if (stData.fps_graph.size() > stData.graphSize)
        stData.fps_graph.erase(stData.fps_graph.begin() + 0);

      printf("%s\n", titleText);
      window.setTitle(titleText);

      fpsCounter = dt = 0;
      stData.numOfTrianglesPerSecond = 0;
    }

    if (stData.triangle_count_graph.size() >= stData.graphSize) {
      stData.triangle_count_graph.erase(stData.triangle_count_graph.begin());
    }
    stData.triangle_count_graph.push_back(stData.numOfTrianglesPerFrame);
  }

  stData.numOfTrianglesPerFrame = 0;
}

bool Engine::checkIfAABBisOnScreen(AABB &aabb, mat4x4 &matWorld, mat4x4 &matView)
{
  Triangle triProjected, triTransformed, triViewed;

  int nClippedTriangles = 0;
  Triangle clipped[2];
  Vec3 av = {0, 0, 1};
  Vec3 bc = {0, 0, 1};

  std::vector<Vec3> verts;
  verts.push_back({aabb.min.x, aabb.min.y, aabb.min.z});
  verts.push_back({aabb.max.x, aabb.min.y, aabb.min.z});
  verts.push_back({aabb.min.x, aabb.min.y, aabb.max.z});
  verts.push_back({aabb.max.x, aabb.min.y, aabb.max.z});

  verts.push_back({aabb.min.x, aabb.max.y, aabb.min.z});
  verts.push_back({aabb.max.x, aabb.max.y, aabb.min.z});
  verts.push_back({aabb.min.x, aabb.max.y, aabb.max.z});
  verts.push_back({aabb.max.x, aabb.max.y, aabb.max.z});

  int k = 0;

  for (int a = 0; a < 8; a++)
  {
    if (a == 0)
    {
      triViewed.p[0] = verts[0];
      triViewed.p[1] = verts[1];
      triViewed.p[2] = verts[2];
    }
    else if (a == 1)
    {
      triViewed.p[0] = verts[1];
      triViewed.p[1] = verts[2];
      triViewed.p[2] = verts[3];
    }
    else if (a == 2)
    {
      triViewed.p[0] = verts[1];
      triViewed.p[1] = verts[0];
      triViewed.p[2] = verts[5];
    }
    else if (a == 3)
    {
      triViewed.p[0] = verts[0];
      triViewed.p[1] = verts[5];
      triViewed.p[2] = verts[4];
    }
    else if (a == 4)
    {
      triViewed.p[0] = verts[2];
      triViewed.p[1] = verts[7];
      triViewed.p[2] = verts[6];
    }
    else if (a == 5)
    {
      triViewed.p[0] = verts[3];
      triViewed.p[1] = verts[7];
      triViewed.p[2] = verts[2];
    }
    else if (a == 6)
    {
      triViewed.p[0] = verts[5];
      triViewed.p[1] = verts[7];
      triViewed.p[2] = verts[6];
    }
    else if (a == 7)
    {
      triViewed.p[0] = verts[4];
      triViewed.p[1] = verts[5];
      triViewed.p[2] = verts[6];
    }

    triTransformed.p[0] = Matrix_MultiplyVector(matWorld, triViewed.p[0]);
    triTransformed.p[1] = Matrix_MultiplyVector(matWorld, triViewed.p[1]);
    triTransformed.p[2] = Matrix_MultiplyVector(matWorld, triViewed.p[2]);

    triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
    triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
    triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);

    nClippedTriangles = Triangle_CLipAgainstPlane(av, bc, triViewed, clipped[0], clipped[1]);

    if (nClippedTriangles == 0)
      continue;

    k++;
  }

  if (k > 0)
    return true;

  return false;
}
void Engine::calculateTriangles(Vec3 &camera, Vec3 &vTarget, Vec3 &vUp)
{
  vecTrianglesToRaster.clear();

  mat4x4 matCamera = Matrix_PointAt(camera, vTarget, vUp);
  mat4x4 matView = Matrix_QuickInverse(matCamera);
  mat4x4 matTrans;

  int numOfRenderedComponents = 0;

  if (components.components.empty()) {
    return;
  }

  for (int i = 0; i < components.components.size(); i++)
  {
    Component &component = components.components[i];
    if (component.meshes.meshes.empty()) {
      continue;
    }

    matTrans = Matrix_MakeTranslation(component.transform.pos);
    component.transform.setupMatrix(matTrans);

    for (auto &mesh : component.meshes.meshes)
    {
      if (mesh.tris.empty()) {
        continue;
      }

      bool r = checkIfAABBisOnScreen(mesh.aabb, component.transform.matWorld, matView);
      if (r == false) {
        continue;
      }

      numOfRenderedComponents++;

      for (auto &tri : mesh.tris)
      {
        Triangle triProjected, triTransformed, triViewed;

        triTransformed.p[0] = Matrix_MultiplyVector(component.transform.matWorld, tri.p[0]);
        triTransformed.p[1] = Matrix_MultiplyVector(component.transform.matWorld, tri.p[1]);
        triTransformed.p[2] = Matrix_MultiplyVector(component.transform.matWorld, tri.p[2]);
        
        triTransformed.t[0] = tri.t[0];
        triTransformed.t[1] = tri.t[1];
        triTransformed.t[2] = tri.t[2];
        triTransformed.textureID = mesh.textureID;
        triTransformed.color = tri.color;

        Vec3 normal, line1, line2;
        line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
        line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);
        normal = Vector_CrossProduct(line1, line2);
        normal = Vector_Normalise(normal);

        Vec3 vCameraRay = Vector_Sub(triTransformed.p[0], camera);

        if (Vector_DotProduct(normal, vCameraRay) < 0.0f)
        {
          Vec3 light_direction = {1, -1, -1};
          light_direction = Vector_Normalise(light_direction);
          float dp = max(0.1f, Vector_DotProduct(light_direction, normal));

          triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
          triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
          triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);
          triViewed.color = tri.color;
          triViewed.textureID = tri.textureID;
          triViewed.t[0] = triTransformed.t[0];
          triViewed.t[1] = triTransformed.t[1];
          triViewed.t[2] = triTransformed.t[2];

          int nClippedTriangles = 0;
          Triangle clipped[2];
          Vec3 av = {0, 0, 1};
          Vec3 bc = {0, 0, 1};
          nClippedTriangles = Triangle_CLipAgainstPlane(av, bc, triViewed, clipped[0], clipped[1]);

          for (int n = 0; n < nClippedTriangles; n++)
          {
            triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
            triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
            triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);
            triProjected.color = clipped[n].color;
            triProjected.textureID = clipped[n].textureID;
            triProjected.t[0] = clipped[n].t[0];
            triProjected.t[1] = clipped[n].t[1];
            triProjected.t[2] = clipped[n].t[2];

            triProjected.t[0].w = 1.0f / triProjected.p[0].w;
            triProjected.t[1].w = 1.0f / triProjected.p[1].w;
            triProjected.t[2].w = 1.0f / triProjected.p[2].w;

            float w = (triProjected.t[0].w + triProjected.t[1].w + triProjected.t[2].w) / 3.0f;
            if (w < clipEnd) {
              continue;
            }

            triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
            triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
            triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

            Vec3 vOffsetView = {1, 1, 0};
            triProjected.p[0] = Vector_Add(triProjected.p[0], vOffsetView);
            triProjected.p[1] = Vector_Add(triProjected.p[1], vOffsetView);
            triProjected.p[2] = Vector_Add(triProjected.p[2], vOffsetView);

            for (int i = 0; i < 3; i++)
            {
              triProjected.p[i].x *= 0.5f * (float)width;
              triProjected.p[i].y *= 0.5f * (float)height;
            }

            uint8_t litR = clamp2((dp * triProjected.color.r), 0.0f, 255.0f);
            uint8_t litG = clamp2((dp * triProjected.color.g), 0.0f, 255.0f);
            uint8_t litB = clamp2((dp * triProjected.color.b), 0.0f, 255.0f);
            triProjected.color = {litR, litG, litB};

            vecTrianglesToRaster.push_back(triProjected);
          }
        }
      }
    }
  }

  if (useSort)
  {
    sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](Triangle &t1, Triangle &t2)
         {
           float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
           float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
           return z1 > z2; });
  }
}

void Engine::rasterize()
{
  if (vecTrianglesToRaster.empty()) {
    return;
  }

  for (auto &triToRaster : vecTrianglesToRaster)
  {
    Triangle clipped[2];
    std::list<Triangle> listTriangles;
    listTriangles.push_back(triToRaster);
    int nNewTriangles = 1;

    for (int p = 0; p < 4; p++)
    {
      int nTrisToAdd = 0;
      while (nNewTriangles > 0)
      {
        Triangle test = listTriangles.front();
        listTriangles.pop_front();
        nNewTriangles--;

        Vec3 av;
        Vec3 bv;

        switch (p)
        {
        case 0:
          av = {0, 0, 0};
          bv = {0, 1, 0};
          nTrisToAdd = Triangle_CLipAgainstPlane(av, bv, test, clipped[0], clipped[1]);
          break;
        case 1:
          av = {0, (float)height - 1, 0};
          bv = {0, -1, 0};
          nTrisToAdd = Triangle_CLipAgainstPlane(av, bv, test, clipped[0], clipped[1]);
          break;
        case 2:
          av = {0, 0, 0};
          bv = {1, 0, 0};
          nTrisToAdd = Triangle_CLipAgainstPlane(av, bv, test, clipped[0], clipped[1]);
          break;
        case 3:
          av = {(float)width - 1, 0, 0};
          bv = {-1, 0, 0};
          nTrisToAdd = Triangle_CLipAgainstPlane(av, bv, test, clipped[0], clipped[1]);
          break;
        }

        for (int w = 0; w < nTrisToAdd; w++)
        {
          listTriangles.push_back(clipped[w]);
        }
      }
      nNewTriangles = listTriangles.size();
    }

    for (auto &t : listTriangles)
    {
      try {
        renderTriangle(t, t.textureID);
      } catch (const std::exception& e) {
      }
    }
  }
}

void Engine::setSort(bool b)
{
  useSort = b;
}

void Engine::setDither(bool v)
{
  useDither = v;
}

void Engine::setVertexSnapping(bool enabled)
{
  enableVertexSnapping = enabled;
}

void Engine::setFogColor(const Color& new_color)
{
  this->fogColor = new_color;
  // Re-initialize clearScreenPtr with the new fog color
  if (clearScreenPtr) { // Ensure it was allocated
    for (int y = 0; y < height; y++)
    {
      for (int x = 0; x < width; x++)
      {
        int i = (y * width + x) * 3;
        clearScreenPtr[i] = this->fogColor.r;
        clearScreenPtr[i + 1] = this->fogColor.g;
        clearScreenPtr[i + 2] = this->fogColor.b;
      }
    }
  }
}
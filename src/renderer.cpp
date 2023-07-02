#include <renderer.hpp>

Renderer::Renderer(int targetFPS, float scale, const char *title)
{
  width = 256;
  height = 224;

  fpsCounter = 0;
  fpsCounterMax = 100;
  fpsLimit = targetFPS;
  this->scale = scale;

  window.create(sf::VideoMode(width * scale, height * scale), title, sf::Style::Default);

  window.setFramerateLimit(fpsLimit);

  screenBuffer.create(width, height, sf::Color::Black);
  // screenBuffer2.create(width, height, sf::Color::Black);

  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      screenBuffer.setPixel(x, y, sf::Color::Black);
      // screenBuffer2.setPixel(x, y, sf::Color::Black);
    }
  }

  videoBuffer = new uint8_t[width * height * 3];
  videoBufferBack = new uint8_t[width * height * 3];

  screenTexture.create(width, height);
  screenTexture.update(screenBuffer);
  sprite.setTexture(screenTexture);
  sprite.setScale(scale, scale);

  dt = getClock();

  px0 = const_cast<sf::Uint8 *>(screenBuffer.getPixelsPtr());
  // px1 = const_cast<sf::Uint8 *>(screenBuffer2.getPixelsPtr());

  clearScreenPtr = new uint8_t[width * height * 3];

  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      int i = (y * width + x) * 3;
      clearScreenPtr[i] = 0x10;
      clearScreenPtr[i + 1] = 0x10;
      clearScreenPtr[i + 2] = 0x10;
    }
  }

  // components.createFromFile("spyro/spyro.obj", {1, 0, 5});

  sf::Image tempImage;
  if (!tempImage.loadFromFile("spyro/Glimmer_ObjectTextures.png"))
  {
    printf("test.png not found.\n");
  }
  textureImage.create(tempImage.getSize().x, tempImage.getSize().y);

  for (int y = 0; y < textureImage.getSize().y; y++)
  {
    for (int x = 0; x < textureImage.getSize().x; x++)
    {
      sf::Color col = tempImage.getPixel(x, y);
      Color c = {col.r, col.g, col.b};
      c = quantise(c);

      textureImage.setPixel(x, y, {c.r, c.g, c.b});
    }
  }

  // projection matrix
  /*
    AS, 0,  0,  0
    0,  S,  0,  0
    0,  0,  f1, f2
    0,  0,  1,  0
  */
  float fNear = 0.1f;
  float fFar = 1000.0f;
  float fFov = 75.0f;
  float fAspectRatio = (float)height / (float)width;

  matProj = Matrix_MakeProjection(fFov, fAspectRatio, fNear, fFar);
  rMode = RenderMode::textured;
}

Renderer::~Renderer()
{
  if (videoBuffer != nullptr)
    delete videoBuffer;
  videoBuffer = nullptr;

  if (videoBufferBack != nullptr)
    delete videoBufferBack;
  videoBufferBack = nullptr;
}

void Renderer::clear()
{
  memcpy(videoBuffer, clearScreenPtr, width * height * 3);

}

inline void Renderer::setPixel(int x, int y, Color &color)
{

  uint32_t offset = (y * width + x) * 3;
  videoBuffer[offset] = color.r;
  videoBuffer[offset + 1] = color.g;
  videoBuffer[offset + 2] = color.b;
}

inline void Renderer::setPixelTo(int x, int y, Color &color, uint8_t *buffer)
{

  uint32_t offset = (y * width + x) * 3;

  buffer[offset] = color.r;
  buffer[offset + 1] = color.g;
  buffer[offset + 2] = color.b;
}

inline Color Renderer::getPixelFrom(int x, int y, uint8_t *buffer)
{
  uint32_t offset = (y * width + x) * 3;
  return {buffer[offset], buffer[offset + 1], buffer[offset + 2]};
}

void Renderer::Dither_FloydSteinberg()
{

  // pDest->copy(*pSource, 0, 0);
  int32_t w = width;
  int32_t h = height;

  memcpy(videoBufferBack, videoBuffer, w * h * 3);

  constexpr float step = 16.0;
  constexpr float step1 = 7.0f / step;
  constexpr float step2 = 3.0f / step;
  constexpr float step3 = 5.0f / step;
  constexpr float step4 = 1.0f / step;

  Color pixel;
  Color qPixel; // quantise(pixel);

  int32_t error[3];

  int32_t k[3];

  Color p;
  int px, py;

  for (int y = 0; y < h; y++)
  {
    for (int x = 0; x < w; x++)
    {

      pixel = getPixelFrom(x, y, videoBufferBack); // pDest->getPixel(x, y);

      qPixel = quantise(pixel);

      error[0] = pixel.r - qPixel.r;
      error[1] = pixel.g - qPixel.g;
      error[2] = pixel.b - qPixel.b;

      // pDest->setPixel(x, y, {qPixel.r, qPixel.g, qPixel.b, 255});
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

        // sf::Color p = pDest->getPixel(px, py);
        p = this->getPixelFrom(px, py, videoBufferBack);
        // if (p.r == 0 && p.g == 0 && p.b == 0)
        // return;

        // if (p.a == 255)
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

void Renderer::drawLine(int sx, int sy, int ex, int ey, Color color)
{

  float x{ex - sx}, y{ey - sy};
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

void Renderer::texturedTriangle(Vec3 &t1, UV &uv1,
                                Vec3 &t2, UV &uv2,
                                Vec3 &t3, UV &uv3,
                                sf::Image &img, Color &color)
{

  int x1 = t1.x;
  int x2 = t2.x;
  int x3 = t3.x;

  int y1 = t1.y;
  int y2 = t2.y;
  int y3 = t3.y;

  float u1 = uv1.u;
  float u2 = uv2.u;
  float u3 = uv3.u;

  float v1 = uv1.v;
  float v2 = uv2.v;
  float v3 = uv3.v;

  if (y2 < y1)
  {
    std::swap(y1, y2);
    std::swap(x1, x2);
    std::swap(u1, u2);
    std::swap(v1, v2);
  }

  if (y3 < y1)
  {
    std::swap(y1, y3);
    std::swap(x1, x3);
    std::swap(u1, u3);
    std::swap(v1, v3);
  }

  if (y3 < y2)
  {
    std::swap(y2, y3);
    std::swap(x2, x3);
    std::swap(u2, u3);
    std::swap(v2, v3);
  }

  int dy1 = y2 - y1;
  int dx1 = x2 - x1;
  float dv1 = v2 - v1;
  float du1 = u2 - u1;

  int dy2 = y3 - y1;
  int dx2 = x3 - x1;
  float dv2 = v3 - v1;
  float du2 = u3 - u1;

  float tex_u, tex_v, tex_w, tex_h;

  tex_w = img.getSize().x;
  tex_h = img.getSize().y;

  float dax_step = 0, dbx_step = 0,
        du1_step = 0, dv1_step = 0,
        du2_step = 0, dv2_step = 0;

  if (dy1)
    dax_step = dx1 / (float)abs(dy1);
  if (dy2)
    dbx_step = dx2 / (float)abs(dy2);

  if (dy1)
    du1_step = du1 / (float)abs(dy1);
  if (dy1)
    dv1_step = dv1 / (float)abs(dy1);

  if (dy2)
    du2_step = du2 / (float)abs(dy2);
  if (dy2)
    dv2_step = dv2 / (float)abs(dy2);

  if (dy1)
  {
    for (int i = y1; i <= y2; i++)
    {
      int ax = x1 + (float)(i - y1) * dax_step;
      int bx = x1 + (float)(i - y1) * dbx_step;

      float tex_su = u1 + (float)(i - y1) * du1_step;
      float tex_sv = v1 + (float)(i - y1) * dv1_step;

      float tex_eu = u1 + (float)(i - y1) * du2_step;
      float tex_ev = v1 + (float)(i - y1) * dv2_step;

      if (ax > bx)
      {
        std::swap(ax, bx);
        std::swap(tex_su, tex_eu);
        std::swap(tex_sv, tex_ev);
      }

      tex_u = tex_su;
      tex_v = tex_sv;

      float tstep = 1.0f / ((float)(bx - ax));
      float t = 0.0f;

      for (int j = ax; j < bx; j++)
      {
        tex_u = (1.0f - t) * tex_su + t * tex_eu;
        tex_v = (1.0f - t) * tex_sv + t * tex_ev;

        tex_v = 1.0f - tex_v;

        Color col;
        sf::Color c = img.getPixel((int)(tex_u * tex_w), (int)(tex_v * tex_h));

        float cr = color.r / 255.0;
        float cg = color.g / 255.0;
        float cb = color.b / 255.0;

        col.r = (uint8_t)(c.r * cr);
        col.g = (uint8_t)(c.g * cg);
        col.b = (uint8_t)(c.b * cb);

        setPixel(j, i, col);

        t += tstep;
      }
    }
  }

  dy1 = y3 - y2;
  dx1 = x3 - x2;
  dv1 = v3 - v2;
  du1 = u3 - u2;

  if (dy1)
    dax_step = dx1 / (float)abs(dy1);
  if (dy2)
    dbx_step = dx2 / (float)abs(dy2);

  du1_step = 0, dv1_step = 0;
  if (dy1)
    du1_step = du1 / (float)abs(dy1);
  if (dy1)
    dv1_step = dv1 / (float)abs(dy1);

  if (dy1)
  {
    for (int i = y2; i <= y3; i++)
    {
      int ax = x2 + (float)(i - y2) * dax_step;
      int bx = x1 + (float)(i - y1) * dbx_step;

      float tex_su = u2 + (float)(i - y2) * du1_step;
      float tex_sv = v2 + (float)(i - y2) * dv1_step;

      float tex_eu = u1 + (float)(i - y1) * du2_step;
      float tex_ev = v1 + (float)(i - y1) * dv2_step;

      if (ax > bx)
      {
        std::swap(ax, bx);
        std::swap(tex_su, tex_eu);
        std::swap(tex_sv, tex_ev);
      }

      tex_u = tex_su;
      tex_v = tex_sv;

      float tstep = 1.0f / ((float)(bx - ax));
      float t = 0.0f;

      for (int j = ax; j < bx; j++)
      {
        tex_u = (1.0f - t) * tex_su + t * tex_eu;
        tex_v = (1.0f - t) * tex_sv + t * tex_ev;
        tex_v = 1.0f - tex_v;

        Color col;
        sf::Color c = img.getPixel((int)(tex_u * tex_w), (int)(tex_v * tex_h));

        float cr = color.r / 255.0;
        float cg = color.g / 255.0;
        float cb = color.b / 255.0;

        col.r = (uint8_t)(c.r * cr);
        col.g = (uint8_t)(c.g * cg);
        col.b = (uint8_t)(c.b * cb);

        setPixel(j, i, col);

        t += tstep;
      }
    }
  }

  stData.numOfTriangles++;
}

void Renderer::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, Color color)
{

  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x3, y3, color);
  drawLine(x1, y1, x3, y3, color);
}

void Renderer::fillTriangle(Vec3 &t1, Vec3 &t2, Vec3 &t3, Color color)
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

  // top part
  if (p0y < p1y)
  {
    slope1 = ((double)p1x - p0x) / (p1y - p0y);
    slope2 = ((double)p2x - p0x) / (p2y - p0y);
    // each horizontal line
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

  // bottom part
  if (p1y < p2y)
  {
    slope1 = ((double)p2x - p1x) / (p2y - p1y);
    slope2 = ((double)p2x - p0x) / (p2y - p0y);
    sx = p2x - (p2y - p1y) * slope2;
    // each horizontal line
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

  stData.numOfTriangles++;
}

void Renderer::renderTriangle(Triangle &triangle)
{
  switch (rMode)
  {
  case RenderMode::textured:
    texturedTriangle(triangle.p[0], triangle.t[0],
                     triangle.p[1], triangle.t[1],
                     triangle.p[2], triangle.t[2],
                     textureImage, triangle.color);
    break;
  case RenderMode::filled:
    fillTriangle(triangle.p[0], triangle.p[1], triangle.p[2], triangle.color);
    break;
  case RenderMode::wireframe:
    drawTriangle(triangle.p[0].x, triangle.p[0].y,
                 triangle.p[1].x, triangle.p[1].y,
                 triangle.p[2].x, triangle.p[2].y, {0, 0, 0});
    break;
  default:
    break;
  };
}

float Renderer::getClock()
{
  return deltaTime;
}

bool Renderer::isOpen()
{
  return window.isOpen();
}

void Renderer::checkEvents()
{
  sf::Event event{};
  while (window.pollEvent(event))
  {
    if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
      window.close();
  }
}

void Renderer::copyVideoBuffer(uint8_t *buffer)
{
  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      uint32_t offset = (y * width + x) * 3;

      Color c;
      c.r = buffer[offset];
      c.g = buffer[offset + 1];
      c.b = buffer[offset + 2];

      screenBuffer.setPixel(x, y, sf::Color{c.r, c.g, c.b, 255});
    }
  }
}

void Renderer::render(int debugMode)
{
  window.clear(sf::Color(0, 80, 128));

  bool dither = false;
  if (dither)
  {
    Dither_FloydSteinberg();
    copyVideoBuffer(videoBufferBack);
  }
  else
  {
    copyVideoBuffer(videoBuffer);
  }

  screenTexture.update(screenBuffer);
  window.draw(sprite);
  sprite.setPosition(0, 0);
  window.display();

  clear();

  deltaTime = clock.restart().asSeconds();

  if (debugMode)
  {
    dt += deltaTime;
    fpsCounter++;
    if (fpsCounter >= fpsCounterMax)
    {
      char titleText[32] = {0};
      sprintf(titleText, "avg. FPS: %0.2f", 1.0 / (dt / (float)fpsCounter));

      printf("%s\n", titleText);
      printf("Rendered triangles: %u\n", stData.numOfTriangles);
      window.setTitle(titleText);

      fpsCounter = dt = 0;
    }
  }

  /*
  Color col;
  for (int y = 0; y < 224; y++)
  {
    for (int x = 0; x < 255; x++)
    {
      uint8_t c = (uint8_t)x;
      col.r = c;
      col.g = c;
      col.b = c;
      setPixel(x, y, col);
    }
  }
  */

  stData.numOfTriangles = 0;
}

void Renderer::calculateTriangles(Vec3 &camera, Vec3 &vTarget, Vec3 &vUp)
{
  vecTrianglesToRaster.clear();

  mat4x4 matCamera = Matrix_PointAt(camera, vTarget, vUp);
  mat4x4 matView = Matrix_QuickInverse(matCamera);
  mat4x4 matTrans;

  for (int i = 0; i < components.components.size(); i++)
  {
    Component &component = components.components[i];

    matTrans = Matrix_MakeTranslation(component.transform.pos);

    component.transform.setupMatrix(matTrans);

    for (auto mesh : component.meshes.meshes)
    {

      for (auto tri : mesh.tris)
      {
        Triangle triProjected, triTransformed, triViewed;

        triTransformed.p[0] = Matrix_MultiplyVector(component.transform.matWorld, tri.p[0]);
        triTransformed.p[1] = Matrix_MultiplyVector(component.transform.matWorld, tri.p[1]);
        triTransformed.p[2] = Matrix_MultiplyVector(component.transform.matWorld, tri.p[2]);
        triTransformed.t[0] = tri.t[0];
        triTransformed.t[1] = tri.t[1];
        triTransformed.t[2] = tri.t[2];

        // calculate triangle normals
        Vec3 normal, line1, line2;

        line1 = Vector_Sub(triTransformed.p[1], triTransformed.p[0]);
        line2 = Vector_Sub(triTransformed.p[2], triTransformed.p[0]);

        normal = Vector_CrossProduct(line1, line2);
        normal = Vector_Normalise(normal);

        Vec3 vCameraRay = Vector_Sub(triTransformed.p[0], camera);

        if (Vector_DotProduct(normal, vCameraRay) < 0.0f)
        {

          // illumination
          Vec3 light_direction = {1, -1, -1};
          light_direction = Vector_Normalise(light_direction);

          float dp = max(0.1, Vector_DotProduct(light_direction, normal));

          // convert world space to view space
          triViewed.p[0] = Matrix_MultiplyVector(matView, triTransformed.p[0]);
          triViewed.p[1] = Matrix_MultiplyVector(matView, triTransformed.p[1]);
          triViewed.p[2] = Matrix_MultiplyVector(matView, triTransformed.p[2]);
          triViewed.color = tri.color;
          triViewed.t[0] = triTransformed.t[0];
          triViewed.t[1] = triTransformed.t[1];
          triViewed.t[2] = triTransformed.t[2];

          // clip viewed triangle against near plane, this could form two
          // additional triangles.
          int nClippedTriangles = 0;
          Triangle clipped[2];
          Vec3 av = {0, 0, 1};
          Vec3 bc = {0, 0, 1};
          nClippedTriangles = Triangle_CLipAgainstPlane(av, bc, triViewed, clipped[0], clipped[1]);

          for (int n = 0; n < nClippedTriangles; n++)
          {

            // project triangles to 2d from 3d
            triProjected.p[0] = Matrix_MultiplyVector(matProj, clipped[n].p[0]);
            triProjected.p[1] = Matrix_MultiplyVector(matProj, clipped[n].p[1]);
            triProjected.p[2] = Matrix_MultiplyVector(matProj, clipped[n].p[2]);
            triProjected.color = clipped[n].color;
            triProjected.t[0] = clipped[n].t[0];
            triProjected.t[1] = clipped[n].t[1];
            triProjected.t[2] = clipped[n].t[2];

            triProjected.p[0] = Vector_Div(triProjected.p[0], triProjected.p[0].w);
            triProjected.p[1] = Vector_Div(triProjected.p[1], triProjected.p[1].w);
            triProjected.p[2] = Vector_Div(triProjected.p[2], triProjected.p[2].w);

            // offset verts into visible normalised space
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

void Renderer::rasterize()
{
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
      // rasterize
      renderTriangle(t);
    }
  }
}

void Renderer::renderAll()
{
  rasterize();
  render(1);
}

void Renderer::setSort(bool b)
{
  useSort = b;
}
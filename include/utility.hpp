#ifndef __UTILITY_H__
#define __UTILITY_H__

#include <cmath>
#include <fstream>
#include <strstream>
#include <vector>

struct Color
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

struct Vec2
{
  float x, y;
};

struct UV
{
  float u = 0;
  float v = 0;
};

struct Vec3
{
  float x = 0;
  float y = 0;
  float z = 0;
  float w = 1;
};

struct Triangle
{
  Vec3 p[3];
  UV t[3];
  Color color;
};

struct mat4x4
{
  float m[4][4] = {0};
};

/*
  - create function to normalize data
  - no of triangles to graph
*/
struct StatisticData
{
  int graphSize;
  uint32_t numOfTriangles;
  std::vector<float> fps_graph;
};

Color quantise(Color &color);

// void MultiplyMatrixVector(Vec3 &i, Vec3 &o, mat4x4 &m);
mat4x4 Matrix_MakeIdentity();
mat4x4 Matrix_MakeRotationX(float fAngleRad);
mat4x4 Matrix_MakeRotationY(float fAngleRad);
mat4x4 Matrix_MakeRotationZ(float fAngleRad);
mat4x4 Matrix_MakeTranslation(float x, float y, float z);
mat4x4 Matrix_MakeTranslation(Vec3 pos);
mat4x4 Matrix_MakeProjection(float fDovDegrees, float fAspectRatio, float fNear, float fFar);
mat4x4 Matrix_PointAt(Vec3 &pos, Vec3 &target, Vec3 &up);
mat4x4 Matrix_QuickInverse(mat4x4 &m);
mat4x4 Matrix_MultiplyMatrix(mat4x4 &m1, mat4x4 &m2);
Vec3 Matrix_MultiplyVector(mat4x4 &m, Vec3 &i);
Vec3 Vector_IntersectPlane(Vec3 &plane_p, Vec3 &plane_n, Vec3 &lineStart, Vec3 &lineEnd, float &t);

int Triangle_CLipAgainstPlane(Vec3 &plane_p, Vec3 &plane_n, Triangle &in_tri, Triangle &out_tri1, Triangle &out_tri2);

Vec3 Vector_Add(Vec3 &v1, Vec3 &v2);
Vec3 Vector_Sub(Vec3 &v1, Vec3 &v2);
Vec3 Vector_Mul(Vec3 &v1, float k);
Vec3 Vector_Div(Vec3 &v1, float k);
float Vector_DotProduct(Vec3 &v1, Vec3 &v2);
float Vector_Length(Vec3 &v);
Vec3 Vector_Normalise(Vec3 &v);
Vec3 Vector_CrossProduct(Vec3 &v1, Vec3 &v2);

template <typename t>
t clamp2(t x, t min, t max)
{
  if (x < min)
    x = min;
  if (x > max)
    x = max;
  return x;
}

float max(float a, float b);
void printVector(Vec3 &v);

#endif // __UTILITY_H__
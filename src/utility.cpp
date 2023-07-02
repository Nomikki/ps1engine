#include "utility.hpp"


Vec3 Matrix_MultiplyVector(mat4x4 &m, Vec3 &i)
{
  Vec3 v;
  v.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + i.w * m.m[3][0];
  v.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + i.w * m.m[3][1];
  v.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + i.w * m.m[3][2];
  v.w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + i.w * m.m[3][3];
  return v;
}

mat4x4 Matrix_MakeIdentity()
{
  mat4x4 matrix;
  matrix.m[0][0] = 1.0f;
  matrix.m[1][1] = 1.0f;
  matrix.m[2][2] = 1.0f;
  matrix.m[3][3] = 1.0f;

  return matrix;
}

mat4x4 Matrix_MakeRotationX(float fAngleRad)
{
  mat4x4 matrix;
  matrix.m[0][0] = 1;
  matrix.m[1][1] = cosf(fAngleRad);
  matrix.m[1][2] = sinf(fAngleRad);
  matrix.m[2][1] = -sinf(fAngleRad);
  matrix.m[2][2] = cosf(fAngleRad);
  matrix.m[3][3] = 1;
  return matrix;
}
mat4x4 Matrix_MakeRotationY(float fAngleRad)
{
  mat4x4 matrix;
  matrix.m[0][0] = cosf(fAngleRad);
  matrix.m[0][2] = sinf(fAngleRad);
  matrix.m[2][0] = -sinf(fAngleRad);
  matrix.m[1][1] = 1;
  matrix.m[2][2] = cosf(fAngleRad);
  matrix.m[3][3] = 1;
  return matrix;
}
mat4x4 Matrix_MakeRotationZ(float fAngleRad)
{
  mat4x4 matrix;
  matrix.m[0][0] = cosf(fAngleRad);
  matrix.m[0][1] = sinf(fAngleRad);
  matrix.m[1][0] = -sinf(fAngleRad);
  matrix.m[1][1] = cosf(fAngleRad);
  matrix.m[2][2] = 1;
  matrix.m[3][3] = 1;
  return matrix;
}

mat4x4 Matrix_MakeTranslation(float x, float y, float z)
{
  mat4x4 matrix;
  matrix.m[0][0] = 1.0f;
  matrix.m[1][1] = 1.0f;
  matrix.m[2][2] = 1.0f;
  matrix.m[3][3] = 1.0f;
  matrix.m[3][0] = x;
  matrix.m[3][1] = y;
  matrix.m[3][2] = z;
  return matrix;
}

mat4x4 Matrix_MakeTranslation(Vec3 pos)
{
  return Matrix_MakeTranslation(pos.x, pos.y, pos.z);
}

mat4x4 Matrix_MakeProjection(float fDovDegrees, float fAspectRatio, float fNear, float fFar)
{
  float fFovRad = 1.0f / tanf(fDovDegrees * 0.5f / 180.0f * 3.14159f);
  mat4x4 matrix;
  matrix.m[0][0] = fAspectRatio * fFovRad;
  matrix.m[1][1] = fFovRad;
  matrix.m[2][2] = fFar / (fFar - fNear);
  matrix.m[3][2] = (-fFar * fNear) / (fFar - fNear);
  matrix.m[2][3] = 1;
  matrix.m[3][3] = 0;
  return matrix;
}

mat4x4 Matrix_MultiplyMatrix(mat4x4 &m1, mat4x4 &m2)
{
  mat4x4 matrix;
  for (int c = 0; c < 4; c++)
    for (int r = 0; r < 4; r++)
      matrix.m[r][c] = m1.m[r][0] * m2.m[0][c] +
                       m1.m[r][1] * m2.m[1][c] +
                       m1.m[r][2] * m2.m[2][c] +
                       m1.m[r][3] * m2.m[3][c];
  return matrix;
}

mat4x4 Matrix_PointAt(Vec3 &pos, Vec3 &target, Vec3 &up)
{
  // calculate forward direction
  Vec3 newForward = Vector_Sub(target, pos);
  newForward = Vector_Normalise(newForward);

  // calculate up direction
  Vec3 a = Vector_Mul(newForward, Vector_DotProduct(up, newForward));
  Vec3 newUp = Vector_Sub(up, a);
  newUp = Vector_Normalise(newUp);

  // new right direction is just cross product
  Vec3 newRight = Vector_CrossProduct(newUp, newForward);

  mat4x4 matrix;
  matrix.m[0][0] = newRight.x;
  matrix.m[0][1] = newRight.y;
  matrix.m[0][2] = newRight.z;
  matrix.m[0][3] = 0;
  matrix.m[1][0] = newUp.x;
  matrix.m[1][1] = newUp.y;
  matrix.m[1][2] = newUp.z;
  matrix.m[1][3] = 0;
  matrix.m[2][0] = newForward.x;
  matrix.m[2][1] = newForward.y;
  matrix.m[2][2] = newForward.z;
  matrix.m[2][3] = 0;
  matrix.m[3][0] = pos.x;
  matrix.m[3][1] = pos.y;
  matrix.m[3][2] = pos.z;
  matrix.m[3][3] = 1;

  return matrix;
}

mat4x4 Matrix_QuickInverse(mat4x4 &m) // only for rotation/translation matrices
{
  mat4x4 matrix;
  matrix.m[0][0] = m.m[0][0];
  matrix.m[0][1] = m.m[1][0];
  matrix.m[0][2] = m.m[2][0];
  matrix.m[0][3] = 0.0f;
  matrix.m[1][0] = m.m[0][1];
  matrix.m[1][1] = m.m[1][1];
  matrix.m[1][2] = m.m[2][1];
  matrix.m[1][3] = 0.0f;
  matrix.m[2][0] = m.m[0][2];
  matrix.m[2][1] = m.m[1][2];
  matrix.m[2][2] = m.m[2][2];
  matrix.m[2][3] = 0.0f;
  matrix.m[3][0] = -(m.m[3][0] * matrix.m[0][0] + m.m[3][1] * matrix.m[1][0] + m.m[3][2] * matrix.m[2][0]);
  matrix.m[3][1] = -(m.m[3][0] * matrix.m[0][1] + m.m[3][1] * matrix.m[1][1] + m.m[3][2] * matrix.m[2][1]);
  matrix.m[3][2] = -(m.m[3][0] * matrix.m[0][2] + m.m[3][1] * matrix.m[1][2] + m.m[3][2] * matrix.m[2][2]);
  matrix.m[3][3] = 1.0f;
  return matrix;
}

Vec3 Vector_Add(Vec3 &v1, Vec3 &v2)
{
  return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

Vec3 Vector_Sub(Vec3 &v1, Vec3 &v2)
{
  return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

Vec3 Vector_Mul(Vec3 &v1, float k)
{
  return {v1.x * k, v1.y * k, v1.z * k};
}

Vec3 Vector_Div(Vec3 &v1, float k)
{
  return {v1.x / k, v1.y / k, v1.z / k};
}

float Vector_DotProduct(Vec3 &v1, Vec3 &v2)
{
  return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float Vector_Length(Vec3 &v)
{
  return sqrtf(Vector_DotProduct(v, v));
}

Vec3 Vector_Normalise(Vec3 &v)
{
  float l = Vector_Length(v);
  return {v.x / l, v.y / l, v.z / l};
}

Vec3 Vector_CrossProduct(Vec3 &v1, Vec3 &v2)
{
  Vec3 v;
  v.x = v1.y * v2.z - v1.z * v2.y;
  v.y = v1.z * v2.x - v1.x * v2.z;
  v.z = v1.x * v2.y - v1.y * v2.x;
  return v;
}

Vec3 Vector_IntersectPlane(Vec3 &plane_p, Vec3 &plane_n, Vec3 &lineStart, Vec3 &lineEnd, float &t)
{
  plane_n = Vector_Normalise(plane_n);
  float plane_d = -Vector_DotProduct(plane_n, plane_p);
  float ad = Vector_DotProduct(lineStart, plane_n);
  float bd = Vector_DotProduct(lineEnd, plane_n);
  t = (-plane_d - ad) / (bd - ad); //
  Vec3 lineStartToEnd = Vector_Sub(lineEnd, lineStart);
  Vec3 lineToIntersect = Vector_Mul(lineStartToEnd, t);
  return Vector_Add(lineStart, lineToIntersect);
}

int Triangle_CLipAgainstPlane(Vec3 &plane_p, Vec3 &plane_n, Triangle &in_tri, Triangle &out_tri1, Triangle &out_tri2)
{
  // Make sure plane normal is indeed normal
  plane_n = Vector_Normalise(plane_n);

  // Return signed shortest distance from point to plane, plane normal must be normalised
  auto dist = [&](Vec3 &p)
  {
    Vec3 n = Vector_Normalise(p);
    return (plane_n.x * p.x + plane_n.y * p.y + plane_n.z * p.z - Vector_DotProduct(plane_n, plane_p));
  };

  // Create two temporary storage arrays to classify points either side of plane
  // If distance sign is positive, point lies on "inside" of plane
  Vec3 *inside_points[3];
  int nInsidePointCount = 0;
  Vec3 *outside_points[3];
  int nOutsidePointCount = 0;
  UV *inside_tex[3];
  int nInsideTexCount = 0;
  UV *outside_tex[3];
  int nOutsideTexCount = 0;

  // Get signed distance of each point in triangle to plane
  float d0 = dist(in_tri.p[0]);
  float d1 = dist(in_tri.p[1]);
  float d2 = dist(in_tri.p[2]);

  if (d0 >= 0)
  {
    inside_points[nInsidePointCount++] = &in_tri.p[0];
    inside_tex[nInsideTexCount++] = &in_tri.t[0];
  }
  else
  {
    outside_points[nOutsidePointCount++] = &in_tri.p[0];
    outside_tex[nOutsideTexCount++] = &in_tri.t[0];
  }
  if (d1 >= 0)
  {
    inside_points[nInsidePointCount++] = &in_tri.p[1];
    inside_tex[nInsideTexCount++] = &in_tri.t[1];
  }
  else
  {
    outside_points[nOutsidePointCount++] = &in_tri.p[1];
    outside_tex[nOutsideTexCount++] = &in_tri.t[1];
  }
  if (d2 >= 0)
  {
    inside_points[nInsidePointCount++] = &in_tri.p[2];
    inside_tex[nInsideTexCount++] = &in_tri.t[2];
  }
  else
  {
    outside_points[nOutsidePointCount++] = &in_tri.p[2];
    outside_tex[nOutsideTexCount++] = &in_tri.t[2];
  }

  // Now classify triangle points, and break the input triangle into
  // smaller output triangles if required. There are four possible
  // outcomes...

  if (nInsidePointCount == 0)
  {
    // All points lie on the outside of plane, so clip whole triangle
    // It ceases to exist

    return 0; // No returned triangles are valid
  }

  if (nInsidePointCount == 3)
  {
    // All points lie on the inside of plane, so do nothing
    // and allow the triangle to simply pass through
    out_tri1 = in_tri;

    return 1; // Just the one returned original triangle is valid
  }

  if (nInsidePointCount == 1 && nOutsidePointCount == 2)
  {
    // Triangle should be clipped. As two points lie outside
    // the plane, the triangle simply becomes a smaller triangle

    // Copy appearance info to new triangle
    out_tri1.color = in_tri.color;

    // The inside point is valid, so keep that...
    out_tri1.p[0] = *inside_points[0];
    out_tri1.t[0] = *inside_tex[0];

    // but the two new points are at the locations where the
    // original sides of the triangle (lines) intersect with the plane
    float t;
    out_tri1.p[1] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
    out_tri1.t[1].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
    out_tri1.t[1].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;

    out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[1], t);
    out_tri1.t[2].u = t * (outside_tex[1]->u - inside_tex[0]->u) + inside_tex[0]->u;
    out_tri1.t[2].v = t * (outside_tex[1]->v - inside_tex[0]->v) + inside_tex[0]->v;

    return 1; // Return the newly formed single triangle
  }

  if (nInsidePointCount == 2 && nOutsidePointCount == 1)
  {
    // Triangle should be clipped. As two points lie inside the plane,
    // the clipped triangle becomes a "quad". Fortunately, we can
    // represent a quad with two new triangles

    // Copy appearance info to new triangles
    out_tri1.color = in_tri.color;

    out_tri2.color = in_tri.color;

    // The first triangle consists of the two inside points and a new
    // point determined by the location where one side of the triangle
    // intersects with the plane
    out_tri1.p[0] = *inside_points[0];
    out_tri1.p[1] = *inside_points[1];
    out_tri1.t[0] = *inside_tex[0];
    out_tri1.t[1] = *inside_tex[1];

    float t;
    out_tri1.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[0], *outside_points[0], t);
    out_tri1.t[2].u = t * (outside_tex[0]->u - inside_tex[0]->u) + inside_tex[0]->u;
    out_tri1.t[2].v = t * (outside_tex[0]->v - inside_tex[0]->v) + inside_tex[0]->v;

    // The second triangle is composed of one of he inside points, a
    // new point determined by the intersection of the other side of the
    // triangle and the plane, and the newly created point above
    out_tri2.p[0] = *inside_points[1];
    out_tri2.t[0] = *inside_tex[1];
    out_tri2.p[1] = out_tri1.p[2];
    out_tri2.t[1] = out_tri1.t[2];
    out_tri2.p[2] = Vector_IntersectPlane(plane_p, plane_n, *inside_points[1], *outside_points[0], t);
    out_tri2.t[2].u = t * (outside_tex[0]->u - inside_tex[1]->u) + inside_tex[1]->u;
    out_tri2.t[2].v = t * (outside_tex[0]->v - inside_tex[1]->v) + inside_tex[1]->v;
    return 2; // Return two newly formed triangles which form a quad
  }
}


Color quantise(Color &color)
{
  constexpr int nBits = 5;
  constexpr float fLevels = (1 << nBits) - 1;

  int tr = int(color.r / 255.0f * fLevels) / fLevels * 255;
  int tg = int(color.g / 255.0f * fLevels) / fLevels * 255;
  int tb = int(color.b / 255.0f * fLevels) / fLevels * 255;

  return {(uint8_t)tr, (uint8_t)tg, (uint8_t)tb};
}

float max(float a, float b)
{
  if (a > b)
    return a;
  return b;
}

void printVector(Vec3 &v)
{
  printf("%0.2f, %0.2f, %0.2f\n", v.x, v.y, v.z);
}

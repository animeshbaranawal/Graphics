/*
 * Primitive.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 *      Modified: sidch
 */

#include "Primitives.hpp"

Primitive::Primitive(RGB const & c, Material const & m, Mat4 const & modelToWorld)
{
  c_ = c;
  m_ = m;
  modelToWorld_ = modelToWorld;
  worldToModel_ = modelToWorld.inverse();
}

Primitive::~Primitive()
{
}

Sphere::Sphere(double radius, RGB const & c, Material const & m, Mat4 const & modelToWorld): Primitive(c, m, modelToWorld)
{
  r_ = radius;
}

bool
Sphere::intersect(Ray & ray) const
{
  // transform world coordinates to local coordinates
  ray.transform(worldToModel_);

  double dot = (ray.start()*ray.direction());
  double discriminant1 = std::pow(dot,2);
  double discriminant2 = ray.direction().length2()*(ray.start().length2() - r_*r_);
  discriminant1 -= discriminant2;
  if(discriminant1 < 0){ return false; } // if discriminant negative implies no intersection
  else{ // else
    double t1 = -dot + std::sqrt(discriminant1);
    double t2 = -dot - std::sqrt(discriminant1);
    t1 = t1/ray.direction().length2();
    t2 = t2/ray.direction().length2();
    if((t1 <= 0 && t2 <= 0) || (t1 > ray.minT() && t2 > ray.minT())){
      return false;
    }
    else{
      if(t2 <= 0){ ray.setMinT(t1); }
      else{ ray.setMinT(t2); }
      return true;
    }
  }
}

Vec3
Sphere::calculateNormal(Vec3 const & position) const
{
  // convert world coordinates to local coordinates
  Vec3 local_position = Vec3(worldToModel_* Vec4(position, 1.0));
  Vec3 local_normal_direction = (local_position)/r_ ; // local normal
  Mat4 normalMatrix = modelToWorld_.inverse().transpose();
  Vec3 world_normal_direction = Vec3(normalMatrix * Vec4(local_normal_direction, 0.0), 3); // transform to global normal
  return world_normal_direction.normalize();
}

//=============================================================================================================================
// Triangle and other primitives are for Assignment 3b, after the midsem. Do not do this for 3a.
//=============================================================================================================================

Triangle::Triangle(Vec3 const & v0, Vec3 const & v1, Vec3 const & v2, RGB const & c, Material const & m,
                   Mat4 const & modelToWorld)
: Primitive(c, m, modelToWorld)
{
  verts[0] = v0;
  verts[1] = v1;
  verts[2] = v2;
  Vec3 dir1 = verts[2] - verts[1];
  Vec3 dir2 = verts[0] - verts[1];
  surfaceNormal = dir1 ^ dir2;
  area_ = 0.5 * surfaceNormal.length();
  surfaceNormal.normalize();
  vertexNormal[0] = surfaceNormal;
  vertexNormal[1] = surfaceNormal;
  vertexNormal[2] = surfaceNormal;
}

bool
Triangle::intersect(Ray & ray) const
{
  // solve the equation (s + cd - b).n = 0 for c where
  // s -> starting point of ray
  // d -> direction of ray
  // b -> any one vertex of triangle
  // n -> local normal of Triangle
  // Calculate local normal
  ray.transform(worldToModel_);

  double RHS = verts[1]*surfaceNormal;
  double LHS1 = ray.start()*surfaceNormal;
  double LHS2 = ray.direction()*surfaceNormal;
  if(LHS2 == 0){ return false; } // Assumption : grazing condition
  else{
    double t = (RHS - LHS1)/LHS2;
    if(t <= 0 || t > ray.minT()){ return false; }
    else{
      // check if point lies inside Triangle
      Vec3 p = ray.start() + t*ray.direction();
      Vec3 p0 = ray.start();

      // courtesy : https://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld018.htm
      Vec3 n1 = (verts[1] - p0)^(verts[0] - p0); n1.normalize();
      double d1 = p*n1 - p0*n1; if(d1 < 0) return false;
      n1 = (verts[2] - p0)^(verts[1] - p0); n1.normalize();
      d1 = p*n1 - p0*n1; if(d1 < 0) return false;
      n1 = (verts[0] - p0)^(verts[2] - p0); n1.normalize();
      d1 = p*n1 - p0*n1; if(d1 < 0) return false;

      ray.setMinT(t);
      return true;
    }
  }
}

Vec3
Triangle::calculateNormal(Vec3 const & position) const
{
  // Assumption: points assumed to be in counter clockwise direction
  Vec3 local_position = Vec3(worldToModel_* Vec4(position, 1.0));
  double weight1 = 0, weight2 = 0, weight3 = 0;
  weight1 = ((local_position - verts[1])^(local_position - verts[2])).length()*0.5/area_;
  weight2 = ((local_position - verts[0])^(local_position - verts[2])).length()*0.5/area_;
  weight3 = ((local_position - verts[0])^(local_position - verts[1])).length()*0.5/area_;

  Vec3 local_normal_direction = vertexNormal[0]*weight1 + vertexNormal[1]*weight2 + vertexNormal[2]*weight3;
  local_normal_direction.normalize();
  Mat4 normalMatrix = modelToWorld_.inverse().transpose();
  Vec3 world_normal_direction = Vec3(normalMatrix * Vec4(local_normal_direction, 0.0), 3); // transform to global normal
  return world_normal_direction.normalize(); // transformation from local to global and then normalize
}

Vec3
Triangle::getvert1(){ return verts[0]; }

Vec3
Triangle::getvert2(){ return verts[1]; }

Vec3
Triangle::getvert3(){ return verts[2]; }

Vec3
Triangle::getSurfaceNormal(){ return surfaceNormal; }

double
Triangle::getArea(){ return area_; }

void
Triangle::addVertNorm1(Vec3 x){ vertexNormal[0] += x; }

void
Triangle::addVertNorm2(Vec3 x){ vertexNormal[1] += x; }

void
Triangle::addVertNorm3(Vec3 x){ vertexNormal[2] += x; }

void
Triangle::findCommon(Triangle* tri) {
  if(verts[0] == (tri->getvert1)()){
    addVertNorm1(tri->getSurfaceNormal());
    tri -> addVertNorm1(surfaceNormal);
  }
  else if(verts[0] == (tri->getvert2)()){
    addVertNorm1(tri->getSurfaceNormal());
    tri -> addVertNorm2(surfaceNormal);
  }
  else if(verts[0] == (tri->getvert3)()){
    addVertNorm1(tri->getSurfaceNormal());
    tri -> addVertNorm3(surfaceNormal);
  }

  if(verts[1] == (tri->getvert1)()){
    addVertNorm2(tri->getSurfaceNormal());
    tri -> addVertNorm1(surfaceNormal);
  }
  else if(verts[1] == (tri->getvert2)()){
    addVertNorm2(tri->getSurfaceNormal());
    tri -> addVertNorm2(surfaceNormal);
  }
  else if(verts[1] == (tri->getvert3)()){
    addVertNorm2(tri->getSurfaceNormal());
    tri -> addVertNorm3(surfaceNormal);
  }

  if(verts[2] == (tri->getvert1)()){
    addVertNorm3(tri->getSurfaceNormal());
    tri -> addVertNorm1(surfaceNormal);
  }
  else if(verts[2] == (tri->getvert2)()){
    addVertNorm3(tri->getSurfaceNormal());
    tri -> addVertNorm2(surfaceNormal);
  }
  else if(verts[2] == (tri->getvert3)()){
    addVertNorm3(tri->getSurfaceNormal());
    tri -> addVertNorm3(surfaceNormal);
  }
}

void
Triangle::computeVertexNormal() {
  vertexNormal[0].normalize();
  vertexNormal[1].normalize();
  vertexNormal[2].normalize();
}

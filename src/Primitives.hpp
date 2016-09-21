/*
 * Primitive.hpp
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 *      Modified: sidch
 */

#ifndef __Primitive_hpp__
#define __Primitive_hpp__

#include "Globals.hpp"

/** Interface for a scene primitive (e.g. a sphere). */
class Primitive
{
  protected:
    Primitive(RGB const & c, Material const & m, Mat4 const & modelToWorld);  // primitives are an abstract class

  public:
    /** Destructor. */
    virtual ~Primitive();

    /**
     * Checks for intersection with the given ray. If there is a valid intersection which is smaller than the ray's current
     * minimum hit time (ray.minT()), then updates ray.minT() to the new hit time (as a multiple of the ray length). The ray is
     * specified in world space.
     *
     * @return True if there was a valid intersection AND the minimum hit time was lowered.
     *
     * !!! REMEMBER TO TAKE THE PRIMITIVE TRANSFORM (modelToWorld_/worldToModel_) INTO ACCOUNT !!!
     */
    virtual bool intersect(Ray & ray) const = 0;

    /**
     * Calculates the normal for the given position on this primitive. You may assume the position is actually on the
     * primitive. The position is specified in world space.
     *
     * !!! REMEMBER TO TAKE THE PRIMITIVE TRANSFORM (modelToWorld_/worldToModel_) INTO ACCOUNT !!!
     */
    virtual Vec3 calculateNormal(Vec3 const & position) const = 0;

    /** Set the primitive's color. */
    void setColor(RGB const & c) { c_ = c; }

    /** Set the primitive's material. */
    void setMaterial(Material const & m) { m_ = m; }

    /** Get the primitive's color. */
    RGB const & getColor() const { return c_; }

    /** Get the primitive's material. */
    Material const & getMaterial() const { return m_; }

  protected:
    Mat4 modelToWorld_;
    Mat4 worldToModel_;

  private:
    RGB c_;
    Material m_;
};

/** A sphere primitive. */
class Sphere : public Primitive
{
  public:
    /** Constructor. */
    Sphere(double radius, RGB const & c, Material const & m, Mat4 const & modelToWorld);

    bool intersect(Ray & ray) const;
    Vec3 calculateNormal(Vec3 const & position) const;

  private:
    double r_;
};

//=============================================================================================================================
// Triangle and other primitives are for Assignment 3b, after the midsem. Do not do this for 3a. The skeleton is included here
// as a preview.
//=============================================================================================================================

/** A triangle primitive. */
class Triangle : public Primitive
{
  public:
    /** Constructor. */
    Triangle(Vec3 const & v0, Vec3 const & v1, Vec3 const & v2, RGB const & c, Material const & m, Mat4 const & modelToWorld);

    bool intersect(Ray & ray) const;
    Vec3 calculateNormal(Vec3 const & position) const;
    void findCommon(Triangle* tri); /* custome function */

    Vec3 getvert1(); /* custome function */
    Vec3 getvert2(); /* custome function */
    Vec3 getvert3(); /* custome function */
    Vec3 getSurfaceNormal(); /* custome function */
    double getArea(); /* custome function */

    void pushvert1(Triangle* tri); /* custome function */
    void pushvert2(Triangle* tri); /* custome function */
    void pushvert3(Triangle* tri); /* custome function */
    void addVertNorm1(Vec3 x); /* custome function */
    void addVertNorm2(Vec3 x); /* custome function */
    void addVertNorm3(Vec3 x); /* custome function */
    void computeVertexNormal(); /* custome function */

  private:
    Vec3 verts[3];
    double area_;
    Vec3 surfaceNormal;
    Vec3 vertexNormal[3];
};

#endif  // __Primitive_hpp__

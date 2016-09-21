/*
 * World.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 *      Modified: sidch
 */

#include "World.hpp"

World::World()
{
}

World::~World()
{
  // TODO Auto-generated destructor stub
}

Primitive *
World::intersect(Ray & r) const
{
  // IMPLEMENT_ME(__FILE__, __LINE__);
  Primitive* nearest = NULL;

  for(PrimitiveConstIterator i = primitivesBegin(); i != primitivesEnd(); ++i){
    Ray copy(r);
  	if(((*i)->intersect)(copy)){
      r.setMinT(copy.minT());
      nearest = (*i);
    }
  }

  return nearest;
}

void
World::addPrimitive(Primitive * p)
{
  primitives_.push_back(p);
}

void
World::addLight(Light * l)
{
  lights_.push_back(l);
}

void
World::setAmbientLightColor(RGB ambientColor)
{
  ambientLight_.setColor(ambientColor);
}

RGB
World::getAmbientLightColor() const
{
  return ambientLight_.getColor();
}

void
World::printStats() const
{
  std::cout << "World data:" << std::endl;
  std::cout << " primitives: " << primitives_.size() << std::endl;
  std::cout << " lights: " << lights_.size() << std::endl;
}

void
World::trianglesUpdate(Triangle* tri) {
  for(unsigned int i=0; i < triangles.size(); i++){
    (*triangles[i]).findCommon(tri);
  }
  triangles.push_back(tri);
}

void
World::trianglesComputerVertexNormal(){
  for(unsigned int i=0; i < triangles.size(); i++){
    (*triangles[i]).computeVertexNormal();
  }
}

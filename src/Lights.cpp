/*
 * Lights.cpp
 *
 *  Created on: Feb 19, 2009
 *      Author: njoubert
 *      Modified: sidch
 */

#include "Lights.hpp"

Light::Light()
{
  RGB black(0, 0, 0);
  illumination_ = black;
  falloff_ = 0;
  angular_falloff_ = 0;
  dead_distance_ = 1;
}

Light::Light(RGB const & illumination)
{
  illumination_ = illumination;
}

Light::Light(RGB const & illumination, double falloff, double dead_distance)
{
  illumination_ = illumination;
  falloff_ = falloff;
  dead_distance_ = dead_distance;
}

Light::~Light()
{}

RGB Light::getColor() const
{
  return illumination_;
}

RGB Light::getColor(Vec3 const & p) const
{
  return illumination_;
}

void
Light::setColor(RGB const & c)
{
  illumination_ = c;
}

AmbientLight::AmbientLight()
{
  // intentionally empty
}

AmbientLight::AmbientLight(RGB const & illumination) : Light(illumination)
{
  // intentionally empty
}

std::vector<Vec3>
AmbientLight::getIncidenceVector(Vec3 const & position) const
{
  throw "AMBIENT LIGHTS DO NOT HAVE A SENSE OF DIRECTION OR POSITION`";
}

std::vector<Ray> AmbientLight::getShadowRay(Vec3 const & position, bool & use_dist) const
{
  throw "AMBIENT LIGHTS DO NOT HAVE A SENSE OF DIRECTION OR POSITION";
}

PointLight::PointLight(RGB const & illumination) : Light(illumination)
{
  // intentionally empty
}

PointLight::PointLight(RGB const & illumination, double falloff, double dead_distance)
: Light(illumination, falloff, dead_distance)
{
  // intentionally empty
}

RGB
PointLight::getColor(Vec3 const & p) const
{
  // Since the fallout color depends the distance between
  // the light source and object point, we pass the distance vector
  // instead of position vector
  double distance = p.length();
  double scale = 1.0/(distance + dead_distance_);
  scale = std::pow(scale,falloff_);
  return illumination_*scale;
}

void
PointLight::setPosition(Vec3 const & pos)
{
  pos_ = pos;
}

std::vector<Vec3>
PointLight::getIncidenceVector(Vec3 const & position) const
{
  // generate a single incident ray
  std::vector<Vec3> incidentVectors;
  incidentVectors.push_back((pos_ - position));
  return incidentVectors;
}

std::vector<Ray>
PointLight::getShadowRay(Vec3 const & position, bool & use_dist) const
{
  // generate a single shadow ray
  use_dist = true;
  std::vector<Ray> shadowRays;
  shadowRays.push_back(Ray::fromOriginAndEnd(position,pos_,1));
  return shadowRays;
}

DirectionalLight::DirectionalLight(RGB const & illumination) : Light(illumination)
{
  // intentionally empty
}

void
DirectionalLight::setDirection(Vec3 const & dir)
{
  dir_ = dir;
  dir_.normalize();
}

std::vector<Vec3>
DirectionalLight::getIncidenceVector(Vec3 const & position) const
{
  // generate a single incident ray
  std::vector<Vec3> incidentVectors;
  incidentVectors.push_back((-1*dir_));
  return incidentVectors;
}

std::vector<Ray>
DirectionalLight::getShadowRay(Vec3 const & position, bool & use_dist) const
{
  // generate a single shadow ray
  use_dist = false;
  std::vector<Ray> shadowRays;
  shadowRays.push_back(Ray::fromOriginAndDirection(position,-dir_));
  return shadowRays;
}

AreaLightSquare::AreaLightSquare(RGB const & illumination) : Light(illumination)
{
}

AreaLightSquare::AreaLightSquare(RGB const & illumination, double falloff, double dead_distance)
: Light(illumination, falloff, dead_distance)
{
}

RGB
AreaLightSquare::getColor(Vec3 const & p) const
{
  // Since the fallout color depends the distance between
  // the light source and object point, we pass the distance vector
  // instead of position vector
  double distance = p.length();
  double scale = 1.0/(distance + dead_distance_);
  scale = std::pow(scale,falloff_);
  return illumination_*scale;
}

void
AreaLightSquare::setPosition(Vec3 const & pos)
{
  pos_ = pos;
}

void
AreaLightSquare::setSide(double const & side)
{
  side_ = side;
}

std::vector<Vec3>
AreaLightSquare::getIncidenceVector(Vec3 const & position) const
{
  double xStep = 0.25;
  double yStep = 0.25;
  double startY = pos_.y() - side_/2;
  std::vector<Vec3> incidentVectors;

  // for every 0.25 x 0.25 square on area light, generate an incident ray
  // with random jittering
  srand(seed);
  for(; startY < pos_.y() + side_/2; startY += yStep){
    double startX = pos_.x() - side_/2;
    for(; startX < pos_.x() + side_/2; startX += xStep){
      double jitter_x = 0.15 * (std::rand() / (double)RAND_MAX);
      double jitter_y = 0.15 * (std::rand() / (double)RAND_MAX);
      Vec3 areaPos_ = Vec3(std::min(startX + jitter_x, pos_.x() + side_/2), std::min(startY + jitter_y, pos_.y() + side_/2), pos_.z());
      incidentVectors.push_back((areaPos_ - position));
    }
  }

  return incidentVectors;
}

std::vector<Ray>
AreaLightSquare::getShadowRay(Vec3 const & position, bool & use_dist) const
{
  use_dist = true;
  double xStep = 0.25;
  double yStep = 0.25;
  double startY = pos_.y() - side_/2;
  std::vector<Ray> shadowRays;

  // for every 0.25 x 0.25 square on area light, generate a shadow ray
  // with random jittering
  srand(seed);
  for(; startY < pos_.y() + side_/2; startY += yStep){
    double startX = pos_.x() - side_/2;
    for(; startX < pos_.x() + side_/2; startX += xStep){
      double jitter_x = 0.15 * (std::rand() / (double)RAND_MAX);
      double jitter_y = 0.15 * (std::rand() / (double)RAND_MAX);
      Vec3 areaPos_ = Vec3(std::min(startX + jitter_x, pos_.x() + side_/2), std::min(startY + jitter_y, pos_.y() + side_/2), pos_.z());
      shadowRays.push_back(Ray::fromOriginAndEnd(position,areaPos_,1));
    }
  }

  return shadowRays;
}

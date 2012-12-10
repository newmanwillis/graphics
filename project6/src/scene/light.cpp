#include <cmath>
#include "light.h"
#include "ray.h"


using namespace std;

double DirectionalLight::distanceAttenuation( const Vec3d& P ) const
{
  // distance to light is infinite, so f(di) goes to 0.  Return 1.
  return 1.0;
}


Vec3d DirectionalLight::shadowAttenuation( const Vec3d& P ) const
{
  Vec3d shadow_dir = -orientation;
  shadow_dir.normalize();
  ray shadow_ray = ray(P, shadow_dir, ray::SHADOW);
  isect i;
<<<<<<< HEAD
  Vec3d shadow_att = Vec3d(1, 1, 1);
  while (scene->intersect( shadow_ray, i )){
    Material m = i.getMaterial();
    shadow_att = prod(shadow_att, m.kt(i)); 
    shadow_ray = ray(shadow_ray.at(i.t), shadow_dir, ray::SHADOW);
  }
  return shadow_att;
=======
  Vec3d sa = Vec3d(1.0, 1.0, 1.0);  // shadow attenuation initializer
  while (scene->intersect(shadow_ray, i)) {
    Material m = i.getMaterial();
    sa = prod(sa, m.kt(i)); 
    shadow_ray = ray(shadow_ray.at(i.t), shadow_dir, ray::SHADOW);
  }
  return sa;
>>>>>>> 97e04c90f68f6dddf906cdd9c0600059eaac6d53
}

Vec3d DirectionalLight::getColor( const Vec3d& P ) const
{
  // Color doesn't depend on P 
  return color;
}

Vec3d DirectionalLight::getDirection( const Vec3d& P ) const
{
  return -orientation;
}

double PointLight::distanceAttenuation( const Vec3d& P ) const
{
  float distance = (P - position).length();
  // use provided terms in light.h to find the distance term for phong model
  float dist_attenuation = 1.0 / (constantTerm + (linearTerm * distance) +
                                   quadraticTerm * pow(distance, 2.0));
  return min((float)1.0, dist_attenuation);
}

Vec3d PointLight::getColor( const Vec3d& P ) const
{
  // Color doesn't depend on P 
  return color;
}

Vec3d PointLight::getDirection( const Vec3d& P ) const
{
  Vec3d ret = position - P;
  ret.normalize();
  return ret;
}


Vec3d PointLight::shadowAttenuation(const Vec3d& P) const
{
  isect i;
  Vec3d shadow_dir = position-P;
  shadow_dir.normalize();
  ray shadow_ray = ray(P, shadow_dir, ray::SHADOW);
<<<<<<< HEAD
  Vec3d shadow_att = Vec3d(1, 1, 1);
  while (scene->intersect( shadow_ray, i )){
    if(shadow_ray.at(i.t) == position)
=======
  isect i;
  Vec3d sa = Vec3d(1.0, 1.0, 1.0);
  while (scene->intersect(shadow_ray, i)) {
    if (shadow_ray.at(i.t) == position)
>>>>>>> 97e04c90f68f6dddf906cdd9c0600059eaac6d53
      break;
    Material m = i.getMaterial();
    shadow_att = prod(shadow_att, m.kt(i)); 
    shadow_ray = ray(shadow_ray.at(i.t), shadow_dir, ray::SHADOW);
  } 
  return shadow_att;
}

#include <cmath>

#include "light.h"



using namespace std;

double DirectionalLight::distanceAttenuation( const Vec3d& P ) const
{
  // distance to light is infinite, so f(di) goes to 0.  Return 1.
  return 1.0;
}


Vec3d DirectionalLight::shadowAttenuation( const Vec3d& P ) const
{
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.

  return Vec3d(1,1,1);

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
  float dist_attenuation = 1/(constantTerm + (linearTerm * distance) + quadraticTerm*pow(distance, 2.0));
  cout << "distance atten: " << dist_attenuation << endl;
  return min((float)1.0, dist_attenuation);
   // return 1.0;

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
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.

  return Vec3d(1,1,1);

}

#include "Ray.h"


Ray::Ray( const Vector3& point, const Vector3& dir, int bounces )
{
	mPoint = point;
	mDirection = dir;
	mBounces = bounces;
}


Ray::~Ray(void)
{
}

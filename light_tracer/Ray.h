#pragma once

#include "Vector3.h"

class Ray
{
public:
	Ray() {};
	Ray( const Vector3& point, const Vector3& dir, int bounces );
	~Ray(void);

	Vector3 mPoint;
	Vector3 mDirection;
	int mBounces;
};


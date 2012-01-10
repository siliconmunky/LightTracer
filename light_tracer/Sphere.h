/*
 *  Sphere.h
 *  light_tracer
 *
 *  Created by Peter Andrews on 09/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

#include "Shape.h"
#include "Vector3.h"

class Sphere : public Shape
{
public:
	Sphere() {};
	Sphere( Vector3 position, float radius );
	
	virtual bool IsHitByRay( const Ray& ray, float& distance );
	virtual ColourRGB GetColourFromRay( const Ray& ray );
	
private:
	Vector3 mPosition;
	float mRadius;
};
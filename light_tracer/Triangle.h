/*
 *  Triangle.h
 *  light_tracer
 *
 *  Created by Peter Andrews on 09/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

#include "Shape.h"
#include "Vector3.h"

class Triangle : public Shape
{
public:
	Triangle() {};
	Triangle( Vector3 v0, Vector3 v1, Vector3 v2 );
	
	virtual bool IsHitByRay( const Ray& ray, float& distance );
	virtual ColourRGB GetColourFromRay( const Ray& ray );
	
private:
	Vector3 mV0;
	Vector3 mV1;
	Vector3 mV2;
	Vector3 mNormal;
};
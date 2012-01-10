/*
 *  Shape.h
 *  light_tracer
 *
 *  Created by Peter Andrews on 09/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once
#include "Ray.h"
#include "ColourRGB.h"
#include "Vector3.h"

class Shape
{
public:
	virtual bool IsHitByRay( const Ray& ray, float& distance ) = 0;
	virtual ColourRGB GetColourFromRay( const Ray& ray ) = 0;

	ColourRGB CalculateLighting( Vector3 point, Vector3 normal, Vector3 view ) const;

private:
	float OrenNayerDiffuse( Vector3 light, Vector3 view, Vector3 normal, float roughness = 0 ) const;
};
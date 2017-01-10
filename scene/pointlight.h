/*
 *  PointLight.h
 *  light_tracer
 *
 *  Created by Peter Andrews on 10/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

#include "../Vector3.h"
#include "../ColourRGB.h"

class PointLight
{
public:
	PointLight() {};
	PointLight( Vector3 position, ColourRGB colour );

	const Vector3& GetPosition() const { return mPosition; };
	const ColourRGB& GetColour() const { return mColour; };

private:
	Vector3 mPosition;
	ColourRGB mColour;
};
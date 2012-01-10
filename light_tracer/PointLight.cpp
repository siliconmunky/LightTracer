/*
 *  PointLight.cpp
 *  light_tracer
 *
 *  Created by Peter Andrews on 10/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "PointLight.h"

PointLight::PointLight( Vector3 position, ColourRGB colour )
: mPosition( position )
, mColour( colour )
{
}
/*
 *  ColourRGB.h
 *  light_tracer
 *
 *  Created by Peter Andrews on 10/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

struct ColourRGB
{
	ColourRGB() { mR = 0.0f; mG = 0.0f; mB = 0.0f; };	
	ColourRGB( float r, float g, float b ) { mR = r; mG = g; mB = b; };
	
	ColourRGB operator +( ColourRGB r ) const
	{
		ColourRGB v;
		v.mR = mR + r.mR;
		v.mG = mG + r.mG;
		v.mB = mB + r.mB;
		return v;
	}
	
	ColourRGB operator /( float r ) const
	{
		ColourRGB v;
		v.mR = mR / r;
		v.mG = mG / r;
		v.mB = mB / r;
		return v;
	}
		
	ColourRGB operator *( float r ) const
	{
		ColourRGB v;
		v.mR = mR * r;
		v.mG = mG * r;
		v.mB = mB * r;
		return v;
	}

	float mR;
	float mG;
	float mB;
};

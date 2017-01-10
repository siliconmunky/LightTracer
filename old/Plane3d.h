//#ifndef PLANE3D_H
//#define PLANE3D_H

#pragma once

#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "d3dx9math.h"

/*	Plane3d
------------------------------------------------------------------------------------------
	
	A cPlane3d in 3D Space represented in point-normal form (Ax + By + Cz + D = 0).

	The convention for the distance constant D is:

	D = -(A, B, C) dot (X, Y, Z)

------------------------------------------------------------------------------------------
*/

class Plane3d
{
public:
	float		distance;
	D3DXVECTOR3	normal;

	void Normalize()
	{
		float mag = D3DXVec3Length( &normal );
		float inv_mag = 1.0f/mag;

		normal.x *= inv_mag;
		normal.y *= inv_mag;
		normal.z *= inv_mag;
		distance *= inv_mag;
	};

	float SignedDistance( const D3DXVECTOR3& point ) const
	{
		return D3DXVec3Dot( &normal, &point ) + distance;
	};

};

/*
float Plane3d::signedDistance( const D3DXVECTOR3& point ) const
{
	return D3DXVec3Dot( &normal, &point ) + distance;
}

void Plane3d::normalize()
{
	float mag = D3DXVec3Length( &normal );
	float inv_mag = 1.0f/mag;

	normal.x *= inv_mag;
	normal.y *= inv_mag;
	normal.z *= inv_mag;
	distance *= inv_mag;
}
*/



//#endif  // end of file      ( cPlane3d.h )


/*
 *  Triangle.cpp
 *  light_tracer
 *
 *  Created by Peter Andrews on 09/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "Triangle.h"
#include "Scene.h"

Triangle::Triangle( Vector3 v0, Vector3 v1, Vector3 v2 )
: mV0( v0 )
, mV1( v1 )
, mV2( v2 )
{
	Vector3 a = mV1 - mV0;
	Vector3 b = mV2 - mV0;
	mNormal = !(a % b);
}

#define SAME_CLOCKNESS 1
#define DIFF_CLOCKNESS 0


int check_same_clock_dir( const Vector3& pt0, const Vector3& pt1, const Vector3& pt2, const Vector3& norm )
{
	// normal of trinagle
	//testi = (((pt2.y - pt1.y)*(pt3.z - pt1.z)) - ((pt3.y - pt1.y)*(pt2.z - pt1.z)));
	//testj = (((pt2.z - pt1.z)*(pt3.x - pt1.x)) - ((pt3.z - pt1.z)*(pt2.x - pt1.x)));
	//testk = (((pt2.x - pt1.x)*(pt3.y - pt1.y)) - ((pt3.x - pt1.x)*(pt2.y - pt1.y)));
	Vector3 a = pt1 - pt0;
	Vector3 b = pt2 - pt0;
	Vector3 new_normal = a % b;

	// Dot product with triangle normal
	float dotprod = new_normal * norm;

	//answer
	if( dotprod < 0 )
	{
		return DIFF_CLOCKNESS;
	}
	else
	{
		return SAME_CLOCKNESS;
	}
}

/*bool Triangle::IsHitByRay( const Ray& ray, float& distance )
{	
	//http://www.angelfire.com/fl/houseofbartlett/solutions/line2tri.html

	// vector form triangle pt1 to pt2
	Vector3 a = mV1 - mV0;

	// vector form triangle pt2 to pt3
	Vector3 b = mV2 - mV1;
   
	// dot product of normal and line's vector if zero line is parallel to triangle
	float dot_prod = mNormal * ray.mDirection;

	if( dot_prod < 0 )
	{
		//Find point of intersect to triangle plane.
		//find t to intersect point
		distance = -(mNormal.mX * (ray.mPoint.mX - mV0.mX) + mNormal.mY * (ray.mPoint.mY - mV0.mY) + mNormal.mZ * (ray.mPoint.mZ - mV0.mZ))/
			(mNormal.mX * ray.mDirection.mX + mNormal.mY * ray.mDirection.mY + mNormal.mZ * ray.mDirection.mZ);

		// if ds is neg line started past triangle so can't hit triangle.
		if( distance < 0 )
		{
			return false;
		}

		Vector3 point_inter = ray.mPoint + ray.mDirection * distance;

		if( check_same_clock_dir( mV0, mV1, point_inter, mNormal) == SAME_CLOCKNESS )
		{
			if( check_same_clock_dir(mV1, mV2, point_inter, mNormal) == SAME_CLOCKNESS )
			{
				if( check_same_clock_dir(mV2, mV0, point_inter, mNormal) == SAME_CLOCKNESS )
				{
				   // answer in pt_int is insde triangle
				   return true;
				}
			}
		}
	}
	return false;
}

ColourRGB Triangle::GetColourFromRay( const Ray& ray )
{
	//Find point of intersect to triangle plane.
	//find t to intersect point
	float distance = -(mNormal.mX * (ray.mPoint.mX - mV0.mX) + mNormal.mY * (ray.mPoint.mY - mV0.mY) + mNormal.mZ * (ray.mPoint.mZ - mV0.mZ))/
		(mNormal.mX * ray.mDirection.mX + mNormal.mY * ray.mDirection.mY + mNormal.mZ * ray.mDirection.mZ);

	Vector3 point_inter = ray.mPoint + ray.mDirection * distance;
		
	return CalculateLighting( point_inter, mNormal, ray.mDirection );
}*/
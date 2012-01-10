/*
 *  Sphere.cpp
 *  light_tracer
 *
 *  Created by Peter Andrews on 09/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "Sphere.h"
#include "Scene.h"
#include <stdlib.h>
using namespace std;

Sphere::Sphere( Vector3 position, float radius )
: mPosition( position )
, mRadius( radius )
{
}

bool Sphere::IsHitByRay( const Ray& ray, float& distance )
{	
	Vector3 to_sphere =	mPosition - ray.mPoint;
	float dot = to_sphere * ray.mDirection;
	
	if( dot < 0.0f )
	{
		return false;
	}
	
	Vector3 point_on_ray = ray.mPoint + (ray.mDirection * dot);
	Vector3 sphere_to_ray = point_on_ray - mPosition;
	float dist_to_ray = ~sphere_to_ray;
	
	bool sphere_hit = dist_to_ray < mRadius;
	
	if( sphere_hit )
	{
		//calc distance to surface
		float length_sphere_to_ray = ~sphere_to_ray;
		float length_back = sqrt( mRadius * mRadius - length_sphere_to_ray * length_sphere_to_ray );
		Vector3 point_on_surface = point_on_ray - (ray.mDirection * length_back);

		distance = ~(point_on_ray - ray.mPoint);
	}
	
	return sphere_hit;
}

ColourRGB Sphere::GetColourFromRay( const Ray& ray )
{	
	Vector3 to_sphere =	mPosition - ray.mPoint;
	float dot = to_sphere * ray.mDirection;
	Vector3 point_on_ray = ray.mPoint + (ray.mDirection * dot);
	Vector3 sphere_to_ray = point_on_ray - mPosition;

	float length_sphere_to_ray = ~sphere_to_ray;
	float length_back = sqrt( mRadius * mRadius - length_sphere_to_ray * length_sphere_to_ray );
	Vector3 point_on_surface = point_on_ray - (ray.mDirection * length_back);
	Vector3 normal = !(point_on_surface - mPosition);

	return CalculateLighting( point_on_surface, normal, ray.mDirection );
}
/*
 *  Scene.cpp
 *  light_tracer
 *
 *  Created by Peter Andrews on 10/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "Scene.h"

#include <iostream>

const int MAX_RAY_BOUNCES = 5;
const int MAX_SHAPES = 10000;
const int MAX_LIGHTS = 10000;
Scene* Scene::mInstance = 0;

void Scene::Init()
{
	mInstance = new Scene();
}

Scene::Scene()
{
	mShapes = new Shape*[MAX_SHAPES];
	memset( mShapes, 0, MAX_SHAPES * sizeof( Shape* ) );
	mNumShapes = 0;

	mPointLights = new PointLight*[MAX_LIGHTS]; 
	memset( mPointLights, 0, MAX_LIGHTS * sizeof( PointLight* ) );
	mNumPointLights = 0;
}

void Scene::AddSphere( Vector3 pos, float radius )
{
	mShapes[mNumShapes++] = new Sphere( pos, radius );
}

void Scene::AddTriangle( Vector3 p1, Vector3 p2, Vector3 p3 )
{
	mShapes[mNumShapes++] = new Triangle( p1, p2, p3 );
}

void Scene::AddPointLight( Vector3 pos, ColourRGB colour, float size, int taps )
{
	for( int i = 0; i < taps; ++i )
	{
		ColourRGB point_colour = colour / taps;
		float x = size * (((float)rand() / RAND_MAX) - 0.5f);
		float y = size * (((float)rand() / RAND_MAX) - 0.5f);
		float z = size * (((float)rand() / RAND_MAX) - 0.5f);
		mPointLights[mNumPointLights++] = new PointLight( pos + Vector3( x, y, z), point_colour );
	}
}

ColourRGB Scene::GetColour( Ray& ray )
{
	if( ray.mBounces++ > MAX_RAY_BOUNCES )
	{
		return ColourRGB();
	}
		
	float distance = 0;
	Shape* shape = NearestCollision( ray, distance );
	
	ColourRGB c( 0, 0, 0 );
	if( shape != 0 )
	{
		c = shape->GetColourFromRay( ray );
	}

	return c;
}

Shape* Scene::NearestCollision( Ray& ray, float& distance )
{
	Shape* nearest_shape = 0;
	float nearest_distance = 0;
	for( int i = 0; i < mNumShapes; ++i )
	{
		float temp_distance = 0;
		if( mShapes[i]->IsHitByRay( ray, temp_distance ) )
		{
			if( nearest_shape == 0 || temp_distance < nearest_distance )
			{
				nearest_shape = mShapes[i];
				nearest_distance = temp_distance;
			}
		}
	}

	distance = nearest_distance;
	return nearest_shape;
}
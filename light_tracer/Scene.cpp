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

	//The SCENE DATA!
	mShapes[mNumShapes++] = new Sphere( Vector3( 0.0f,	0.0f,	5.0f ),	1.0f );
	mShapes[mNumShapes++] = new Sphere( Vector3( 0.1f, -0.05f,	3.0f ),	0.2f );
	mShapes[mNumShapes++] = new Sphere( Vector3( 1.0f,	1.0f,	7.0f ),	1.0f );

	mShapes[mNumShapes++] = new Triangle( Vector3( -10, -2, 0 ), Vector3( 10, -2, 10.f ), Vector3( 10.f, -2, 0 ) );
	mShapes[mNumShapes++] = new Triangle( Vector3( -10, -2, 0 ), Vector3( -10, -2, 10.f ), Vector3( 10.f, -2, 10 ) );
	
	for( int i = 0; i < 300; ++i )
	{
		float x = ((float)rand() / RAND_MAX) / 4.0f;
		float y = ((float)rand() / RAND_MAX) / 4.0f;
		float z = ((float)rand() / RAND_MAX) / 4.0f;
		mPointLights[mNumPointLights++] = new PointLight( Vector3( 1.0f + x,	1.0f + y, z ), ColourRGB( 1.0f/300.0f, 1.0f/300.0f, 1.0f/300.0f ) );
	}

	for( int i = 0; i < 300; ++i )
	{
		float x = ((float)rand() / RAND_MAX) / 3.0f;
		float y = ((float)rand() / RAND_MAX) / 3.0f;
		float z = ((float)rand() / RAND_MAX) / 3.0f;
		mPointLights[mNumPointLights++] = new PointLight( Vector3( -0.5f + x,	-1.5f + y, z ), ColourRGB( 0, 0, 1.0f/300.0f ) );
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
/*
 *  Scene.h
 *  light_tracer
 *
 *  Created by Peter Andrews on 10/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "Sphere.h"
#include "Triangle.h"
#include "PointLight.h"

class Scene
{
public:
	static void Init();
	static Scene* Instance() { return mInstance;};
	
	ColourRGB GetColour( Ray& ray );
	Shape* Scene::NearestCollision( Ray& ray, float& distance );

	PointLight** GetPointLights( int& num_lights ) const { num_lights = mNumPointLights; return mPointLights; };

private:
	Scene();

	static Scene* mInstance;
	
	Shape** mShapes;
	int mNumShapes;

	PointLight** mPointLights;
	int mNumPointLights;
};
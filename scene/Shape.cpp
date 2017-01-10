/*
 *  Shape.cpp
 *  light_tracer
 *
 *  Created by Peter Andrews on 09/12/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "Shape.h"
#include "PointLight.h"
#include "Scene.h"
#include <algorithm>
using namespace std;

/*ColourRGB Shape::CalculateLighting( Vector3 point, Vector3 normal, Vector3 view ) const
{
	//get all the lights, loop over them and test for vision

	int num_lights = 0;
	PointLight** point_lights = Scene::Instance()->GetPointLights( num_lights );
	
	ColourRGB c( 0.05f, 0.05f, 0.05f );

	for( int i = 0; i < num_lights; ++i )
	{
		Vector3 to_light = point_lights[i]->GetPosition() - point;
		float light_distance = ~(to_light);
		to_light = !to_light;

		float collision_distance = 0.0f;
		Ray light_ray( point, to_light, 0 );
		Shape* collider = Scene::Instance()->NearestCollision( light_ray, collision_distance );

		if( collider == 0 || light_distance < collision_distance )
		{
			float diffuse = OrenNayerDiffuse( !to_light, !view, !normal, 0.7f );
			if( diffuse > 1.01f )
			{
				int bp = 0;
				bp++;
			}

			c = c + point_lights[i]->GetColour() * diffuse;
		}
	}

	return c;
}

float Shape::OrenNayerDiffuse( Vector3 light, Vector3 view, Vector3 normal, float roughness ) const
{
	//http://www.gamasutra.com/view/feature/2860/implementing_modular_hlsl_with_.php?page=3
	//http://pastebin.com/jk6Tc6mx

	Vector3 rev_view;
	rev_view.mX = view.mX;
	rev_view.mY = view.mY;
	rev_view.mZ = view.mZ;

	float v_dot_n = rev_view * normal;
	float l_dot_n = light * normal;
	float theta_r = acos( v_dot_n );
	float theta_i = acos( l_dot_n );
	float gamma = !(rev_view - (normal*v_dot_n)) * !(light - (normal*l_dot_n)) ;
		
	float sigma2 = roughness * roughness;
	float A = 1.0f - 0.5f * sigma2 / ( sigma2 + 0.33f );
	float B = 0.45f * sigma2 / ( sigma2 + 0.09f );
	
	float alpha = max( theta_i, theta_r );
	float beta = min( theta_i, theta_r );

	float C = sin( alpha ) * tan( beta );
	float final = A + B * max(0.0f, gamma) * C;
	
	float output = max( 0.0f, l_dot_n ) * final;

	return output;
}*/
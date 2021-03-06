
#include "shared.h"


struct Pixel
{
	float4 mColour;
};

RWStructuredBuffer<Pixel> BufferOut : register(u0);

struct Ray
{
	float3 mPoint;
	float3 mDirection;
};



#define INVALID_ID -1
struct RayCastResult
{
	int mNearestSphereID;
	int mNearestTriID;
	float mCollisionDistance;
	float3 mCollisionNormal;
};





void writeToPixel(int x, int y, float3 colour)
{
	uint index = x + y * gWidth;
    BufferOut[index].mColour = float4( colour, 1 );
}



bool SphereIsHitByRay(Sphere sphere, Ray ray, inout float distance, inout float3 normal)
{
	float3 to_sphere = sphere.mPosition - ray.mPoint;
	float dot_p = dot(to_sphere, ray.mDirection);

	if (dot_p < 0.0f)
	{
		return false;
	}

	float3 point_on_ray = ray.mPoint + (ray.mDirection * dot_p);
	float3 sphere_to_ray = point_on_ray - sphere.mPosition;
	float dist_to_ray = length(sphere_to_ray);

	bool sphere_hit = dist_to_ray < sphere.mRadius;

	if (sphere_hit)
	{
		//calc distance to surface
		float length_back = sqrt(sphere.mRadius * sphere.mRadius - dist_to_ray * dist_to_ray);
		float3 point_on_surface = point_on_ray - (ray.mDirection * length_back);

		distance = length(point_on_surface - ray.mPoint);
		normal = normalize(point_on_surface - sphere.mPosition);
	}

	return sphere_hit;
}

bool TriIsHitByRay(Tri tri, Ray ray, inout float distance, inout float3 normal)
{
	//https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
	//if this doesn't work out, try https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/ray-triangle-intersection-geometric-solution
	//mayve this? http://www.lighthouse3d.com/tutorials/maths/ray-triangle-intersection/
	float3 v0v1 = tri.mV1 - tri.mV0;
	float3 v0v2 = tri.mV2 - tri.mV0;
	float3 pvec = cross(ray.mDirection, v0v2);
	float det = dot(v0v1, pvec);
	
	if (det < 0.001f)
	{
		return false; //do we really need to do this?
	}

	float inv_det = 1 / det;

	float3 tvec = ray.mPoint - tri.mV0;
	float u = dot(tvec, pvec) * inv_det;
	if (u < 0 || u > 1)
	{
		return false;
	}

	float3 qvec = cross(tvec, v0v1);
	float v = dot(ray.mDirection, qvec) * inv_det;
	if (v < 0 || u + v > 1)
	{
		return false;
	}

	distance = dot(v0v2, qvec) * inv_det;	
	if (distance < 0)
	{
		return false;
	}

	normal = float3(0, 1, 0);  //ACTUALLY GET THE NORMAL HERE
	return true;
}


RayCastResult FindNearestCollision(Ray ray)
{
	//temp out vars
	float distance = 0;
	float3 normal = float3(0, 0, 0);

	float near_dist = 999999.f;
	float3 near_normal = float3(0, 0, 0);
	int near_sphere_id = INVALID_ID;
	for (int i = 0; i < gNumSpheres; ++i)
	{
		if (SphereIsHitByRay(gSpheres[i], ray, distance, normal))
		{
			if (distance < near_dist)
			{
				near_sphere_id = i;
				near_dist = distance;
				near_normal = normal;
			}
		}
	}

	int near_tri_id = INVALID_ID;
	for (i = 0; i < gNumTris; ++i)
	{
		if (TriIsHitByRay(gTris[i], ray, distance, normal))
		{
			if (distance < near_dist)
			{
				near_tri_id = i;
				near_sphere_id = INVALID_ID;
				near_dist = distance;
				near_normal = normal;
			}
		}
	}

	//DO CONES AND SHIT HERE


	RayCastResult res;
	res.mNearestSphereID = near_sphere_id;
	res.mNearestTriID = near_tri_id;
	res.mCollisionDistance = near_dist;
	res.mCollisionNormal = near_normal;

	return res;
}

float OrenNayerDiffuse( float3 light, float3 view, float3 normal, float roughness )
{
	//http://www.gamasutra.com/view/feature/2860/implementing_modular_hlsl_with_.php?page=3
	//http://pastebin.com/jk6Tc6mx

	float3 rev_view;
	rev_view.x = view.x;
	rev_view.y = view.y;
	rev_view.z = view.z;

	float v_dot_n = dot(rev_view, normal);
	float l_dot_n = dot(light, normal);
	float theta_r = acos( v_dot_n );
	float theta_i = acos( l_dot_n );
	float gamma = dot( normalize(  rev_view - (normal * v_dot_n) ), normalize( light - (normal * l_dot_n)  ) );
		
	float sigma2 = roughness * roughness;
	float A = 1.0f - 0.5f * sigma2 / ( sigma2 + 0.33f );
	float B = 0.45f * sigma2 / ( sigma2 + 0.09f );
	
	float alpha = max( theta_i, theta_r );
	float beta = min( theta_i, theta_r );

	float C = sin( alpha ) * tan( beta );
	float final = A + B * max(0.0f, gamma) * C;
	
	float output = max( 0.0f, l_dot_n ) * final;

	return output;
}

float Phong(float3 light, float3 view, float3 normal)
{
	float l_dot_n = dot(light, normal);

	float3 r = reflect(light, normal);
	float cos_a = saturate(dot(view, r));
	float spec = pow(cos_a, 50);
	return l_dot_n + spec;
}

float BlinnPhong(float3 light, float3 view, float3 normal)
{
	float l_dot_n = dot(light, normal);

	float3 half_vec = normalize(light - view);
	float n_dot_h = saturate(dot(normal, half_vec));
	float spec = pow(n_dot_h, 90);

	return l_dot_n + spec;
}

float3 CalculateLighting( float3 position, float3 normal, float3 view)
{
	//get all the lights, loop over them and test for vision	
	float3 c = float3(0.05, 0.05, 0.05);

	for( int i = 0; i < gNumLights; ++i )
	{
		float3 to_light = gLights[i].mPosition - position;
		float light_distance = length(to_light);
		to_light = normalize(to_light);

		Ray ray;
		ray.mPoint = position;
		ray.mDirection = to_light;
		RayCastResult res = FindNearestCollision(ray);

		if((res.mNearestSphereID == INVALID_ID && res.mNearestTriID == INVALID_ID) || light_distance < res.mCollisionDistance )
		{
			float diffuse = BlinnPhong(to_light, view, normal);
			float intensity = 1 / (light_distance);

			c = c + gLights[i].mColour * diffuse * intensity;
		}
	}
	
	return c;
}


float3 LightPassThroughGlow(float max_dist, Ray ray)
{
	float3 contribution = 0;
	for (int i = 0; i < gNumLights; ++i)
	{
		float3 to_light = gLights[i].mPosition - ray.mPoint;
		if (length(to_light) < max_dist)
		{
			float dot_p = dot(to_light, ray.mDirection);

			if (dot_p > 0.0f)
			{
				float3 point_on_ray = ray.mPoint + (ray.mDirection * dot_p);
				float3 light_to_ray = point_on_ray - gLights[i].mPosition;
				float dist_to_ray = length(light_to_ray);

				if (dist_to_ray < 0.1f)
				{
					float f = lerp(1, 0, dist_to_ray * 10);
					contribution += gLights[i].mColour * f;
				}
			}
		}
	}
	return contribution;
}

float3 GetColourFromRay(Ray ray)
{
	//Get the lighting from the nearest primitive
	float3 pixel = float3(0, 0, 0);

	float bounce_scale = 1.0f;
	const int max_bounces = 3;
	int bounces = -1;
	while (bounce_scale > 0.0f && bounces < max_bounces)
	{
		float3 ray_colour = float3(0, 0, 0);

		float next_bounce_scale = 0;
		//Find the nearest primitive
		RayCastResult res = FindNearestCollision(ray);

		if (res.mNearestSphereID != INVALID_ID || res.mNearestTriID != INVALID_ID)
		{
			float3 point_on_surface = ray.mPoint + ray.mDirection * res.mCollisionDistance;
			ray_colour = CalculateLighting(point_on_surface, res.mCollisionNormal, ray.mDirection);
			next_bounce_scale = bounce_scale * 0.35f;

			ray.mPoint = point_on_surface;
			ray.mDirection = reflect(ray.mDirection, res.mCollisionNormal);
		}
		else
		{
			float x = lerp(0.05, 1.0, ray.mDirection.y);
			ray_colour = float3(x, x, x);
			next_bounce_scale = 0.0f;
		}
		ray_colour += LightPassThroughGlow(res.mCollisionDistance, ray);


		pixel += ray_colour * bounce_scale;		
		bounce_scale = next_bounce_scale;
		
		bounces++;
	}

	return pixel;
}


float3 ToneMap(float3 pixel)
{
	pixel = pixel * pixel;

	// Linear to sRGB.
	//pixel = sqrt(pixel);
	
	// Add additional nonlinearities to shadows and highlights.
	//pixel = smoothstep(0.0, 1.0, pixel);
	//return pixel;
	
	
	return pixel / (pixel + 1);
}


[numthreads(16, 16, 1)]
void CSMain( uint3 dispatchThreadID : SV_DispatchThreadID )
{
	//Setup ray for this pixel
	float3 direction = float3( 0, 0, 1 );
	float aspect = (float)gWidth / gHeight;
	float fov = 0.75f;
			
	float3 ray_dir = direction;
	ray_dir.x = fov * aspect * ((float)dispatchThreadID.x - ( (float)gWidth / 2.0f ))/ (float)gWidth;
	ray_dir.y = -1.0f * fov * ((float)dispatchThreadID.y - ( (float)gHeight / 2.0f ))/ (float)gHeight;
	ray_dir = normalize(ray_dir);

	float3x3 gCameraOrientation;
	gCameraOrientation._m00 = cam_orientation_00; gCameraOrientation._m01 = cam_orientation_01; gCameraOrientation._m02 = cam_orientation_02;
	gCameraOrientation._m10 = cam_orientation_10; gCameraOrientation._m11 = cam_orientation_11; gCameraOrientation._m12 = cam_orientation_12;
	gCameraOrientation._m20 = cam_orientation_20; gCameraOrientation._m21 = cam_orientation_21; gCameraOrientation._m22 = cam_orientation_22;

	ray_dir = mul( gCameraOrientation, ray_dir);

	Ray ray;
	ray.mPoint = float3( gCameraPosition_x, gCameraPosition_y, gCameraPosition_z );
	ray.mDirection = ray_dir;

	float3 pixel = GetColourFromRay(ray);
	
	pixel = ToneMap(pixel);

	writeToPixel( dispatchThreadID.x, dispatchThreadID.y, pixel );
}

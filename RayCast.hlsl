struct Pixel
{
    float4 mColour;
};

struct Ray
{
	float3 mPoint;
	float3 mDirection;
};

struct PointLight
{
	float3 mPosition;
	float3 mColour;
};

struct Sphere
{
	float3 mPosition;
	float mRadius;
};

struct Tri
{
	float3 mV0;
	float3 mV1;
	float3 mV2;
};


#define INVALID_ID -1
struct RayCastResult
{
	int mNearestSphereID;
	int mNearestTriID;
	float mCollisionDistance;
};



cbuffer ConstantBufferResolution : register(cb0)
{
    int gWidth;
    int gHeight;
};

cbuffer ConstantBufferCamera : register(cb1)
{
    float3 gCameraPosition;
	float cam_orientation_00, cam_orientation_01, cam_orientation_02;
	float cam_orientation_10, cam_orientation_11, cam_orientation_12;
	float cam_orientation_20, cam_orientation_21, cam_orientation_22; 
};

cbuffer ConstantBufferPrimitives : register(cb2)
{
	int gNumLights;
	int gNumSpheres;
	int gNumTris;
};

StructuredBuffer<PointLight> LightBuffer : register(t0);
StructuredBuffer<Sphere> SphereBuffer : register(t1);
StructuredBuffer<Tri> TriBuffer : register(t2);


RWStructuredBuffer<Pixel> BufferOut : register(u0);




void writeToPixel(int x, int y, float3 colour)
{
	uint index = x + y * gWidth;
    BufferOut[index].mColour = float4( colour, 1 );
}



bool SphereIsHitByRay(Sphere sphere, Ray ray, inout float distance)
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
	}

	return sphere_hit;
}

bool TriIsHitByRay(Tri tri, Ray ray, inout float distance)
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

	return true;
}


RayCastResult FindNearestCollision(Ray ray)
{
	float near_dist = 999999.f;
	int near_sphere_id = INVALID_ID;
	for (int i = 0; i < gNumSpheres; ++i)
	{
		float distance = 0;
		if (SphereIsHitByRay(SphereBuffer[i], ray, distance))
		{
			if (distance < near_dist)
			{
				near_sphere_id = i;
				near_dist = distance;
			}
		}
	}

	int near_tri_id = INVALID_ID;
	for (i = 0; i < gNumTris; ++i)
	{
		float distance = 0;
		if (TriIsHitByRay(TriBuffer[i], ray, distance))
		{
			if (distance < near_dist)
			{
				near_tri_id = i;
				near_sphere_id = INVALID_ID;
				near_dist = distance;
			}
		}
	}

	//DO CONES AND SHIT HERE


	RayCastResult res;
	res.mNearestSphereID = near_sphere_id;
	res.mNearestTriID = near_tri_id;
	res.mCollisionDistance = near_dist;

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

float3 CalculateLighting( float3 position, float3 normal, float3 view )
{
	//get all the lights, loop over them and test for vision	
	float3 c = float3(0.1, 0.1, 0.1);

	for( int i = 0; i < gNumLights; ++i )
	{
		float3 to_light = LightBuffer[i].mPosition - position;
		float light_distance = length(to_light);
		to_light = normalize(to_light);

		Ray ray;
		ray.mPoint = position;
		ray.mDirection = to_light;
		RayCastResult res = FindNearestCollision(ray);

		if(res.mNearestSphereID == INVALID_ID || light_distance < res.mCollisionDistance )
		{
			float diffuse = OrenNayerDiffuse( to_light, view, normal, 0.7f );

			c = c + LightBuffer[i].mColour * diffuse; //QUADRATIC FALLOF HERE?
		}
	}

	return c;
}

float3 SphereGetColourFromRay( Sphere sphere, Ray ray, float distance )
{
	//TOOD, OPTIMIZE THIS BASED ON KNOWING distance already
	float3 to_sphere = sphere.mPosition - ray.mPoint;
	float dot_p = dot( to_sphere, ray.mDirection );
	float3 point_on_ray = ray.mPoint + (ray.mDirection * dot_p);
	float3 sphere_to_ray = point_on_ray - sphere.mPosition;

	float length_sphere_to_ray = length(sphere_to_ray);

	float length_back = sqrt( sphere.mRadius * sphere.mRadius - length_sphere_to_ray * length_sphere_to_ray );
	float3 point_on_surface = point_on_ray - (ray.mDirection * length_back);
	float3 normal = normalize(point_on_surface - sphere.mPosition);

	return CalculateLighting( point_on_surface, normal, ray.mDirection );
}

float3 TriGetColourFromRay(Tri tri, Ray ray, float distance)
{
	float3 point_on_surface = ray.mPoint + ray.mDirection * distance;
	float3 normal = float3(0, 1, 0);// normalize(cross(tri.mV1 - tri.mV0, tri.mV2 - tri.mV0));

	return CalculateLighting(point_on_surface, normal, ray.mDirection);
}


float3 LightPassThroughGlow(Ray ray)
{
	float3 contribution = 0;
	for (int i = 0; i < gNumLights; ++i)
	{
		float3 to_light = LightBuffer[i].mPosition - ray.mPoint;
		float dot_p = dot(to_light, ray.mDirection);

		if (dot_p > 0.0f)
		{
			float3 point_on_ray = ray.mPoint + (ray.mDirection * dot_p);
			float3 light_to_ray = point_on_ray - LightBuffer[i].mPosition;
			float dist_to_ray = length(light_to_ray);

			if (dist_to_ray < 0.1f)
			{
				float f = lerp(1,0, dist_to_ray*10);
				contribution += LightBuffer[i].mColour * f;
			}
		}
	}
	return contribution;
}


[numthreads(32, 32, 1)]
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
	ray.mPoint = gCameraPosition;
	ray.mDirection = ray_dir;


	//Find the nearest primitive
	RayCastResult res = FindNearestCollision(ray);


	//Get the lighting from the nearest primitive
	float3 pixel = float3(0, 0, 0);
	if (res.mNearestSphereID != INVALID_ID)
	{
		pixel = SphereGetColourFromRay(SphereBuffer[res.mNearestSphereID], ray, res.mCollisionDistance);
	}
	else if(res.mNearestTriID != INVALID_ID)
	{
		pixel = TriGetColourFromRay(TriBuffer[res.mNearestTriID], ray, res.mCollisionDistance);
	}
	else
	{
		float x = ray.mDirection.y;
		pixel = float3(x, x, x);
	}
	pixel += LightPassThroughGlow(ray);


	writeToPixel( dispatchThreadID.x, dispatchThreadID.y, pixel );
}

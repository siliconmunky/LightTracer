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
	int gNumSpheres;
};

StructuredBuffer<Sphere> SphereBuffer : register(t0);


RWStructuredBuffer<Pixel> BufferOut : register(u0);




void writeToPixel(int x, int y, float3 colour)
{
	uint index = x + y * gWidth;
    BufferOut[index].mColour = float4( colour, 1 );
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

	//int num_lights = 0;
	//PointLight** point_lights = Scene::Instance()->GetPointLights( num_lights );
	
	float3 c = float3(0.1, 0.105, 0.12);
	
	PointLight light;
	light.mPosition = float3( 1, 2, 3 );
	light.mColour = float3( 0.65f, 0.63, 0.6f );

	//for( int i = 0; i < num_lights; ++i )
	{
		float3 to_light = light.mPosition - position;
		float light_distance = length(to_light);
		to_light = normalize(to_light);

		//float collision_distance = 0.0f;
		//Ray light_ray( point, to_light, 0 );
		//Shape* collider = Scene::Instance()->NearestCollision( light_ray, collision_distance );

		//if( collider == 0 || light_distance < collision_distance )
		{
			float diffuse = OrenNayerDiffuse( to_light, view, normal, 0.7f );

			c = c + light.mColour * diffuse;
		}
	}

	return c;
}





bool SphereIsHitByRay( Sphere sphere, Ray ray, inout float distance )
{	
	float3 to_sphere = sphere.mPosition - ray.mPoint;
	float dot_p = dot( to_sphere, ray.mDirection );
	
	if( dot_p < 0.0f )
	{
		return false;
	}
	
	float3 point_on_ray = ray.mPoint + (ray.mDirection * dot_p);
	float3 sphere_to_ray = point_on_ray - sphere.mPosition;
	float dist_to_ray = length(sphere_to_ray);
	
	bool sphere_hit = dist_to_ray < sphere.mRadius;
	
	if( sphere_hit )
	{
		//calc distance to surface
		float length_sphere_to_ray = length(sphere_to_ray);
		float length_back = sqrt( sphere.mRadius * sphere.mRadius - length_sphere_to_ray * length_sphere_to_ray );
		float3 point_on_surface = point_on_ray - (ray.mDirection * length_back);

		distance = length(point_on_ray - ray.mPoint);
	}
	
	return sphere_hit;
}

float3 SphereGetColourFromRay( Sphere sphere, Ray ray )
{	
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

[numthreads(32, 32, 1)]
void CSMain( uint3 dispatchThreadID : SV_DispatchThreadID )
{
	float3 pixel = float3(0,0,0);

	float3 cam_point = gCameraPosition;
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
	//ray_dir = normalize(ray_dir);


	Ray ray;
	ray.mPoint = cam_point;
	ray.mDirection = ray_dir;

	float near_dist = 999999.f;
	int near_sphere_id = -1;

	
	for( int i = 0; i < gNumSpheres; ++i )
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
	if (near_sphere_id >= 0)
	{
		pixel = SphereGetColourFromRay(SphereBuffer[near_sphere_id], ray);
	}
	else
	{
		float x = ray.mDirection.y;
		pixel = float3(x, x, x);
	}
	
	writeToPixel( dispatchThreadID.x, dispatchThreadID.y, pixel );
}

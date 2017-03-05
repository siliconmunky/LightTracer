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

#define MAX_LIGHTS 32
#define MAX_SPHERES 32
#define MAX_TRIS 32

cbuffer SharedCB : register(b0)
{
    int gWidth;
    int gHeight;
	
	float gCameraPosition_x;
	float gCameraPosition_y;
	float gCameraPosition_z;

	float cam_orientation_00, cam_orientation_01, cam_orientation_02;
	float cam_orientation_10, cam_orientation_11, cam_orientation_12;
	float cam_orientation_20, cam_orientation_21, cam_orientation_22;
	
	
	int gNumLights;
	int gNumSpheres;
	int gNumTris;
	
	
	float gNoiseOffsetX;
	float gNoiseOffsetY;
	float gTime;

	PointLight gLights[MAX_LIGHTS];
	Sphere gSpheres[MAX_SPHERES];
	Tri gTris[MAX_TRIS];
};


#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <string>

#include "ImageBuffer.h"
#include "Vector3.h"
#include "Ray.h"
#include "Sphere.h"
#include "Scene.h"

using std::ifstream;
using std::cout;
using std::string;
using std::endl;

int main (int argc, char * const argv[])
{
	if( argc != 3 ) //we need 2 arguments
	{
		cout << "light_tracer.exe scene_description_file.txt output_image.bmp\n";
		cout << "scene_description_file.txt example:\n";
		cout << "	width 400\n";
		cout << "	height 300\n";
		cout << "	num_spheres 3\n";
		cout << "	0.0 0.0 5.0 1.0\n";
		cout << "	0.1 -0.05	3.0 0.2\n";
		cout << "	1.0	1.0	7.0 1.0\n";
		cout << "	num_triangles 2\n";
		cout << "	-10 -2 0 10 -2 10 10 -2 0\n";
		cout << "	-10 -2 0 -10 -2 10 10 -2 10\n";
		cout << "	num_point_lights 2\n";
		cout << "	1.0 1.0 0 1.0 1.0 1.0 0.2 50\n";
		cout << "	-0.5 -1.5 0 0 0 1.0 0.2 50\n";
		return 1;
	}

	Scene::Init();

	ifstream indata; // indata is like cin
	indata.open( argv[1] ); // opens the file

	if( !indata )
	{ // file couldn't be opened
      return 1;
	}
	
	int w_in_pix;
	string temp;
	indata >> temp;
	indata >> w_in_pix;
		
	int h_in_pix;
	indata >> temp;
	indata >> h_in_pix;

	const float aspect = (float)w_in_pix / h_in_pix;
	const float fov = 0.75f;

	int num_spheres;
	indata >> temp;
	indata >> num_spheres;
	for( int i = 0; i < num_spheres; ++i )
	{
		Vector3 pos;
		float radius;
		indata >> pos.mX;
		indata >> pos.mY;
		indata >> pos.mZ;
		indata >> radius;		
		Scene::Instance()->AddSphere( pos, radius );
	}

	int num_triangles;
	indata >> temp;
	indata >> num_triangles;
	for( int i = 0; i < num_triangles; ++i )
	{
		Vector3 p1;
		Vector3 p2;
		Vector3 p3;
		indata >> p1.mX;
		indata >> p1.mY;
		indata >> p1.mZ;
		indata >> p2.mX;
		indata >> p2.mY;
		indata >> p2.mZ;
		indata >> p3.mX;
		indata >> p3.mY;
		indata >> p3.mZ;		
		Scene::Instance()->AddTriangle( p1, p2, p3 );
	}
	
	int num_point_lights;
	indata >> temp;
	indata >> num_point_lights;
	for( int i = 0; i < num_point_lights; ++i )
	{
		Vector3 pos;
		ColourRGB colour;
		float size;
		int taps;
		indata >> pos.mX;
		indata >> pos.mY;
		indata >> pos.mZ;
		indata >> colour.mR;
		indata >> colour.mG;
		indata >> colour.mB;
		indata >> size;		
		indata >> taps;	
		Scene::Instance()->AddPointLight( pos, colour, size, taps );
	}

	indata.close();


	ImageBuffer* image = new ImageBuffer( w_in_pix, h_in_pix );
	
	Vector3 cam_point( 0, 0, 0 );
	Vector3 direction( 0, 0, 1 );

	srand( (unsigned int)time(0) );
	
	for( int y = 0; y < h_in_pix; y++ )
    {
		for( int x = 0; x < w_in_pix; x++ )
		{
			Vector3 ray_dir = direction;
			ray_dir.mX = fov * aspect * ((float)x - ( (float)w_in_pix / 2.0f ))/ (float)w_in_pix; //deal with aspect ratio and fov here
			ray_dir.mY = -1.0f * fov * ((float)y - ( (float)h_in_pix / 2.0f ))/ (float)h_in_pix;
			ray_dir = !ray_dir;
			Ray ray( cam_point, ray_dir, 0 );
			
			image->SetPixel( x, y, Scene::Instance()->GetColour( ray ) );
		}
    } 
	
	
	image->WriteToBMP( argv[2] );
	
	
    return 0;
}

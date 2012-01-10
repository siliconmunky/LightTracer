#include <iostream>
#include <stdlib.h>
#include <time.h>

#include "ImageBuffer.h"
#include "Vector3.h"
#include "Ray.h"
#include "Sphere.h"
#include "Scene.h"

int main (int argc, char * const argv[])
{    
	const int w_in_pix = 1280;
	const int h_in_pix = 720;
	const float aspect = (float)w_in_pix / h_in_pix;
	const float fov = 0.75f;
	
	ImageBuffer* image = new ImageBuffer( w_in_pix, h_in_pix );
	
	Vector3 cam_point( 0, 0, 0 );
	Vector3 direction( 0, 0, 1 );
	Scene::Init();

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
	
	
	image->WriteToBMP( "C:\\Users\\smunky\\Desktop\\test.bmp" );
	
	
    return 0;
}

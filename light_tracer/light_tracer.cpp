// light_tracer.cpp : Defines the entry point for the console application.
//

#include "ImageBuffer.h"
#include "Vector3.h"
#include "Ray.h"



int main(int argc, char* argv[])
{
	int w_in_pix = 1280;
	int h_in_pix = 720;

	ImageBuffer* image = new ImageBuffer( w_in_pix, h_in_pix );
	
	Vector3 cam_point( 0, 0, 0 );
	Vector3 direction( 0, 0, 1 );

	for( int y = 0; y < h_in_pix; y++ )
    {
          for( int x = 0; x < w_in_pix; x++ )
          {
			  Ray ray( cam_point, direction, 0 );

			  //Cast a ray and see what we hit
			  ColourRGB c;


			  image->SetPixel( x, y, c );
          }
    } 


	image->WriteToBMP( "C:\\Users\\smunky\\Desktop\\test.bmp" );

	return 0;
}


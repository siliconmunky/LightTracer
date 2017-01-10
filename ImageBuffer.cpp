#include "ImageBuffer.h"

#include <fstream>
#include <iostream>
#include <algorithm>


using namespace std;




ImageBuffer::ImageBuffer( int width, int height )
	: mWidth( width )
	, mHeight( height )
{
	mBuffer = new ColourRGB[width * height];
	memset( mBuffer, 0, width * height * sizeof(ColourRGB) );
}


ImageBuffer::~ImageBuffer(void)
{
}

void ImageBuffer::WriteToBMP( const char* filename ) const
{
	struct BMPHeader
	{
			short BM;
			long size_of_file;
			long reserve;
			long offset_of_pixel_data;
			long size_of_header;
			long width;
			long height;
			short num_of_colour_plane;
			short num_of_bit_per_pix;
			long compression;
			long size_of_pix_data;
			long h_resolution;
			long v_resolution;
			long num_of_colour_in_palette;
			long important_colours;
	 };

	int bitmap_bytes = 3 * mWidth * mHeight;	

	BMPHeader header;
		
	header.BM = 0x4d42;
    header.size_of_file = 54/*sizeof(BMPHeader)*/ + bitmap_bytes;
    header.reserve = 0000; 
    header.offset_of_pixel_data = 54;
    header.size_of_header = 40;
    header.width = mWidth;
    header.height = mHeight;
    header.num_of_colour_plane = 1;
    header.num_of_bit_per_pix = 24;
    header.compression = 0;
    header.size_of_pix_data = bitmap_bytes;
    header.h_resolution = 2835;
    header.v_resolution = 2835;
    header.num_of_colour_in_palette = 0;
    header.important_colours = 0;


	// write BMP Header
	ofstream file;
    file.open( filename, ios::out | ios::trunc | ios::binary );
	file.write( (char*)(&header.BM), 2 );
	file.write( (char*)(&header.size_of_file), 4 );
	file.write( (char*)(&header.reserve), 4 );
	file.write( (char*)(&header.offset_of_pixel_data), 4 );
	file.write( (char*)(&header.size_of_header), 4 );
	file.write( (char*)(&header.width), 4 );
	file.write( (char*)(&header.height), 4 );
	file.write( (char*)(&header.num_of_colour_plane), 2 );
	file.write( (char*)(&header.num_of_bit_per_pix), 2 );
	file.write( (char*)(&header.compression), 4 );
	file.write( (char*)(&header.size_of_pix_data), 4 );
	file.write( (char*)(&header.h_resolution), 4 );
	file.write( (char*)(&header.v_resolution), 4 );
	file.write( (char*)(&header.num_of_colour_in_palette), 4 );
	file.write( (char*)(&header.important_colours), 4 );



    ////////////////////////////////////////////////////////////////////////////////////
    // write BMP data //////////////////////////////////////////////////////////////////
    for( int y = mHeight - 1; y > -1; y-- )
    {
          for( int x = 0; x != mWidth; x++ )
          {
			  char r = (char)(min( 1.0f, mBuffer[ Index( x, y ) ].mR ) * 255.0f);
			  char g = (char)(min( 1.0f, mBuffer[ Index( x, y ) ].mG ) * 255.0f);
			  char b = (char)(min( 1.0f, mBuffer[ Index( x, y ) ].mB ) * 255.0f);
			  
			  file.write( &b, 1 );
			  file.write( &g, 1 );
              file.write( &r, 1 );
          }
    } 
}
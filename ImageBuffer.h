#pragma once
#include <string.h>

#include "ColourRGB.h"

class ImageBuffer
{
public:
	ImageBuffer( int width, int height );
	~ImageBuffer(void);

	void WriteToBMP( const char* filename ) const;

	void SetPixel( int column, int row, const ColourRGB& value )
	{
		mBuffer[Index( column, row )] = value;
	}

	ColourRGB* GetBuffer() { return mBuffer; };

private:

	int Index( int column, int row ) const
	{
		return column + row * mWidth;
	}

	int mWidth;
	int mHeight;

	ColourRGB* mBuffer;
};



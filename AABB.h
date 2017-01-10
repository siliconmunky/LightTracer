#ifndef AABB_H
#define AABB_H

struct AABB
{
    float x0, x1;
    float y0, y1;
	float z0, z1;

	AABB( float fx0, float fx1, float fy0, float fy1, float fz0, float fz1 )
		:x0(fx0),
		x1(fx1),
		y0(fy0),
		y1(fy1),
		z0(fz0),
		z1(fz1){}

};

#endif 
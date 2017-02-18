#include "Vector3.h"


Vector3 Vector3::X_AXIS = Vector3(1, 0, 0);
Vector3 Vector3::Y_AXIS = Vector3(0, 1, 0);
Vector3 Vector3::Z_AXIS = Vector3(0, 0, 1);
Vector3 Vector3::ORIGIN = Vector3(0, 0, 0);

Vector3::Vector3( float x, float y, float z )
{
	mX = x;
	mY = y;
	mZ = z;
}


Vector3::~Vector3(void)
{
}

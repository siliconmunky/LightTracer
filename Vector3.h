#pragma once

#include <math.h>

class Vector3
{
public:
	static Vector3 X_AXIS;
	static Vector3 Y_AXIS;
	static Vector3 Z_AXIS;
	static Vector3 ORIGIN;

	Vector3() {};
	Vector3( const Vector3& r ) { mX = r.mX; mY = r.mY; mZ = r.mZ; };
	Vector3( float x, float y, float z );

	~Vector3(void);
	
	//Add
	Vector3 operator +( Vector3 r ) const
	{
		Vector3 v;
		v.mX = mX + r.mX;
		v.mY = mY + r.mY;
		v.mZ = mZ + r.mZ;
		return v;
	}
	
	//Subtract
	Vector3 operator -( Vector3 r ) const
	{
		Vector3 v;
		v.mX = mX - r.mX;
		v.mY = mY - r.mY;
		v.mZ = mZ - r.mZ;
		return v;
	}
	
	//Length
	float operator ~() const
	{
		float temp = mX * mX + mY * mY + mZ * mZ;
		return sqrtf( temp );
	}
	
	//Normalize
	Vector3 operator!() const
	{
		float length = ~*this;
		Vector3 v;
		v.mX = mX / length;
		v.mY = mY / length;
		v.mZ = mZ / length;
		return v;
	}
	
	//Dot product
	float operator *( Vector3 r ) const
	{
		return mX * r.mX + mY * r.mY + mZ * r.mZ;
	}
	
	//Scale
	Vector3 operator *( float scale ) const
	{
		Vector3 v;
		v.mX = mX * scale;
		v.mY = mY * scale;
		v.mZ = mZ * scale;
		return v;
	}	
	
	//Cross product
	Vector3 operator %( Vector3 r ) const
	{
		Vector3 v;
		v.mX = mY * r.mZ - mZ * r.mY;
		v.mY = mZ * r.mX - mX * r.mZ;
		v.mZ = mX * r.mY - mY * r.mX;
		return v;
	}

	float mX;
	float mY;
	float mZ;
};


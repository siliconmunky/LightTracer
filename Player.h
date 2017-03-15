#pragma once

#include "aabb.h"
#include "Vector3.h"
#include "Matrix3x3.h"

class Player
{
public:
	static Player* Instance;

	Player();
	~Player();
	
	void StrafeLeftRight(float dist);
	void MoveForwardBack(float dist);
	void RotateLeftRight(float radians);
	void RotateUpDown(float radians);
	
	void Update(float dt);

private:
	Vector3	mPosition;
	Vector3	mView;
	Vector3	mUpVector;
	
	Vector3 mMoveFB;
	Vector3 mStrafeLR;
	Vector3 mVelocity;
};
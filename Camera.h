#pragma once

#include "aabb.h"
#include "Vector3.h"
#include "Matrix3x3.h"

class Camera
{
public:
	static Camera* Instance;

	Camera();
	~Camera();
	
	Vector3* GetPosition();
	Vector3* GetView();
	Vector3* GetUpVector();
	Matrix3x3 GetViewMatrix();

	void StrafeLeftRight(float dist);
	void MoveForwardBack(float dist);
	void MoveUpDown(float dist);
	void RotateLeftRight(float radians);
	void RotateUpDown(float radians);

	void SetPosition(const Vector3& position);
	void SetLookDir(const Vector3& lookDir);
	void SetUpDir(const Vector3& upDir);
	

private:
	Vector3	mPosition;
	Vector3	mView;
	Vector3	mUpVector;

    bool bViewMatrixNeedsUpdating;

};
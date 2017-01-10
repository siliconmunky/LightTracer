#pragma once

#include "aabb.h"
#include "Vector3.h"

enum eCameraModes
{
	CAM_MODE_ORIENTATION,
	CAM_MODE_MATRIX_STORAGE,
	CAM_MODE_MAX
};

class Camera
{
public:
	Camera();
	~Camera();
	
	Vector3* GetPosition();
	Vector3* GetView();
	Vector3* GetUpVector();
	//D3DXMATRIXA16 GetViewMatrix();

	void StrafeLeftRight(float dist);
	void MoveForwardBack(float dist);
	void MoveUpDown(float dist);
	void RotateLeftRight(float radians);
	void RotateUpDown(float radians);
	//void LookAt(Vector3& pointToLookAt);

	void SetPosition(const Vector3& position);
	void SetLookDir(const Vector3& lookDir);
	void SetUpDir(const Vector3& upDir);
//	void SetViewMatrix(const D3DXMATRIXA16& viewMatrix);
//	void SetCameraMode(const eCameraModes camMode); //lol almost commode
	
	//bool TestRectToFrustum(const AABB& box, LPDIRECT3DDEVICE9 lpD3DDevice, bool drawBoundingBox);

private:
	Vector3	mPosition;
	Vector3	mView;
	Vector3	mUpVector;
	//D3DXMATRIXA16 mViewMatrix;

	//frustum planes
	/*Plane3d mLeftPlane;
	Plane3d mRightPlane;
	Plane3d mTopPlane;
	Plane3d mBottomPlane;
	Plane3d mNearPlane;
	Plane3d mFarPlane;*/

	eCameraModes cameraMode;
    bool bViewMatrixNeedsUpdating;
	bool bFrustumNeedUpdating;

	enum ePlaneClassification
	{
		k_PLANE_FRONT = 0,
		k_PLANE_BACK,
		k_PLANE_INTERSECT
	};

	//int PlaneClassify( const AABB& box, const Plane3d& plane );
	//void UpdateFrustumPlanes( bool normalizePlanes, LPDIRECT3DDEVICE9 lpD3DDevice );
};
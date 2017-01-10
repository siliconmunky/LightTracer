
#include "Camera.h"


//-------------------------------------------------------------------------------------------
Camera::Camera()
//-------------------------------------------------------------------------------------------
{
	mPosition = Vector3(0,0,0);
	mView = Vector3( 0, 0, 1);
	mUpVector = Vector3(0,1,0);
	bFrustumNeedUpdating = true;
	bViewMatrixNeedsUpdating = true;
	cameraMode = CAM_MODE_ORIENTATION;
}


//-------------------------------------------------------------------------------------------
Camera::~Camera()
//-------------------------------------------------------------------------------------------
{

}


//-------------------------------------------------------------------------------------------
Vector3* Camera::GetPosition()
//-------------------------------------------------------------------------------------------
{
	return &mPosition;
}


//-------------------------------------------------------------------------------------------
Vector3* Camera::GetView() 
//-------------------------------------------------------------------------------------------
{
	return &mView;
}


//-------------------------------------------------------------------------------------------
Vector3* Camera::GetUpVector() 
//-------------------------------------------------------------------------------------------
{
	return &mUpVector;
}

/*//-------------------------------------------------------------------------------------------
D3DXMATRIXA16 Camera::GetViewMatrix()
//-------------------------------------------------------------------------------------------
{
	if(bViewMatrixNeedsUpdating)
	{
		switch(cameraMode)
		{
			case CAM_MODE_ORIENTATION:
				D3DXMatrixLookAtLH( &mViewMatrix, GetPosition(), &(*GetPosition() + *GetView()), GetUpVector() );
				bViewMatrixNeedsUpdating = false;
				break;
			case CAM_MODE_MATRIX_STORAGE:
				//do nothing we're already holding the correct view matrix
				break;
		}
	}
	return mViewMatrix;
}*/

//-------------------------------------------------------------------------------------------
void Camera::StrafeLeftRight(float dist)
//-------------------------------------------------------------------------------------------
{
	Vector3 sideVec = !(mUpVector % mView);
	
	mPosition = mPosition + sideVec*dist;
	bFrustumNeedUpdating = true;
	bViewMatrixNeedsUpdating = true;
}

//-------------------------------------------------------------------------------------------
void Camera::MoveForwardBack(float dist)
//-------------------------------------------------------------------------------------------
{
	mPosition = mPosition + mView*dist;
	bFrustumNeedUpdating = true;
	bViewMatrixNeedsUpdating = true;
}

//-------------------------------------------------------------------------------------------
void Camera::MoveUpDown(float dist)
//-------------------------------------------------------------------------------------------
{
	mPosition = mPosition + mUpVector*dist;
	bFrustumNeedUpdating = true;
	bViewMatrixNeedsUpdating = true;
}

//-------------------------------------------------------------------------------------------
void Camera::RotateLeftRight(float radians)
//-------------------------------------------------------------------------------------------
{
    float mat[] = { cos(radians), 0, sin(radians), 
				0, 1, 0, 
				-sin(radians), 0, cos(radians)}; 

	mView.mX = mat[0]*mView.mX + mat[1]*mView.mY + mat[2]*mView.mZ;
	mView.mY = mat[3]*mView.mX + mat[4]*mView.mY + mat[5]*mView.mZ; 
	mView.mZ = mat[6]*mView.mX + mat[7]*mView.mY + mat[8]*mView.mZ; 
	mView = !mView;

	mUpVector.mX = mat[0]*mUpVector.mX + mat[1]*mUpVector.mY + mat[2]*mUpVector.mZ;
	mUpVector.mY = mat[3]*mUpVector.mX + mat[4]*mUpVector.mY + mat[5]*mUpVector.mZ; 
	mUpVector.mZ = mat[6]*mUpVector.mX + mat[7]*mUpVector.mY + mat[8]*mUpVector.mZ; 
	mUpVector = !mUpVector;

	bFrustumNeedUpdating = true;
	bViewMatrixNeedsUpdating = true;
}


//-------------------------------------------------------------------------------------------
void Camera::RotateUpDown(float radians)
//-------------------------------------------------------------------------------------------
{
	Vector3 sideVec = !(mUpVector % mView);

	float x = sideVec.mX;
	float y = sideVec.mY;
	float z = sideVec.mZ;

    float mat[] = { (1+(1-cos(radians))*(x*x-1)), (-z*sin(radians)+(1-cos(radians))*x*y), (y*sin(radians)+(1-cos(radians))*x*z), 
				(z*sin(radians)+(1-cos(radians))*x*y), (1 + (1-cos(radians))*(y*y-1)), (-x*sin(radians)+(1-cos(radians))*y*z), 
				(-y*sin(radians)+(1-cos(radians))*x*z), (x*sin(radians)+(1-cos(radians))*y*z), (1 + (1-cos(radians))*(z*z-1))}; 

	mView.mX = mat[0]*mView.mX + mat[1]*mView.mY + mat[2]*mView.mZ;
	mView.mY = mat[3]*mView.mX + mat[4]*mView.mY + mat[5]*mView.mZ; 
	mView.mZ = mat[6]*mView.mX + mat[7]*mView.mY + mat[8]*mView.mZ; 
	mView = !mView;

	mUpVector.mX = mat[0]*mUpVector.mX + mat[1]*mUpVector.mY + mat[2]*mUpVector.mZ;
	mUpVector.mY = mat[3]*mUpVector.mX + mat[4]*mUpVector.mY + mat[5]*mUpVector.mZ; 
	mUpVector.mZ = mat[6]*mUpVector.mX + mat[7]*mUpVector.mY + mat[8]*mUpVector.mZ; 
	mUpVector = !mUpVector;

	bFrustumNeedUpdating = true;
	bViewMatrixNeedsUpdating = true;
}

/*//-------------------------------------------------------------------------------------------
void Camera::LookAt(D3DXVECTOR3& pointToLookAt)
//-------------------------------------------------------------------------------------------
{
	mView = pointToLookAt - mPosition;
	D3DXVec3Normalize(&mView, &mView);
	bFrustumNeedUpdating = true;
	bViewMatrixNeedsUpdating = true;
}*/

//-------------------------------------------------------------------------------------------
void Camera::SetPosition(const Vector3& position)
//-------------------------------------------------------------------------------------------
{
	mPosition = position;
	bFrustumNeedUpdating = true;
	bViewMatrixNeedsUpdating = true;
}

//-------------------------------------------------------------------------------------------
void Camera::SetLookDir(const Vector3& lookDir)
//-------------------------------------------------------------------------------------------
{
	mView = !lookDir;

	bFrustumNeedUpdating = true;
	bViewMatrixNeedsUpdating = true;
}

//-------------------------------------------------------------------------------------------
void Camera::SetUpDir(const Vector3& upDir)
//-------------------------------------------------------------------------------------------
{
	mUpVector = !upDir;

	bFrustumNeedUpdating = true;
	bViewMatrixNeedsUpdating = true;
}

/*//-------------------------------------------------------------------------------------------
void Camera::SetViewMatrix(const D3DXMATRIXA16& viewMatrix)
//-------------------------------------------------------------------------------------------
{
	//http://www.mvps.org/vbdx/articles/billboards/ for extracting
	SetPosition(D3DXVECTOR3(viewMatrix._41, viewMatrix._42, viewMatrix._43));
	SetLookDir(D3DXVECTOR3(viewMatrix._13, viewMatrix._23, viewMatrix._33));
	SetUpDir(D3DXVECTOR3(viewMatrix._12, viewMatrix._22, viewMatrix._32));
	bViewMatrixNeedsUpdating = false;
	mViewMatrix = viewMatrix;
}*/


/*//-------------------------------------------------------------------------------------------
void Camera::SetCameraMode(const eCameraModes camMode)
//-------------------------------------------------------------------------------------------
{
	cameraMode = camMode;
}*/

/*//-------------------------------------------------------------------------------------------
bool Camera::TestRectToFrustum(const AABB& box, LPDIRECT3DDEVICE9 lpD3DDevice, bool drawBoundingBox)
//-------------------------------------------------------------------------------------------
{
	//debugging drawing of the bounding box
	if(drawBoundingBox)
	{
		DWGGFXDrawLineStartAndEnd( D3DXVECTOR3(box.x0, box.y0, box.z0), D3DXVECTOR3(box.x0, box.y1, box.z0), RGB(0, 255, 0) );
		DWGGFXDrawLineStartAndEnd( D3DXVECTOR3(box.x0, box.y0, box.z0), D3DXVECTOR3(box.x1, box.y0, box.z0), RGB(0, 255, 0) );
		DWGGFXDrawLineStartAndEnd( D3DXVECTOR3(box.x0, box.y1, box.z0), D3DXVECTOR3(box.x1, box.y1, box.z0), RGB(0, 255, 0) );
		DWGGFXDrawLineStartAndEnd( D3DXVECTOR3(box.x1, box.y0, box.z0), D3DXVECTOR3(box.x1, box.y1, box.z0), RGB(0, 255, 0) );
		DWGGFXDrawLineStartAndEnd( D3DXVECTOR3(box.x0, box.y0, box.z1), D3DXVECTOR3(box.x0, box.y1, box.z1), RGB(0, 255, 0) );
		DWGGFXDrawLineStartAndEnd( D3DXVECTOR3(box.x0, box.y0, box.z1), D3DXVECTOR3(box.x1, box.y0, box.z1), RGB(0, 255, 0) );
		DWGGFXDrawLineStartAndEnd( D3DXVECTOR3(box.x0, box.y1, box.z1), D3DXVECTOR3(box.x1, box.y1, box.z1), RGB(0, 255, 0) );
		DWGGFXDrawLineStartAndEnd( D3DXVECTOR3(box.x1, box.y0, box.z1), D3DXVECTOR3(box.x1, box.y1, box.z1), RGB(0, 255, 0) );
		DWGGFXDrawLineStartAndEnd( D3DXVECTOR3(box.x0, box.y0, box.z0), D3DXVECTOR3(box.x0, box.y0, box.z1), RGB(0, 255, 0) );
		DWGGFXDrawLineStartAndEnd( D3DXVECTOR3(box.x0, box.y1, box.z0), D3DXVECTOR3(box.x0, box.y1, box.z1), RGB(0, 255, 0) );
		DWGGFXDrawLineStartAndEnd( D3DXVECTOR3(box.x1, box.y0, box.z0), D3DXVECTOR3(box.x1, box.y0, box.z1), RGB(0, 255, 0) );
		DWGGFXDrawLineStartAndEnd( D3DXVECTOR3(box.x1, box.y1, box.z0), D3DXVECTOR3(box.x1, box.y1, box.z1), RGB(0, 255, 0) );
	}

	if(bFrustumNeedUpdating)
	{
		UpdateFrustumPlanes( true, lpD3DDevice );
	}

	if((PlaneClassify(box, mLeftPlane)
			== k_PLANE_BACK)
		|| (PlaneClassify(box, mRightPlane)
			== k_PLANE_BACK)
		|| (PlaneClassify(box, mTopPlane)
			== k_PLANE_BACK)
		|| (PlaneClassify(box, mBottomPlane)
			== k_PLANE_BACK)
		|| (PlaneClassify(box, mNearPlane)
			== k_PLANE_BACK)
		|| (PlaneClassify(box, mFarPlane)
			== k_PLANE_BACK))
	{
		return false;
	}
	
	return true;
}*/

/*//-------------------------------------------------------------------------------------------
int Camera::PlaneClassify( const AABB& box, const Plane3d& plane )
//-------------------------------------------------------------------------------------------
{
	//this function used for frustum testing

	D3DXVECTOR3 minPoint, maxPoint;

	// get the closest and farthest points on the rect
	if(plane.normal.x > 0.0f)
	{
		minPoint.x = box.x0;
		maxPoint.x = box.x1;
	}
	else
	{
		minPoint.x = box.x1;
		maxPoint.x = box.x0;
	}
	
	if(plane.normal.y > 0.0f)
	{
		minPoint.y = box.y0;
		maxPoint.y = box.y1;
	}
	else
	{
		minPoint.y = box.y1;
		maxPoint.y = box.y0;
	}

	if(plane.normal.z > 0.0f)
	{
		minPoint.z = box.z0;
		maxPoint.z = box.z1;
	}
	else
	{
		minPoint.z = box.z1;
		maxPoint.z = box.z0;
	}

	//calc the signed distancee from the plane to both points
	float dmin = plane.SignedDistance(minPoint);
	float dmax = plane.SignedDistance(maxPoint);
	
	//the rect intersects the plane if the values have oposite signs
	if(dmin * dmax < 0.0f)
	{
		return k_PLANE_INTERSECT;
	}
	else if(dmin > 0)
	{
		return k_PLANE_FRONT;
	}
	
	return k_PLANE_BACK;
}*/

/*//-------------------------------------------------------------------------------------------
void Camera::UpdateFrustumPlanes( bool normalizePlanes, LPDIRECT3DDEVICE9 lpD3DDevice )
//-------------------------------------------------------------------------------------------
{
	bFrustumNeedUpdating = false;

	D3DXMATRIX matrix, viewMatrix, projMatrix;
	lpD3DDevice->GetTransform( D3DTS_VIEW, &viewMatrix );
	lpD3DDevice->GetTransform( D3DTS_PROJECTION, &projMatrix );
	D3DXMatrixMultiply( &matrix, &viewMatrix, &projMatrix );

	mLeftPlane.normal.x = matrix._14 + matrix._11;
	mLeftPlane.normal.y = matrix._24 + matrix._21;
	mLeftPlane.normal.z = matrix._34 + matrix._31;
	mLeftPlane.distance = matrix._44 + matrix._41;

	mRightPlane.normal.x = matrix._14 - matrix._11;
	mRightPlane.normal.y = matrix._24 - matrix._21;
	mRightPlane.normal.z = matrix._34 - matrix._31;
	mRightPlane.distance = matrix._44 - matrix._41;

	mTopPlane.normal.x = matrix._14 - matrix._12;
	mTopPlane.normal.y = matrix._24 - matrix._22;
	mTopPlane.normal.z = matrix._34 - matrix._32;
	mTopPlane.distance = matrix._44 - matrix._42;

	mBottomPlane.normal.x = matrix._14 + matrix._12;
	mBottomPlane.normal.y = matrix._24 + matrix._22;
	mBottomPlane.normal.z = matrix._34 + matrix._32;
	mBottomPlane.distance = matrix._44 + matrix._42;

	mNearPlane.normal.x = matrix._13;
	mNearPlane.normal.y = matrix._23;
	mNearPlane.normal.z = matrix._33;
	mNearPlane.distance = matrix._43;

	mFarPlane.normal.x = matrix._14 - matrix._13;
	mFarPlane.normal.y = matrix._24 - matrix._23;
	mFarPlane.normal.z = matrix._34 - matrix._33;
	mFarPlane.distance = matrix._44 - matrix._43;

	if(normalizePlanes)
	{
		mLeftPlane.Normalize();
		mRightPlane.Normalize();
		mTopPlane.Normalize();
		mBottomPlane.Normalize();
		mNearPlane.Normalize();
		mFarPlane.Normalize();
	}
}*/


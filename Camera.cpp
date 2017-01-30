
#include "Camera.h"
#include "Log.h"

Camera* Camera::Instance = NULL;

//-------------------------------------------------------------------------------------------
Camera::Camera()
//-------------------------------------------------------------------------------------------
{
	Instance = this;

	mPosition = Vector3(0,0,0);
	mView = Vector3( 0, 0, 1);
	mUpVector = Vector3(0,1,0);
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

//-------------------------------------------------------------------------------------------
Matrix3x3 Camera::GetViewMatrix()
//-------------------------------------------------------------------------------------------
{
	Matrix3x3 mat;

	Vector3 xaxis = mUpVector % mView;
	Vector3 yaxis = mView % xaxis;

	mat.m00 = xaxis.mX; mat.m01 = yaxis.mX; mat.m02 = mView.mX;
	mat.m10 = xaxis.mY; mat.m11 = yaxis.mY; mat.m12 = mView.mY;
	mat.m20 = xaxis.mZ; mat.m21 = yaxis.mZ; mat.m22 = mView.mZ;

	return mat;
}

//-------------------------------------------------------------------------------------------
void Camera::StrafeLeftRight(float dist)
//-------------------------------------------------------------------------------------------
{
	Vector3 sideVec = !(mUpVector % mView);
	
	mPosition = mPosition + sideVec*dist;
}

//-------------------------------------------------------------------------------------------
void Camera::MoveForwardBack(float dist)
//-------------------------------------------------------------------------------------------
{
	mPosition = mPosition + mView*dist;
}

//-------------------------------------------------------------------------------------------
void Camera::MoveUpDown(float dist)
//-------------------------------------------------------------------------------------------
{
	mPosition = mPosition + mUpVector*dist;
}

//-------------------------------------------------------------------------------------------
void Camera::RotateLeftRight(float radians)
//-------------------------------------------------------------------------------------------
{
    float mat[] = { cosf(radians), 0, sinf(radians), 
				0, 1, 0, 
				-sinf(radians), 0, cosf(radians)}; 

	mView.mX = mat[0]*mView.mX + mat[1]*mView.mY + mat[2]*mView.mZ;
	mView.mY = mat[3]*mView.mX + mat[4]*mView.mY + mat[5]*mView.mZ; 
	mView.mZ = mat[6]*mView.mX + mat[7]*mView.mY + mat[8]*mView.mZ; 
	mView = !mView;

	mUpVector.mX = mat[0]*mUpVector.mX + mat[1]*mUpVector.mY + mat[2]*mUpVector.mZ;
	mUpVector.mY = mat[3]*mUpVector.mX + mat[4]*mUpVector.mY + mat[5]*mUpVector.mZ; 
	mUpVector.mZ = mat[6]*mUpVector.mX + mat[7]*mUpVector.mY + mat[8]*mUpVector.mZ; 
	mUpVector = !mUpVector;
}


//-------------------------------------------------------------------------------------------
void Camera::RotateUpDown(float radians)
//-------------------------------------------------------------------------------------------
{
	Vector3 sideVec = !(mUpVector % mView);

	float x = sideVec.mX;
	float y = sideVec.mY;
	float z = sideVec.mZ;

    float mat[] = { (1.0f+(1.0f-cosf(radians))*(x*x-1.0f)), (-z*sinf(radians)+(1.0f-cosf(radians))*x*y), (y*sinf(radians)+(1.0f-cosf(radians))*x*z), 
				(z*sinf(radians)+(1.0f-cosf(radians))*x*y), (1.0f + (1.0f-cosf(radians))*(y*y-1.0f)), (-x*sinf(radians)+(1.0f-cosf(radians))*y*z), 
				(-y*sinf(radians)+(1.0f-cosf(radians))*x*z), (x*sinf(radians)+(1.0f-cosf(radians))*y*z), (1.0f + (1.0f-cosf(radians))*(z*z-1.0f))}; 

	mView.mX = mat[0]*mView.mX + mat[1]*mView.mY + mat[2]*mView.mZ;
	mView.mY = mat[3]*mView.mX + mat[4]*mView.mY + mat[5]*mView.mZ; 
	mView.mZ = mat[6]*mView.mX + mat[7]*mView.mY + mat[8]*mView.mZ; 
	mView = !mView;

	mUpVector.mX = mat[0]*mUpVector.mX + mat[1]*mUpVector.mY + mat[2]*mUpVector.mZ;
	mUpVector.mY = mat[3]*mUpVector.mX + mat[4]*mUpVector.mY + mat[5]*mUpVector.mZ; 
	mUpVector.mZ = mat[6]*mUpVector.mX + mat[7]*mUpVector.mY + mat[8]*mUpVector.mZ; 
	mUpVector = !mUpVector;
}

//-------------------------------------------------------------------------------------------
void Camera::SetPosition(const Vector3& position)
//-------------------------------------------------------------------------------------------
{
	mPosition = position;
}

//-------------------------------------------------------------------------------------------
void Camera::SetLookDir(const Vector3& lookDir)
//-------------------------------------------------------------------------------------------
{
	mView = !lookDir;
}

//-------------------------------------------------------------------------------------------
void Camera::SetUpDir(const Vector3& upDir)
//-------------------------------------------------------------------------------------------
{
	mUpVector = !upDir;
}




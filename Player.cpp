
#include "Player.h"
#include "Log.h"

#include "Game.h"

Player* Player::Instance = NULL;

//-------------------------------------------------------------------------------------------
Player::Player()
//-------------------------------------------------------------------------------------------
{
	Instance = this;

	mPosition = Vector3(5,0,0);
	mView = Vector3( 0, 0, 1);
	mUpVector = Vector3(0,1,0);

	mStrafeLR = Vector3::ORIGIN;
	mMoveFB = Vector3::ORIGIN;
	mVelocity = Vector3::ORIGIN;
}


//-------------------------------------------------------------------------------------------
Player::~Player()
//-------------------------------------------------------------------------------------------
{

}


//-------------------------------------------------------------------------------------------
void Player::StrafeLeftRight(float dist)
//-------------------------------------------------------------------------------------------
{
	Vector3 sideVec = !(mUpVector % mView);
	mStrafeLR = sideVec*dist;
}

//-------------------------------------------------------------------------------------------
void Player::MoveForwardBack(float dist)
//-------------------------------------------------------------------------------------------
{
	Vector3 forward = mView;
	forward.mY = 0;
	forward = !forward;

	mMoveFB = forward*dist;
}


//-------------------------------------------------------------------------------------------
void Player::RotateLeftRight(float radians)
//-------------------------------------------------------------------------------------------
{
    float mat[] = { cosf(radians), 0, sinf(radians), 
				0, 1, 0, 
				-sinf(radians), 0, cosf(radians)}; 

	mView.mX = mat[0]*mView.mX + mat[1]*mView.mY + mat[2]*mView.mZ;
	mView.mY = mat[3]*mView.mX + mat[4]*mView.mY + mat[5]*mView.mZ; 
	mView.mZ = mat[6]*mView.mX + mat[7]*mView.mY + mat[8]*mView.mZ; 
	mView = !mView;
	
	Vector3 side_vec = !(Vector3::Y_AXIS % mView);
	mUpVector = !(mView % side_vec);
}


//-------------------------------------------------------------------------------------------
void Player::RotateUpDown(float radians)
//-------------------------------------------------------------------------------------------
{
	Vector3 old_mView = mView;
	Vector3 old_mUpVector = mUpVector;

	Vector3 side_vec = !(mUpVector % mView);

	float x = side_vec.mX;
	float y = side_vec.mY;
	float z = side_vec.mZ;

	float mat[] = { (1.0f + (1.0f - cosf(radians))*(x*x - 1.0f)), (-z*sinf(radians) + (1.0f - cosf(radians))*x*y), (y*sinf(radians) + (1.0f - cosf(radians))*x*z),
				(z*sinf(radians) + (1.0f - cosf(radians))*x*y), (1.0f + (1.0f - cosf(radians))*(y*y - 1.0f)), (-x*sinf(radians) + (1.0f - cosf(radians))*y*z),
				(-y*sinf(radians) + (1.0f - cosf(radians))*x*z), (x*sinf(radians) + (1.0f - cosf(radians))*y*z), (1.0f + (1.0f - cosf(radians))*(z*z - 1.0f)) };

	mView.mX = mat[0] * mView.mX + mat[1] * mView.mY + mat[2] * mView.mZ;
	mView.mY = mat[3] * mView.mX + mat[4] * mView.mY + mat[5] * mView.mZ;
	mView.mZ = mat[6] * mView.mX + mat[7] * mView.mY + mat[8] * mView.mZ;
	mView = !mView;

	mUpVector = !(mView % side_vec);

	if (mUpVector.mY < 0.1f)
	{
		mUpVector = old_mUpVector;
		mView = old_mView;
	}
}

//-------------------------------------------------------------------------------------------
void Player::Update(float dt)
//-------------------------------------------------------------------------------------------
{
	static float accel = 100.0f;
	Vector3 motion_vec = mStrafeLR + mMoveFB;
	if (~motion_vec > 0.0f)
	{
		motion_vec = !motion_vec;
	}
	mVelocity = mVelocity + motion_vec*accel*dt;

	static float motion_speed = 2.2f;
	mPosition = mPosition + (mVelocity)*motion_speed*dt;

	mStrafeLR = Vector3::ORIGIN;
	mMoveFB = Vector3::ORIGIN;

	static float decel = 100.0f;
	mVelocity = mVelocity - mVelocity * ~mVelocity*decel*dt;


	Camera& cam = Game::Instance->mCam;

	const float height = 1.5f;
	cam.SetPosition(mPosition + Vector3(0, height, 0));
	cam.SetLookDir(mView);
	cam.SetUpDir(mUpVector);
}




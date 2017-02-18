#include "Game.h"




#include "scene\scene.h"


#include "Render.h"
#include "Vector3.h"

Game* Game::Instance = NULL;



float m0 = 0;
float m1 = 0;
float m2 = 0;
float m3 = 0;


Game::Game()
{
	Instance = this;
	
	mCam.SetPosition(Vector3(0, 4, -5));
	mCam.SetLookDir(Vector3(0, 0, 1));
	mCam.SetUpDir(Vector3(0, 1, 0));

	mGameStartTime = std::clock();
	mPrevFrameTime = std::clock();


	AfxHandle hum_fx = mAudio.StartSound(AFX_BG_HUM, Vector3(0,0,0) );


	Scene::Init(); //IS TIHS USED AT THE MOMENT?
}


Game::~Game()
{
}

void Game::GameLoop()
{
	std::clock_t now = std::clock();
	float dt = float(now - mPrevFrameTime) / CLOCKS_PER_SEC;
	mPrevFrameTime = now;


	HandleInput(dt);

	mAudio.Update(dt);

	if (mAudio.IsValidHandle(mHelloAFX))
	{
		m0 = 0.3f + mAudio.GetFFTData(mHelloAFX);
		m1 = m0;
		m2 = m0;
		m3 = m0;
	}
	else
	{
		m0 = m1 = m2 = m3 = 0.3f;
	}


	mRender.DoFrame();
}


float Game::GetTime()
{
	std::clock_t now = std::clock();
	float elapsed = float(now - mGameStartTime) / CLOCKS_PER_SEC;
	return elapsed;
}


void Game::HandleInput(float dt)
{
	static float move_sens = 2.5f;
	if( mInput.IsKeyDown(IX_KEY_W) )
	{
		mCam.MoveForwardBack(move_sens*dt);
	}
	if (mInput.IsKeyDown(IX_KEY_S))
	{
		mCam.MoveForwardBack(-move_sens*dt);
	}

	if (mInput.IsKeyDown(IX_KEY_A))
	{
		mCam.StrafeLeftRight(-move_sens*dt);
	}
	if (mInput.IsKeyDown(IX_KEY_D))
	{
		mCam.StrafeLeftRight(move_sens*dt);
	}

	static bool first_down = true;
	if (mInput.IsKeyDown(IX_KEY_SPACE) && first_down)
	{
		mHelloAFX = mAudio.StartSound(AFX_HELLO, Vector3(0, 0, 0), true);
		first_down = false;
	}
	else if (!mInput.IsKeyDown(IX_KEY_SPACE))
	{
		first_down = true;
	}

	static float x_sensitivity = 0.0006f;
	static float y_sensitivity = 0.0006f;
	mCam.RotateLeftRight(mInput.GetInputMotion(IX_MOUSE_X) * x_sensitivity);
	mCam.RotateUpDown(-mInput.GetInputMotion(IX_MOUSE_Y) * x_sensitivity);
}
#include "Game.h"




#include "scene\scene.h"


#include "Render.h"
#include "Vector3.h"

Game* Game::Instance = NULL;

Game::Game()
{
	Instance = this;
	
	mCam.SetPosition(Vector3(0, 0, 0));
	mCam.SetLookDir(Vector3(0, 0, 1));
	mCam.SetUpDir(Vector3(0, 1, 0));

	mGameStartTime = std::clock();
	mPrevFrameTime = std::clock();

	

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

	static float x_sensitivity = 0.0006f;
	static float y_sensitivity = 0.0006f;
	mCam.RotateLeftRight(mInput.GetInputMotion(IX_MOUSE_X) * x_sensitivity);
	mCam.RotateUpDown(-mInput.GetInputMotion(IX_MOUSE_Y) * x_sensitivity);
}
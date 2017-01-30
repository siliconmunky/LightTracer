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

	mPrevFrame = std::clock();

	Scene::Init(); //IS TIHS USED AT THE MOMENT?
}


Game::~Game()
{
}

void Game::GameLoop()
{
	std::clock_t now = std::clock();
	float dt = float(now - mPrevFrame) / CLOCKS_PER_SEC;
	mPrevFrame = now;


	HandleInput(dt);

	mRender.DoFrame();


}




void Game::HandleInput(float dt)
{	
	if( mInput.IsKeyDown(IX_KEY_W) )
	{
		mCam.MoveForwardBack(1.0f*dt);
	}
	if (mInput.IsKeyDown(IX_KEY_S))
	{
		mCam.MoveForwardBack(-1.0f*dt);
	}

	if (mInput.IsKeyDown(IX_KEY_A))
	{
		mCam.StrafeLeftRight(-1.0f*dt);
	}
	if (mInput.IsKeyDown(IX_KEY_D))
	{
		mCam.StrafeLeftRight(1.0f*dt);
	}

	static float x_sensitivity = 0.1f;
	static float y_sensitivity = 0.1f;
	mCam.RotateLeftRight(mInput.GetInputMotion(IX_MOUSE_X) * x_sensitivity*dt);
	mCam.RotateUpDown(-mInput.GetInputMotion(IX_MOUSE_Y) * x_sensitivity*dt);
}
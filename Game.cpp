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
	
	mGameStartTime = std::clock();
	mPrevFrameTime = std::clock();


	mHumAfx = mAudio.StartSound(AFX_BG_HUM, Vector3(0, 0, 0), true);


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


	CalculateFPS(dt);
	HandleInput(dt);

	mPlayer.Update(dt);

	mAudio.Update(dt);

	if (mAudio.IsValidHandle(mHelloAfx))
	{
		m0 = 0.3f + mAudio.GetFFTData(mHelloAfx) * 10.f;
		m1 = m0;
		m2 = m0;
		m3 = m0;
	}
	else
	{
		m0 = m1 = m2 = m3 = 0.3f;
	}


	mRender.DoFrame();


	mInput.EndFrame();
}

void Game::CalculateFPS(float dt)
{
	const int num_frames = 64;
	static float fps_avgs[num_frames];
	static int frame = 0;
	fps_avgs[frame] = 1.0f / dt;
	frame = (frame + 1) % num_frames;
	float fps_avg = 0.0f;
	for (int i = 0; i < num_frames; ++i)
	{
		fps_avg += fps_avgs[i];
	}
	fps_avg /= num_frames;
	if (frame == 0)
	{
		Log::Printf("FPS %f\n", fps_avg);
	}
}


float Game::GetTime()
{
	std::clock_t now = std::clock();
	float elapsed = float(now - mGameStartTime) / CLOCKS_PER_SEC;
	return elapsed;
}


void Game::HandleInput(float dt)
{
	if( mInput.IsKeyDown(IX_KEY_W) )
	{
		mPlayer.MoveForwardBack(1.0f);
	}
	if (mInput.IsKeyDown(IX_KEY_S))
	{
		mPlayer.MoveForwardBack(-1.0f);
	}

	if (mInput.IsKeyDown(IX_KEY_A))
	{
		mPlayer.StrafeLeftRight(-1.0f);
	}
	if (mInput.IsKeyDown(IX_KEY_D))
	{
		mPlayer.StrafeLeftRight(1.0f);
	}

	if (mInput.IsKeyJustDown(IX_KEY_SPACE))
	{
		mHelloAfx = mAudio.StartSound(AFX_HELLO, Vector3(0, 0, 0), true);
	}


	if (mInput.IsKeyJustDown(IX_KEY_B))
	{
		if (mAudio.IsValidHandle(mHumAfx))
		{
			mAudio.StopSound(mHumAfx);
		}
		else
		{
			mHumAfx = mAudio.StartSound(AFX_BG_HUM, Vector3(0, 0, 0), true);
		}
	}


	static float x_sensitivity = 0.0006f;
	static float y_sensitivity = 0.0006f;
	mPlayer.RotateLeftRight(mInput.GetInputMotion(IX_MOUSE_X) * x_sensitivity);
	mPlayer.RotateUpDown(-mInput.GetInputMotion(IX_MOUSE_Y) * x_sensitivity);
}
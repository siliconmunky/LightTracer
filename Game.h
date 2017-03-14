#pragma once

#include "BasicDefs.h"

#include "Input.h"
#include "Camera.h"
#include "Render.h"
#include "Audio.h"

#include <ctime>

class Game
{
public:
	static Game* Instance;

	Game();
	~Game();


	void GameLoop();

	float GetTime();

	
	Input mInput;
	Camera mCam;
	Render mRender;
	Audio mAudio;

private:
	std::clock_t mGameStartTime;
	std::clock_t mPrevFrameTime;

	
	void HandleInput(float dt);
	
	void CalculateFPS(float dt);

	AfxHandle mHelloAfx;
	AfxHandle mHumAfx;
};


#pragma once

#include "BasicDefs.h"

#include "Input.h"
#include "Camera.h"
#include "Render.h"

#include <ctime>

class Game
{
public:
	static Game* Instance;

	Game();
	~Game();


	void GameLoop();
	

	void HandleInput(float dt);

	Input mInput;
	Camera mCam;
	Render mRender;

private:
	std::clock_t mPrevFrame;
};


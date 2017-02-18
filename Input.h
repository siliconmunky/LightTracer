#pragma once

#include "BasicDefs.h"

enum IX_KEY
{
	IX_KEY_W = 'W',
	IX_KEY_A = 'A',
	IX_KEY_S = 'S',
	IX_KEY_D = 'D',
	IX_KEY_SPACE = ' ',

	NUM_IX_KEY = UCHAR_MAX,
};

enum IX_MOTION
{
	IX_MOUSE_X,
	IX_MOUSE_Y,

	NUM_IX_MOTION,
};

class Input
{
public:
	static Input* Instance;

	Input();
	~Input();

	void SetKeyState(IX_KEY key, bool down );
	bool IsKeyDown(IX_KEY key);


	void SetInputMotion(IX_MOTION input, float motion);
	float GetInputMotion(IX_MOTION input);

private:
	bool mKeyStates[NUM_IX_KEY];
	float mMotionStates[NUM_IX_MOTION];
};


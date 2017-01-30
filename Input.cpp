#include "Input.h"



Input* Input::Instance = NULL;

Input::Input()
{
	Instance = this;

	for (int i = 0; i < NUM_IX_KEY; i++)
	{
		mKeyStates[i] = false;
	}
	for (int i = 0; i < NUM_IX_MOTION; i++)
	{
		mMotionStates[i] = 0.0f;
	}
}


Input::~Input()
{
}


void Input::SetKeyState(IX_KEY key, bool down )
{
	mKeyStates[key] = down;
}

bool Input::IsKeyDown(IX_KEY key)
{
	return mKeyStates[key];
}



void Input::SetInputMotion(IX_MOTION input, float motion)
{
	mMotionStates[input] = motion;
}

float Input::GetInputMotion(IX_MOTION input)
{
	return mMotionStates[input];
}

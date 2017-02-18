#include "BasicDefs.h"

#include "Audio.h"

#include "Camera.h"

#include "fmod_errors.h"

/*int routing(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData)
{
	return Audio::Instance->Routing(outputBuffer, inputBuffer, nBufferFrames, streamTime, status, userData );
}*/

Audio* Audio::Instance = NULL;

Audio::Audio()
{
	Instance = this;
	
	FMOD_RESULT result;

	result = FMOD::System_Create(&mSystem);      // Create the main system object.
	if (result != FMOD_OK)
	{
		Log::Printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1);
	}

	result = mSystem->init(512, FMOD_INIT_NORMAL, 0);    // Initialize FMOD.
	if (result != FMOD_OK)
	{
		Log::Printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1);
	}


	
	for (int i = 0; i < NUM_TRACKED_FFT_GROUPS; ++i)
	{
		mCurrentMagnitude[i] = 0.2f;
	}



	LoadData();
}


Audio::~Audio()
{
}


void Audio::LoadData()
{
	FMOD_RESULT result;

	result = mSystem->createSound("Sound/hello.wav", FMOD_3D, 0, &mSound);
	if (result != FMOD_OK)
	{
		Log::Printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}


}

void Audio::StartSound()
{
	FMOD_RESULT result;
	/*result = mSound->setMode(FMOD_LOOP_NORMAL);
	if (result != FMOD_OK)
	{
		Log::Printf("FMOD error! setMode %s\n", FMOD_ErrorString(result));
	}*/

	result = mSystem->playSound(mSound, 0, false, &mChannel);

	FMOD_VECTOR sound_pos;
	sound_pos.x = 0;
	sound_pos.y = 0;
	sound_pos.z = 0;
	mChannel->set3DAttributes(&sound_pos, NULL);

}

void Audio::Update(float dt)
{
	Vector3* pos = Camera::Instance->GetPosition();
	FMOD_VECTOR listener_pos;
	listener_pos.x = pos->mX;
	listener_pos.y = pos->mY;
	listener_pos.z = pos->mZ;

	Vector3 fwd = *Camera::Instance->GetView();
	FMOD_VECTOR listener_forward;
	listener_forward.x = fwd.mX;
	listener_forward.y = fwd.mY;
	listener_forward.z = fwd.mZ;

	Vector3 up = *Camera::Instance->GetUpVector();
	FMOD_VECTOR listener_up;
	listener_up.x = up.mX;
	listener_up.y = up.mY;
	listener_up.z = up.mZ;
	

	FMOD_VECTOR listener_vel;
	listener_vel.x = 0;
	listener_vel.y = 0;
	listener_vel.z = 0;

	FMOD_RESULT result;
	result = mSystem->set3DListenerAttributes(0, &listener_pos, &listener_vel, &listener_forward, &listener_up);     // update 'ears'
	if (result != FMOD_OK)
	{
		Log::Printf("FMOD error! set3DListenerAttributes failed %s\n", FMOD_ErrorString(result));
	}

	mSystem->update();   // needed to update 3d engine, once per frame.
}




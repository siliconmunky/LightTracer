#pragma once


#define AUDIO_FX_TUPLE\
	AUDIO_FX_DATA( AFX_HELLO, "Sound/hello.wav", false )\
	AUDIO_FX_DATA( AFX_BG_HUM, "Sound/Ambience/ambience_background_hum.wav", true )\



#define AUDIO_FX_DATA( e, wav, loop ) e,
enum AUDIO_FX_DATA_ENUM
{
	AUDIO_FX_TUPLE
	MAX_AUDIO_FX_DATA_ENUM
};
#undef AUDIO_FX_DATA





#define NUM_TRACKED_FFT_GROUPS 4
#define MAX_ACTIVE_AFX 256


#include "Vector3.h"

#include "fmod.hpp"

struct AfxHandle
{
	AfxHandle() { mSlot = INVALID_HANDLE; };

	AfxHandle(char slot, char verifier)
	{
		mSlot = slot;
		mVerifier = verifier;
	}

	void Clear() { mSlot = INVALID_HANDLE; };
	bool IsValid() { return mSlot != INVALID_HANDLE; };

	char mSlot;
	char mVerifier;
};

struct ActiveAudio
{
	ActiveAudio()
	{
		mActive = false;
		mChannel = NULL;
		mDSP = NULL;
		mVerifier = 0;
	};

	void Clear()
	{
		mActive = false;
		mChannel = NULL;
		mDSP = NULL;
		mVerifier = 0;
	};

	FMOD::Channel* mChannel;
	FMOD::DSP* mDSP;
	char mVerifier;

	bool mActive;
};


class Audio
{
public:
	static Audio* Instance;

	Audio();
	~Audio();

	void Update(float dt);
	
	AfxHandle StartSound(AUDIO_FX_DATA_ENUM e, Vector3& pos, bool do_fft = false);
	void UpdateSoundPos(AfxHandle handle, Vector3& pos);
	void StopSound(AfxHandle& handle);

	float GetFFTData(AfxHandle handle);

	bool IsValidHandle(AfxHandle handle);

private:
	void LoadData();


	FMOD::System* mSystem;
	FMOD::Sound* mSoundsData[MAX_AUDIO_FX_DATA_ENUM];

	ActiveAudio mActiveAudio[MAX_ACTIVE_AFX];

};


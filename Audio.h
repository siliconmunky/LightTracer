#pragma once


#define MAXIMILIAN_RT_AUDIO
#include "../Maximilian/maximilian.h"
#include "../Maximilian/libs/maxim.h"
#include "../Maximilian/RtAudio.h"

#define NUM_TRACKED_FFT_GROUPS 4

class Audio
{
public:
	static Audio* Instance;

	Audio();
	~Audio();

	float mCurrentMagnitude[NUM_TRACKED_FFT_GROUPS];

	int Routing(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData);

private:
	void LoadData();
	
	void InternalPlay(double *output);

	RtAudio* mDac;


	maxiSample ambience_rain_outside; //We give our sample a name. It's called beats this time. We could have loads of them, but they have to have different names.
	maxiSample ambience_inside_fan;
	maxiSample hello;

	maxiDyn compressor; //this is a compressor
	maxiFFT myFFT;


};


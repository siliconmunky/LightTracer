#include "BasicDefs.h"

#include "Audio.h"


int routing(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData)
{
	return Audio::Instance->Routing(outputBuffer, inputBuffer, nBufferFrames, streamTime, status, userData );
}

Audio* Audio::Instance = NULL;

Audio::Audio()
{
	Instance = this;


	mDac = new RtAudio(RtAudio::WINDOWS_DS);
	if (mDac->getDeviceCount() < 1)
	{
		Log::Printf("No audio devices found!\n");
		return;
	}

	LoadData();

	RtAudio::StreamParameters parameters;
	parameters.deviceId = mDac->getDefaultOutputDevice();
	parameters.nChannels = maxiSettings::channels;
	parameters.firstChannel = 0;
	unsigned int sampleRate = maxiSettings::sampleRate;
	unsigned int bufferFrames = maxiSettings::bufferSize;
	//double data[maxiSettings::channels];
	vector<double> data(maxiSettings::channels, 0);

	try
	{
		mDac->openStream(&parameters, NULL, RTAUDIO_FLOAT64, sampleRate, &bufferFrames, &routing, (void *)&(data[0]));

		mDac->startStream();
	}
	catch (RtError& e)
	{
		Log::Printf("Error opening stream!\n %s\n", e.getMessage());
	}
}


Audio::~Audio()
{
	if (mDac->isStreamOpen())
	{
		mDac->closeStream();
	}
	mDac->stopStream();
}


void Audio::LoadData()
{
	/*if (!ambience_rain_outside.load("C:/dwg/github/LightTracer/Ambience/ambience_rain_outside.wav"))
	{
		Log::Printf("Failed to load sample\n");
	}
	if (!ambience_inside_fan.load("C:/dwg/github/LightTracer/Ambience/ambience_inside_fan.wav"))
	{
		Log::Printf("Failed to load sample\n");
	}*/
	if (!hello.load("Sound/hello.wav"))
	{
		Log::Printf("Failed to load sample\n");
	}
	//Log::Printf("Summary:\n%s\n", ambience_rain_outside.getSummary());
	//Log::Printf("Summary:\n%s\n", ambience_inside_fan.getSummary());

	/*compressor.setAttack(500);
	compressor.setRelease(100);
	compressor.setThreshold(0.05);
	compressor.setRatio(10);*/


	myFFT.setup(2* NUM_AUDIO_FFT_GROUPS, 16, 16);
}





void Audio::InternalPlay(double *output)
{
	static double a = 1;
	//double out = a * ambience_rain_outside.play() + (1 - a) * ambience_inside_fan.play();
	double out = a * hello.play();


	//output[0]=beats.play(1,0,44100);//linear interpolationplay with a frequency input, start point and end point. Useful for syncing.
	//output[0]=beats.play4(0.5,0,44100);//cubic interpolation play with a frequency input, start point and end point. Useful for syncing.

	//out = compressor.compress(out);

	if (myFFT.process(out))
	{
		for (int i = 0; i < NUM_AUDIO_FFT_GROUPS; i++)
		{
			mCurrentMagnitude[i] = myFFT.magnitudes[i];
		}
	}


	output[0] = out;
	output[1] = out;
}

int Audio::Routing(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData)
{
	double *buffer = (double *)outputBuffer;
	double *lastValues = (double *)userData;

	//	double currentTime = (double) streamTime; Might come in handy for control
	if (status)
	{
		Log::Printf("Stream underflow detected!\n");
	}

	// Write interleaved audio data.
	for (unsigned int i = 0; i < nBufferFrames; i++)
	{
		InternalPlay(lastValues);
		for (int j = 0; j < maxiSettings::channels; j++)
		{
			*buffer++ = lastValues[j];
		}
	}
	return 0;
}

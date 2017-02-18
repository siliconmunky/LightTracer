#include "BasicDefs.h"

#include "Audio.h"

#include "Camera.h"

#include "fmod_errors.h"





#define AUDIO_FX_DATA( e, wav, loop ) wav,
static const char* gAudioFXFileNames[] =
{
	AUDIO_FX_TUPLE
};
#undef AUDIO_FX_DATA



#define AUDIO_FX_DATA( e, wav, loop ) loop,
static const bool gAudioFXLoops[] =
{
	AUDIO_FX_TUPLE
};
#undef AUDIO_FX_DATA






void FMOD_FN(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		Log::Printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
	}
};




Audio* Audio::Instance = NULL;

Audio::Audio()
{
	Instance = this;


	mSystem = NULL;


	FMOD_FN(FMOD::System_Create(&mSystem));      // Create the main system object.
	FMOD_FN(mSystem->init(512, FMOD_INIT_NORMAL, 0));    // Initialize FMOD.


	LoadData();
}


Audio::~Audio()
{
}


void Audio::LoadData()
{
	for (int i = 0; i < MAX_AUDIO_FX_DATA_ENUM; ++i)
	{
		FMOD_FN(mSystem->createSound(gAudioFXFileNames[i], FMOD_3D, 0, &mSoundsData[i]));
		if (gAudioFXLoops[i])
		{
			FMOD_FN(mSoundsData[i]->setMode(FMOD_LOOP_NORMAL));
		}
	}
}

AfxHandle Audio::StartSound(AUDIO_FX_DATA_ENUM e, Vector3& pos, bool do_fft /*= false*/)
{
	int slot = INVALID_HANDLE;

	for (int i = 0; i < MAX_ACTIVE_AFX; ++i)
	{
		if (!mActiveAudio[i].mActive)
		{
			slot = i;
			break;
		}
	}
	if( slot == INVALID_HANDLE)
	{
		AfxHandle handle(INVALID_HANDLE, 0);
		return handle;
	}

	
	FMOD_FN( mSystem->playSound(mSoundsData[e], 0, false, &mActiveAudio[slot].mChannel) );

	FMOD_FN(mActiveAudio[slot].mChannel->set3DAttributes((FMOD_VECTOR*)&pos, NULL));

	if (do_fft)
	{
		FMOD_FN(mSystem->createDSPByType(FMOD_DSP_TYPE_FFT, &mActiveAudio[slot].mDSP));

		FMOD_FN(mActiveAudio[slot].mChannel->addDSP(0, mActiveAudio[slot].mDSP)); //FMOD_DSP_PARAMETER_DATA_TYPE_FFT???
		FMOD_FN(mActiveAudio[slot].mDSP->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, NUM_TRACKED_FFT_GROUPS));
		FMOD_FN(mActiveAudio[slot].mDSP->setParameterInt(FMOD_DSP_FFT_WINDOWTYPE, FMOD_DSP_FFT_WINDOW_HAMMING));
	}

	mActiveAudio[slot].mActive = true;	
	AfxHandle handle(slot, ++mActiveAudio[slot].mVerifier);

	return handle;
}

bool Audio::IsValidHandle(AfxHandle handle)
{
	return handle.mSlot >= 0 && handle.mSlot < MAX_ACTIVE_AFX && mActiveAudio[handle.mSlot].mVerifier == handle.mVerifier;

}
void Audio::UpdateSoundPos(AfxHandle handle, Vector3& pos)
{
	if( IsValidHandle(handle) )
	{

		FMOD_FN(mActiveAudio[handle.mSlot].mChannel->set3DAttributes((FMOD_VECTOR*)&pos, NULL));
	}
	else
	{
		Log::Printf("Failure to UpdateSoundPos, invalid handle\n");
	}
}

void Audio::StopSound(AfxHandle handle)
{
	if (IsValidHandle(handle))
	{
		if (mActiveAudio[handle.mSlot].mActive)
		{
			FMOD_FN(mActiveAudio[handle.mSlot].mChannel->stop());
			
			mActiveAudio[handle.mSlot].Clear();
		}
	}
	else
	{
		Log::Printf("Failure to StopSound, invalid handle\n");
	}
}

float Audio::GetFFTData(AfxHandle handle)
{
	if (IsValidHandle(handle))
	{
		//FMOD_FN(mActiveAudio[handle.mSlot].mChannel->set3DAttributes((FMOD_VECTOR*)&pos, NULL));
		if (mActiveAudio[handle.mSlot].mDSP != NULL)
		{
			FMOD_DSP_PARAMETER_FFT* fft = NULL;

			FMOD_FN(mActiveAudio[handle.mSlot].mDSP->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, (void**)&fft, NULL, NULL, 0) );

			if (fft->numchannels > 0 && fft->length > 0)
			{
				return fft->spectrum[0][0];
			}
			return 0.0f;

			/*for (int channel = 0; channel < fft->numchannels; channel++)
			{
			for (int bin = 0; bin < fft->length; ++bin) //fft->length
			{
			float freq_val = fft->spectrum[channel][bin];
			//mCurrentMagnitude[bin] = freq_val * 100.0f;
			}
			}*/
		}
	}
	else
	{
		Log::Printf("Failure to GetFFTData, invalid handle\n");
	}
	return 0;
}


void Audio::Update(float dt)
{
	for (int i = 0; i < MAX_ACTIVE_AFX; ++i)
	{
		if (mActiveAudio[i].mActive)
		{
			bool is_playing = false;
			FMOD_FN(mActiveAudio[i].mChannel->isPlaying(&is_playing));
			if (!is_playing)
			{
				mActiveAudio[i].Clear();
			}
		}
	}



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

	FMOD_FN( mSystem->set3DListenerAttributes(0, &listener_pos, &listener_vel, &listener_forward, &listener_up) );

	mSystem->update();
}




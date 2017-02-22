#pragma once

#include <stdlib.h>
#include <string.h>

#include "Log.h"


#define NULL 0
#define INVALID_HANDLE -1


inline float RandFloat()
{
	return (float)rand() / RAND_MAX;
}
#include "Log.h"

#include <iostream>
#include <windows.h>


void Log::Printf(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	const int size = 256;

	int nBuf;
	char str[size];
	nBuf = _vsnprintf_s(str, size-1, fmt, args);

	wchar_t w_str[size];
	size_t s;
	mbstowcs_s(&s, w_str, size, str, size);

	OutputDebugString(w_str);

	va_end(args);
}


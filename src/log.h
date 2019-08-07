
#pragma once

#ifndef ANDROID
	#include <cstdio>
	#define SYS_LOG(...) ((void)printf( __VA_ARGS__))
#else
	#include <android/log.h>
	#define SYS_LOG(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "nenuzhno", __VA_ARGS__))
#endif
#define LOG(...) ((void)Log( __VA_ARGS__))

enum eLogMode{
	eLogNone=0,
	eLogToConsole=1,
	eLogToFile=2,
	eLogBoth=3
};

void SetLogMode(int m);
void Log(const char *msg, ...);
//av_printf_format(3, 4);

void LogInit();


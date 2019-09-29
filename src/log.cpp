
#include <stdarg.h>
#include <cstdlib>
#include <fstream>
using namespace std;
#include "log.h"
#include "system/FileSystem.h"

ofstream enginelog;
int logMode = eLogBoth;

void LogInit()
{
	logMode = eLogBoth;

	char path[256];
#ifndef ANDROID
	snprintf(path,256,"%s/log.txt", g_fs.gamedir.c_str());
#else
	snprintf(path,256,"%s/nenuzhno-engine/%s/log.txt", getenv("EXTERNAL_STORAGE"), g_fs.gamedir.c_str());
#endif
	enginelog.open(path);
	Log("LogInit() path = %s\n",path);
	if(!enginelog)
	{
		SYS_LOG("Can't create engine log file!\n");
		return;
	}
}

void Log(const char *msg, ...)
{
	va_list vl;
	va_start(vl,msg);
	
	char buff[4096];
	vsnprintf(buff,4095, msg, vl);
	buff[4095]=0;
	
	if(logMode&eLogToConsole)
		SYS_LOG("%s",buff);
	if(logMode&eLogToFile && enginelog){
		enginelog << buff;
		enginelog.flush();
	}
	
	va_end(vl);
}

void SetLogMode(int m)
{
	if(m<eLogNone||m>eLogBoth){
		Log("Error: unknown log mode %d\n",m);
		return;
	}
	logMode = m;
}

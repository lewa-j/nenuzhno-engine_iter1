
#include <cstdlib>
#include <fstream>

#include <SLES/OpenSLES.h>

#include "log.h"
#include "sound.h"
#include "system/FileSystem.h"
#include "sound/AudioClip.h"

using namespace std;

SLObjectItf engineObj=0;
SLEngineItf engine=0;
SLObjectItf outputMixObj=0;

SLObjectItf bgPlayerObj=0;
SLPlayItf bgPlayer=0;

SLObjectItf queuePlayerObj=0;
SLPlayItf queuePlayer=0;
SLBufferQueueItf bufferQueue=0;

SLObjectItf listener=0;
SL3DLocationItf listenerLoc=0;

SLObjectItf player3DObj=0;
SLPlayItf player3D=0;
SL3DLocationItf player3DLoc=0;
SLBufferQueueItf bufferQueue3D=0;

const char *StringFromSLRes(SLresult res){
	if(res==SL_RESULT_SUCCESS)
		return "SL_RESULT_SUCCESS";
	if(res==SL_RESULT_PARAMETER_INVALID)
		return "SL_RESULT_PARAMETER_INVALID";
	if(res==SL_RESULT_FEATURE_UNSUPPORTED)
		return "SL_RESULT_FEATURE_UNSUPPORTED";
	return 0;
}

bool CheckSLError(SLresult res, const char *func, const char* file, int line){
	if(res){
		Log("sles Error %s(0x%lX) on %s %s(%d)\n",StringFromSLRes(res),res,func,file,line);
	}
	return res;
}

void SoundSystem::Init(int flags)
{
	Log("InitSound\n");
	
	SLresult res;
	
	/*SLuint32 numItfs=0;
	res = slQueryNumSupportedEngineInterfaces(&numItfs);
	Log("NumSupportedEngineInterfaces %d\n",numItfs);
	for(SLuint32 i=0;i<numItfs;i++){
		SLInterfaceID itfId;
		res = slQuerySupportedEngineInterfaces(i,&itfId);
		Log("sl intetface %d: %X\n",i,itfId);
	}*/
	const SLInterfaceID engIIds[]={SL_IID_ENGINE,SL_IID_ENGINECAPABILITIES};
	const SLboolean engReqs[]={SL_BOOLEAN_TRUE,SL_BOOLEAN_FALSE};
	int engIntfNum=2;
	res = slCreateEngine(&engineObj, 0, 0, engIntfNum, engIIds, engReqs);
	CheckSLError(res,"slCreateEngine", __FILE__, __LINE__);
	res = (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
	CheckSLError(res,"sl engineObj Realize", __FILE__, __LINE__);
	res = (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engine);
	CheckSLError(res,"sl engineObj GetInterface engine", __FILE__, __LINE__);
	/*SLuint32 numExts;
	res = (*engine)->QueryNumSupportedExtensions(engine,&numExts);
	CheckSLError(res,"sl QueryNumSupportedExtensions", __FILE__, __LINE__);
	Log("sl num extensions: %d\n",numExts);
	for(SLuint32 i=0;i<numExts;i++){
		char extName[1024];
		SLint16 extNameLen = 1024;
		res = (*engine)->QuerySupportedExtension(engine,i,(SLchar*)extName,&extNameLen);
		CheckSLError(res,"sl QuerySupportedExtension", __FILE__, __LINE__);
		Log("ext %d: %s\n",i,extName);
	}*/
	/*SLEngineCapabilitiesItf engCaps;
	res = (*engineObj)->GetInterface(engineObj, SL_IID_ENGINECAPABILITIES, &engCaps);
	if(!CheckSLError(res,"sl engineObj GetItf engCaps", __FILE__, __LINE__)){
		SLint16 ver[3];
		res = (*engCaps)->QueryAPIVersion(engCaps, ver, ver+1, ver+2);
		CheckSLError(res,"sl QueryAPIVersion", __FILE__, __LINE__);
		SLuint16 profilesSupported = 0;
		res = (*engCaps)->QuerySupportedProfiles(engCaps, &profilesSupported);
		CheckSLError(res,"sl QuerySupportedProfiles", __FILE__, __LINE__);
		Log("OpenSL ES ver(%d.%d.%d), supported profiles %d\n",ver[0],ver[1],ver[2],profilesSupported);
	}*/

	res = (*engine)->CreateOutputMix(engine, &outputMixObj, 0, 0, 0);
	CheckSLError(res,"sl engine CreateOutputMix outputMixObj", __FILE__, __LINE__);
	res = (*outputMixObj)->Realize(outputMixObj, SL_BOOLEAN_FALSE);
	CheckSLError(res,"sl outputMixObj Realize", __FILE__, __LINE__);
	
	if(flags&SOUND_QUEUE){
		InitQueuePlayer();
	}
	
	if(flags&SOUND_3D){
		Init3DPlayer();
	}

	//Log("Done\n");
}

void SoundSystem::Init3DPlayer(){
	Log("Sound: Init Listener\n");
	
	SLresult res;
	
	const SLInterfaceID listenerItfs[]={SL_IID_3DLOCATION};
	const SLboolean listenerReqs[]={SL_BOOLEAN_TRUE};
	res = (*engine)->CreateListener(engine,&listener,1,listenerItfs,listenerReqs);
	CheckSLError(res,"CreateListener", __FILE__, __LINE__);
	res = (*listener)->Realize(listener,SL_BOOLEAN_FALSE);
	CheckSLError(res,"listener Realize", __FILE__, __LINE__);
	res = (*listener)->GetInterface(listener, SL_IID_3DLOCATION, &listenerLoc);
	CheckSLError(res,"sl player3DObj GetInterface Location", __FILE__, __LINE__);
	
	Log("Sound: Init 3D Player\n");
	
	//SLDataSource dataSrc={0,0};
	SLDataLocator_BufferQueue dataLocIn={SL_DATALOCATOR_BUFFERQUEUE, 1};
	SLDataFormat_PCM dataFormat;
	dataFormat.formatType = SL_DATAFORMAT_PCM; 
	dataFormat.numChannels = 1;//mono
	dataFormat.samplesPerSec = SL_SAMPLINGRATE_44_1; 
	dataFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
	dataFormat.containerSize = 16; 
	dataFormat.channelMask = SL_SPEAKER_FRONT_CENTER;
	dataFormat.endianness = SL_BYTEORDER_LITTLEENDIAN;
	SLDataSource dataSrc={&dataLocIn, &dataFormat};
	
	SLDataLocator_OutputMix dataLocOut={SL_DATALOCATOR_OUTPUTMIX, outputMixObj};
	SLDataSink dataSink={&dataLocOut,0};
	
	const SLInterfaceID playerIIds[]={SL_IID_PLAY,SL_IID_3DLOCATION,SL_IID_BUFFERQUEUE};
	const SLboolean playerReqs[]={SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE};
	int numReqs=3;

	res = (*engine)->CreateAudioPlayer(engine, &player3DObj, &dataSrc, &dataSink, numReqs, playerIIds, playerReqs);
	if(CheckSLError(res,"sl CreateAudioPlayer Player3DObj", __FILE__, __LINE__))
		return;
	res = (*player3DObj)->Realize(player3DObj, SL_BOOLEAN_FALSE);
	CheckSLError(res,"sl player3DObj Realize", __FILE__, __LINE__);
	res = (*player3DObj)->GetInterface(player3DObj, SL_IID_PLAY, &player3D);
	CheckSLError(res,"sl player3DObj GetInterface Play", __FILE__, __LINE__);
	res = (*player3DObj)->GetInterface(player3DObj, SL_IID_3DLOCATION, &player3DLoc);
	CheckSLError(res,"sl player3DObj GetInterface Location", __FILE__, __LINE__);
	res = (*player3DObj)->GetInterface(player3DObj, SL_IID_BUFFERQUEUE, &bufferQueue3D);
	CheckSLError(res,"sl player3DObj GetInterface bufferQueue3D", __FILE__, __LINE__);
	
}

void SoundSystem::InitQueuePlayer()
{
	//SLDataLocator_AndroidSimpleBufferQueue
	SLDataLocator_BufferQueue dataLocIn={SL_DATALOCATOR_BUFFERQUEUE, 1};
	SLDataFormat_PCM dataFormat;
	dataFormat.formatType = SL_DATAFORMAT_PCM; 
	//dataFormat.numChannels = 1;//mono
	dataFormat.numChannels = 2;
	dataFormat.samplesPerSec = SL_SAMPLINGRATE_44_1; 
	dataFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16; 
	dataFormat.containerSize = 16; 
	//dataFormat.channelMask = SL_SPEAKER_FRONT_CENTER;
	dataFormat.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT; 
	dataFormat.endianness = SL_BYTEORDER_LITTLEENDIAN;
	SLDataSource dataSrc={&dataLocIn, &dataFormat};

	SLDataLocator_OutputMix dataLocOut={SL_DATALOCATOR_OUTPUTMIX, outputMixObj};
	SLDataSink dataSink={&dataLocOut,0};

	const SLInterfaceID playerIIds[]={SL_IID_PLAY, SL_IID_BUFFERQUEUE};
	const SLboolean playerReqs[]={SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

	SLresult res;

	res = (*engine)->CreateAudioPlayer(engine, &queuePlayerObj, &dataSrc, &dataSink, 2, playerIIds, playerReqs);
	CheckSLError(res,"sl engine CreateAudioPlayer queuePlayerObj", __FILE__, __LINE__);
	if(res)
		return;
	res = (*queuePlayerObj)->Realize(queuePlayerObj, SL_BOOLEAN_FALSE);
	CheckSLError(res,"sl queuePlayerObj Realize", __FILE__, __LINE__);
	res = (*queuePlayerObj)->GetInterface(queuePlayerObj, SL_IID_PLAY, &queuePlayer);
	CheckSLError(res,"sl queuePlayerObj GetInterface Play", __FILE__, __LINE__);
	res = (*queuePlayerObj)->GetInterface(queuePlayerObj, SL_IID_BUFFERQUEUE, &bufferQueue);
	CheckSLError(res,"sl queuePlayerObj GetInterface bufferQueue", __FILE__, __LINE__);
	
	res = (*queuePlayer)->SetPlayState(queuePlayer, SL_PLAYSTATE_PLAYING);
	CheckSLError(res,"sl queuePlayer SetPlayState SL_PLAYSTATE_PLAYING", __FILE__, __LINE__);
}

void SoundSystem::Play(AudioClip *clip)
{
	SLresult res;
	SLuint32 playerState;

	if(!queuePlayerObj)
		return;

	res = (*queuePlayerObj)->GetState(queuePlayerObj, &playerState);
	//Log("sl queuePlayerObj GetState playerState %lx\n", res);
	if(playerState == SL_OBJECT_STATE_REALIZED)
	{
		res = (*bufferQueue)->Clear(bufferQueue);
		//Log("sl bufferQueue Clear %lx\n", res);
		
		res = (*bufferQueue)->Enqueue(bufferQueue, clip->buffer, clip->length);
		//Log("sl bufferQueue Enqueue %lx\n", res);
	
		res = (*queuePlayer)->SetPlayState(queuePlayer, SL_PLAYSTATE_PLAYING);
		
		CheckSLError(res,"SoundSystem::Play", __FILE__, __LINE__);
	}
	else
		Log("sl playerState %lx\n", playerState);
}

void SoundSystem::Play3D(AudioClip *clip)
{
	SLresult res;
	SLuint32 playerState;
	
	if(!player3DObj)
		return;
	
	res = (*player3DObj)->GetState(player3DObj, &playerState);
	if(playerState == SL_OBJECT_STATE_REALIZED){
		res = (*bufferQueue3D)->Clear(bufferQueue3D);
		res = (*bufferQueue3D)->Enqueue(bufferQueue3D, clip->buffer, clip->length);
		res = (*player3D)->SetPlayState(player3D, SL_PLAYSTATE_PLAYING);
		
		CheckSLError(res,"SoundSystem::Play3D", __FILE__, __LINE__);
	}
	else
		Log("sl Play3D playerState %lx\n", playerState);
}

void SoundSystem::StopSound()
{
	SLresult res = (*queuePlayer)->SetPlayState(queuePlayer, SL_PLAYSTATE_STOPPED);
	CheckSLError(res,"sl queuePlayer SetPlayState SL_PLAYSTATE_STOPPED", __FILE__, __LINE__);
}

SLchar uri[1024]={0};
void SoundSystem::PlayMusic(const char *path)
{
	Log("PlayMusic %s\n", path);
	if(!std::ifstream(path))
		Log("File not found!\n");
	snprintf((char*)uri,256,"file://%s",path);
	
	SLDataLocator_URI dataLocIn={SL_DATALOCATOR_URI, uri};
	SLDataFormat_MIME dataFormat={SL_DATAFORMAT_MIME, 0, SL_CONTAINERTYPE_UNSPECIFIED};
	SLDataSource dataSrc={&dataLocIn, &dataFormat};

	SLDataLocator_OutputMix dataLocOut={SL_DATALOCATOR_OUTPUTMIX, outputMixObj};
	SLDataSink dataSink={&dataLocOut,0};

	const SLInterfaceID playerIIds[]={SL_IID_PLAY};
	const SLboolean playerReqs[]={SL_BOOLEAN_TRUE};

	SLresult res;
	
	res = (*engine)->CreateAudioPlayer(engine, &bgPlayerObj, &dataSrc, &dataSink, 1, playerIIds, playerReqs);
	CheckSLError(res,"sl engine CreateAudioPlayer bgPlayerObj", __FILE__, __LINE__);
	res = (*bgPlayerObj)->Realize(bgPlayerObj, SL_BOOLEAN_FALSE);
	CheckSLError(res,"sl bgPlayerObj Realize", __FILE__, __LINE__);
	res = (*bgPlayerObj)->GetInterface(bgPlayerObj, SL_IID_PLAY, &bgPlayer);
	CheckSLError(res,"sl bgPlayerObj GetInterface bgPlayer", __FILE__, __LINE__);
	
	res = (*bgPlayer)->SetPlayState(bgPlayer, SL_PLAYSTATE_PLAYING);
	CheckSLError(res,"sl bgPlayer SetPlayState SL_PLAYSTATE_PLAYING", __FILE__, __LINE__);
}

void SoundSystem::StopMusic()
{
	if(bgPlayer){
		SLresult res;
		SLuint32 playerState;
		res = (*bgPlayerObj)->GetState(bgPlayerObj, &playerState);
		CheckSLError(res,"sl bgPlayerObj GetState playerState", __FILE__, __LINE__);
		Log("sl playerState %lx\n", playerState);
		if(playerState == SL_OBJECT_STATE_REALIZED){
			res = (*bgPlayer)->SetPlayState(bgPlayer, SL_PLAYSTATE_PAUSED);
			CheckSLError(res,"sl bgPlayer SetPlayState SL_PLAYSTATE_PAUSED", __FILE__, __LINE__);
			
			(*bgPlayerObj)->Destroy(bgPlayerObj);
			
			bgPlayerObj = 0;
			bgPlayer = 0;
		}
	}
}

void SoundSystem::Destroy()
{
	StopMusic();
	
	if(queuePlayerObj){
		(*queuePlayerObj)->Destroy(queuePlayerObj);
		queuePlayerObj=0;
		queuePlayer=0;
		bufferQueue=0;
	}
	if(outputMixObj){
		(*outputMixObj)->Destroy(outputMixObj);
		outputMixObj=0;
	}
	if(engineObj){
		(*engineObj)->Destroy(engineObj);
		engineObj=0;
		engine=0;
	}
}

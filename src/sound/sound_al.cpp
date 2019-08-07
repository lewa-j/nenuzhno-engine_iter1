
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <fstream>
#include <stdint.h>

#include <al.h>
#include <alc.h>

#include "log.h"
#include "system/FileSystem.h"
#include "sound/sound.h"
#include "sound/AudioClip.h"

using namespace std;

int CheckALError(const char *func, const char* file, int line);

class ALSource: public IAudioSource{
public:
	ALSource(AudioClip *ac);
	~ALSource();
	void SetPos(glm::vec3 pos);
	void Play(bool loop=false);
	void Stop();

private:
	ALuint buffer;
	ALuint source;
};

class ALSoundSystem: public ISoundSystem{
public:
	ALSoundSystem():sources(),device(0),context(0){}
	~ALSoundSystem(){}
	void Init(int flags=0);
	void Update(float deltaTime);
	void Destroy();

	IAudioSource *CreateSource(const char *name);
	IAudioSource *CreateSource(AudioClip *ac);
	void SetListenerOrientation(glm::mat4 mtx);

private:
	std::vector<IAudioSource*> sources;

	ALCdevice *device;
	ALCcontext *context;
};

ISoundSystem *CreateSoundSystem(){
	return new ALSoundSystem();
}


void ALSoundSystem::Init(int flags)
{
	Log("InitSound\n");
	const char *selectedDeviceName = NULL;

	if(alcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT")){
		const char *allDevices = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
		int num=0;
		const char *curDevice = allDevices;
		//Log("OpenAL Devices:\n");
		while(*curDevice){
			//Log("AL Device %d: %s\n",num,curDevice);
			if(!selectedDeviceName&&strstr(curDevice,"OpenAL Soft")){
				selectedDeviceName = curDevice;
			}
			num++;
			curDevice += strlen(curDevice)+1;
		}
	}

	device = alcOpenDevice(selectedDeviceName);

	if(device){
		context = alcCreateContext(device, 0);
		alcMakeContextCurrent(context);
	}else{
		Log("AL: no device\n");
		return;
	}

	CheckALError("Init", __FILE__, __LINE__);

	Log("OpenAL %s %s %s (%s)\n",alGetString(AL_VERSION),alGetString(AL_VENDOR),alGetString(AL_RENDERER),alcGetString(device, ALC_DEVICE_SPECIFIER));
	//Log("Extensions: %s\n",alGetString(AL_EXTENSIONS));


}

void ALSoundSystem::SetListenerOrientation(glm::mat4 mtx)
{
	alListenerfv(AL_POSITION, &(mtx[3].x));
	//alListenerfv(AL_VELOCITY, listenerVel);
	float listenerOri[]={mtx[2].x,mtx[2].y,mtx[2].z, mtx[1].x,mtx[1].y,mtx[1].z};//fwd up
	alListenerfv(AL_ORIENTATION, listenerOri);
}

void ALSoundSystem::Update(float deltaTime){

}

void ALSoundSystem::Destroy()
{
	//StopMusic();
	
	for(uint32_t i=0; i<sources.size();i++){
		delete sources[i];
	}
	sources.clear();

	//context = alcGetCurrentContext();
	device = alcGetContextsDevice(context);
	alcMakeContextCurrent(0);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

IAudioSource *ALSoundSystem::CreateSource(const char *name)
{
	AudioClip clip;
	if(!clip.LoadFromFile(name))
		return 0;

	ALSource *src = new ALSource(&clip);

	sources.push_back(src);

	return src;
}

IAudioSource *ALSoundSystem::CreateSource(AudioClip *ac)
{
	ALSource *src = new ALSource(ac);
	sources.push_back(src);
	return src;
}

ALSource::ALSource(AudioClip *ac)
{
	int format = 0;
	if(ac->width==16){
		if(ac->channels==2)
			format = AL_FORMAT_STEREO16;
		else
			format = AL_FORMAT_MONO16;
	}else{
		if(ac->channels==2)
			format = AL_FORMAT_STEREO8;
		else
			format = AL_FORMAT_MONO8;
	}

	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, ac->buffer, ac->length, ac->freq);

	alGenSources(1, &source);
	alSourcei(source, AL_BUFFER, buffer);
}

ALSource::~ALSource()
{
	alDeleteSources(1, &source);
	alDeleteBuffers(1, &buffer);
}

void ALSource::SetPos(glm::vec3 pos)
{
	alSourcefv(source, AL_POSITION, &(pos.x));
}

void ALSource::Play(bool loop)
{
	alSourcei(source, AL_LOOPING, loop);
	alSourcePlay(source);
}

void ALSource::Stop()
{
	alSourceStop(source);
}

const char *GetALErrorString(int err)
{
	switch(err){
	case AL_INVALID_NAME:
		return "AL_INVALID_NAME";
	case AL_INVALID_ENUM:
		return "AL_INVALID_ENUM";
	case AL_INVALID_VALUE:
		return "AL_INVALID_VALUE";
	case AL_INVALID_OPERATION:
		return "AL_INVALID_OPERATION";
	case AL_OUT_OF_MEMORY:
		return "AL_OUT_OF_MEMORY";
	default:
		return "???";
	}
}

int CheckALError(const char *func, const char* file, int line)
{
	ALenum err = alGetError();
	if(err){
		Log("al Error 0x%X(%s) on %s %s(%d)\n",err,GetALErrorString(err),func,file,line);
	}
	return err;
}

//
#if 0
void ALSoundSystem::InitQueuePlayer()
{

}

void ALSoundSystem::Play(AudioClip *clip)
{
	int format = 0;
	if(clip->width==16){
		if(clip->channels==2)
			format = AL_FORMAT_STEREO16;
		else
			format = AL_FORMAT_MONO16;
	}else{
		if(clip->channels==2)
			format = AL_FORMAT_STEREO8;
		else
			format = AL_FORMAT_MONO8;
	}

	alSourceStop(source);
	CheckALError("alSourceStop", __FILE__, __LINE__);
	alSourcei(source,AL_BUFFER,0);
	CheckALError("alSourcei 0", __FILE__, __LINE__);
	alBufferData(buffer, format, clip->buffer, clip->length, clip->freq);
	CheckALError("alBufferData", __FILE__, __LINE__);
	alSourcei(source, AL_BUFFER, buffer);
	CheckALError("alSourcei buffer", __FILE__, __LINE__);
	alSourcePlay(source);
	CheckALError("alSourcePlay", __FILE__, __LINE__);
}

void ALSoundSystem::Play3D(AudioClip *clip)
{
	Log("SoundSystem::Play3D(%p)\n",clip);
}

void ALSoundSystem::StopSound()
{
	alSourceStop(source);
	CheckALError("alSourceStop", __FILE__, __LINE__);
}

void ALSoundSystem::PlayMusic(const char *path)
{
	Log("PlayMusic %s\n", path);
	if(!std::ifstream(path))
		Log("File not found!\n");

	//nothing
}

void ALSoundSystem::StopMusic()
{
	alSourceStop(source);
}
#endif

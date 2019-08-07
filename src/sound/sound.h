
#pragma once

#include <vec3.hpp>
#include <mat4x4.hpp>
#include <vector>

class AudioClip;
class ISoundSystem;

ISoundSystem *CreateSoundSystem();

#define SOUND_3D 1
#define SOUND_QUEUE 2
//#defone SOUND_MUSIC 4

class IAudioSource{
public:
	virtual ~IAudioSource(){}
	virtual void SetPos(glm::vec3 pos)=0;
	virtual void Play(bool loop=false)=0;
	virtual void Stop()=0;
};

class ISoundSystem
{
public:
	virtual ~ISoundSystem(){}
	virtual void Init(int flags=0)=0;
	virtual void Update(float deltaTime)=0;
	virtual void Destroy()=0;

	virtual IAudioSource *CreateSource(const char *name)=0;
	virtual IAudioSource *CreateSource(AudioClip *ac)=0;
	virtual void SetListenerOrientation(glm::mat4 mtx)=0;
};

/*
	void InitQueuePlayer();
	void Init3DPlayer();
	void Play(AudioClip *clip);
	void Play3D(AudioClip *clip);
	void StopSound();
	void PlayMusic(const char *path);
	void StopMusic();

};
*/

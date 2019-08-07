
#pragma once

class AudioClip
{
public:
	void *buffer;
	unsigned int length;
	int freq;
	int channels;
	int width;
	AudioClip();
	~AudioClip();

	//TODO: move to wav loader
	bool LoadFromFile(const char *fileName);
	float GetSample(int i, int c=0);
};

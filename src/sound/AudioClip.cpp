
#include <stdint.h>
#include <string>
#include <fstream>
using namespace std;

#include "log.h"
#include "system/FileSystem.h"
#include "AudioClip.h"

AudioClip::AudioClip():buffer(0),length(0),freq(0),channels(0)
{}

AudioClip::~AudioClip()
{
	if(buffer){
		//Log("destroy clip\n");
		delete[] (char*)buffer;
		buffer = 0;
	}
}

struct wavHeader_t{
	int RIFF;
	int fileSize;
	int WAVE;
	int fmtCC;
	int fmtSize;//16
	uint16_t fmt;
	uint16_t channels;
	int freq;
	int byteRate;
	uint16_t blockAlign;
	uint16_t width;
};

bool AudioClip::LoadFromFile(const char *fileName)
{
	string filePath = "sound/"+string(fileName);
	IFile *input = g_fs.Open(filePath.c_str());
	if(!input){
		//Log( "File: %s not found\n", path);
		return false;
	}

	wavHeader_t header;
	int fourCC;
	int chunkSize;
/*
	int fmt=0;
	int width=0;
	input.read((char*)&fourCC, 4);//RIFF
	input.read((char*)&chunkSize, 4);
	//Log("%s chunk size %d\n",(char*)&fourCC,  chunkSize);
	input.read((char*)&fourCC, 4);//WAVE
	input.read((char*)&fourCC, 4);//"fmt "
	input.read((char*)&chunkSize, 4);
	//Log("%s chunk size %d\n",(char*)&fourCC, chunkSize);
	input.read((char*)&fmt, 2);
	input.read((char*)&channels, 2);
	input.read((char*)&freq, 4);
	input.seekg(6,ios_base::cur);
	input.read((char*)&width, 2);
*/
	input->Read(&header,sizeof(wavHeader_t));
	freq = header.freq;
	channels = header.channels;
	width = header.width;
	Log("Load sound %s: fmt %d, channels %d, freq %d, width %d\n", fileName, header.fmt, header.channels, header.freq, header.width);

	input->Read(&fourCC, 4);//data
	if(fourCC!=0x61746164){
		if(fourCC==0x5453494C){
			input->Read(&chunkSize, 4);
			input->Seek(chunkSize,eCur);
			input->Read(&fourCC, 4);
		}else{
			//input.seekg(input.tellg()-2,ios_base::beg);
			input->Seek(-2,eCur);
			input->Read(&fourCC, 4);//data
		}
		if(fourCC!=0x61746164){
			Log("Cant find data ident in %s\n",fileName);
			g_fs.Close(input);
			return false;
		}
	}
	input->Read(&chunkSize, 4);

	if(chunkSize<0){
		Log("Bad chunk size (%d) in %s\n",chunkSize,fileName);
		g_fs.Close(input);
		return false;
	}

	length = chunkSize;
	buffer = new char[length];
	input->Read(buffer, length);

	g_fs.Close(input);

	return true;
}

float AudioClip::GetSample(int i, int c)
{
	if(width==16)
		return ((short*)buffer)[i*channels+c]/65535.0f;
	else
		return -0.5+((uint8_t*)buffer)[i*channels+c]/255.0f;
}

/*
bool AudioClip::LoadFromFile(const char *fileName)
{
	string filePath = "sound/"+string(fileName);
	char path[256];
	GetFilePath(filePath.c_str(), path);
	ifstream input(path, ios::binary);
	if(!input)
	{
		//LOG( "File: %s not found\n", path);
		return false;
	}

	int fourCC;
	int chunkSize;
	int fmt=0;
	int width=0;
	input.read((char*)&fourCC, 4);//RIFF
	input.read((char*)&chunkSize, 4);
	//LOG("%s chunk size %d\n",(char*)&fourCC,  chunkSize);
	input.read((char*)&fourCC, 4);//WAVE
	input.read((char*)&fourCC, 4);//"fmt "
	input.read((char*)&chunkSize, 4);
	//LOG("%s chunk size %d\n",(char*)&fourCC, chunkSize);
	input.read((char*)&fmt, 2);
	input.read((char*)&channels, 2);
	input.read((char*)&freq, 4);
	input.seekg(6,ios_base::cur);
	input.read((char*)&width, 2);
	//LOG("fmt %d, channels %d, freq %d, width %d\n", fmt, channels, freq, width);
	input.read((char*)&fourCC, 4);//data
	input.read((char*)&chunkSize, 4);
	//LOG("%s chunk size %d\n",(char*)&fourCC,  chunkSize);

	length = chunkSize;
	buffer = new char[length];
	input.read((char*)buffer, length);

	input.close();

	LOG("Loaded sound %s (size %d)\n",fileName,length);
	return true;
}
*/

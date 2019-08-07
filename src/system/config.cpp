
#include "config.h"
#include <stdint.h>
#include <string>
#include <fstream>
using namespace std;
#include "log.h"
#include "system/FileSystem.h"

ConfigFile::ConfigFile():values(){
	
}

ConfigFile::~ConfigFile(){
	
}

bool ConfigFile::Load(const char *fileName){
	IFile *cfg = g_fs.Open(fileName);
	if(!cfg){
		Log("Config file not found!\n");
		return false;
	}

	string line;
	while(!cfg->eof())
	{
		cfg->GetLine(line);
		//Log("line (%s)\n",line.c_str());
		
		if(line.empty())
			continue;
		if(line[0]=='#')
			continue;
		if(line.find("//")==0)
			continue;
		if(line[0]==' '){
			Log("Error: config line (%s) has no key\n",line.c_str());
			continue;
		}
		
		uint32_t t = line.find(' ');
		string key = line.substr(0,t);
		//Log("key: (%s)\n",key.c_str());
		if(t==string::npos|| line.length()-t<2){
			Log("Error: key(%s) has no value\n",key.c_str());
			continue;
		}
		
		string val=line.substr(t+1,line.length()-t-1);
		Log("%d: %s = %s\n",values.size(),key.c_str(),val.c_str());
		
		values[key] = val;
	}

	g_fs.Close(cfg);
	return true;
}

glm::vec3 ConfigFile::GetVec3(const char *k){
	glm::vec3 out;
	sscanf(values[k].c_str(), "%f %f %f", &out.x, &out.y, &out.z);
	return out;
}

int ConfigFile::GetInt(const char *k){
	int out = 0;
	sscanf(values[k].c_str(), "%d", &out);
	return out;
}

float ConfigFile::GetFloat(const char *k){
	float out = 0;
	sscanf(values[k].c_str(), "%f", &out);
	return out;
}


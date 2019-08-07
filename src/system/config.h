
#pragma once

#include <map>
#include <string>
#include <vec3.hpp>

class ConfigFile{
public:
	ConfigFile();
	~ConfigFile();
	bool Load(const char* fileName);
	std::string& operator[] (const char *k){
		return values[k];
	}
	glm::vec3 GetVec3(const char *k);
	int GetInt(const char *k);
	float GetFloat(const char *k);
	std::map<std::string,std::string> values;
};


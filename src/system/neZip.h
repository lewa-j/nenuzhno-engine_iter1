
#pragma once

#include <string.h>
#include "neFile.h"
#include "system/FileSystem.h"

class zipEntry: public IFile{
public:
	zipEntry();
	zipEntry(char *d, int l);
	~zipEntry(){}
	int GetLen(){return size;}
	int GetPos(){return p;}
	bool eof(){ return (p>=size);}
	void Seek(int ofs, eOrig_t orig);

	void Read(void *d,int l);
	void GetLine(std::string &str);
	void GetString(char *s, int l);

	int p;
	char *data;
	int size;
};

//in FileSystem.cpp
class zipArchive: public IArchive{
public:
	zipArchive();
	zipArchive(char *d, int l);
	~zipArchive();
	bool FileExists(const char *name);
	IFile *Open(const char *name);
//private:
	std::map<std::string,zipEntry> entries;
	char *data;
	int len;
};

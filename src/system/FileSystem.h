
#pragma once

#include <string>
#include <map>
#include "system/neFile.h"
#include "system/neArray.h"

//bool GetFilePath(const char *fileName, char *path, bool gamedirOnly = false);

class IArchive{
public:
	virtual ~IArchive(){}
	virtual bool FileExists(const char *name)=0;
	virtual IFile *Open(const char *name)=0;
};

class FileSystem{
public:
	~FileSystem();
	bool Init(const char *dir);
	IFile *Open(const char *name);
	void Close(IFile *n);
	char *ReadAll(const char *name);
	bool WriteAll(const char *fileName, const char *data);
	bool FileExists(const char *name,bool gamedirOnly=false);
	bool DirExists(const char *name);

	void AddArchive(IArchive *archive);

	bool GetFilePath(const char *fileName, char *path, bool gamedirOnly = false);

	std::string gamedir;
private:
	void LoadSearchPaths();
	neArray<std::string> searchPaths;
	IArchive *arc;
};

extern FileSystem g_fs;

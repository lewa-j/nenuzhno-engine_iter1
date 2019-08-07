
#pragma once

#include <fstream>

enum eOrig_t{
	eBeg,
	eCur,
	eEnd
};

#define FILE_ZIP_ENTRY 32

class IFile{
public:
	virtual ~IFile(){}

	virtual int GetLen()=0;
	virtual int GetPos()=0;
	virtual bool eof()=0;
	virtual void Seek(int ofs, eOrig_t orig=eBeg)=0;
	virtual void Read(void *d,int l)=0;
	virtual void GetString(char *s, int l)=0;
	virtual void GetLine(std::string &str)=0;

	int flags;
};

class neFile: public IFile{
public:
	neFile(){flags=0;len=0;}
	~neFile();

	bool Open(const char *name);
	int GetLen();
	int GetPos();
	bool eof();
	void Seek(int ofs, eOrig_t orig);
	void Read(void *d, int l);
	void GetString(char *s, int l);
	void GetLine(std::string &str);
private:
	std::ifstream strm;
	int len;
};

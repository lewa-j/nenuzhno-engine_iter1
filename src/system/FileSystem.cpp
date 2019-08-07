
#include <cstdlib>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fstream>
#ifdef WIN32
#include <direct.h>
#endif

#include "system/FileSystem.h"
#include "log.h"

using namespace std;

FileSystem g_fs;
ofstream fslog;

void replace(char *str, char a, char b){
	for(int i=0; str[i]; i++){
		if(str[i]==a)
			str[i]=b;
	}
}

FileSystem::~FileSystem()
{

}

bool FileSystem::Init(const char *dir)
{
	arc = 0;
	gamedir = dir;

	char path[256];
#ifdef WIN32
	snprintf(path,256,".\\%s", dir);
#else
	snprintf(path,256,"%s/nenuzhno-engine/%s", getenv("EXTERNAL_STORAGE"), dir);
#endif
	Log("InitFS() gamedir = %s\n",path);

	if(!DirExists(path)){
		Log("FS: gamedir doesn't exist\n");
#ifdef WIN32
		if(int e=_mkdir(path)){
			Log("FS: Can't create gamedir (%d)\n",e);
		}
#else
		mkdir(path,0);
#endif
		return false;
	}

	LoadSearchPaths();
#ifdef WIN32
	snprintf(path,256,"%s/fslog.txt", dir);
#else
	snprintf(path,256,"%s/nenuzhno-engine/%s/fslog.txt", getenv("EXTERNAL_STORAGE"), dir);
#endif
	fslog.open(path);
	if(!fslog){
		Log("FS: Can't create fs log file!\n");
		return false;
	}
	return true;
}

IFile *FileSystem::Open(const char *name)
{
	if(arc){
		if(arc->FileExists(name)){
			IFile *f = arc->Open(name);
			if(f)
				return f;
		}
	}
	neFile *f = new neFile();
	if(f->Open(name))
		return f;
	delete f;

	return 0;
}

void FileSystem::Close(IFile *f)
{
	if(f->flags&FILE_ZIP_ENTRY)
		return;

	delete f;
}

char* FileSystem::ReadAll(const char *fileName){
	IFile *f = Open(fileName);
	if(!f)
		return NULL;

	char* content = new char[ f->GetLen()+1 ];

	f->Read(content, f->GetLen());
	content[f->GetLen()] = '\0';
	/*if(f->flags&FILE_ZIP_ENTRY){
		Log("FileSystem::ReadAll(%s) len %d\n",fileName,f->GetLen());
		//Log("content: %s\n",content);
	}*/
	Close(f);

	return content;
}

bool FileSystem::WriteAll(const char *fileName, const char *data){
	char path[1024];
	g_fs.GetFilePath(fileName,path);
	ofstream out(path);
	out.write(data,strlen(data));
	out.close();
	return true;
}

bool FileSystem::FileExists(const char *fileName, bool gamedirOnly)
{
	char path[256];
#ifdef WIN32
	snprintf(path,256,"%s/%s", gamedir.c_str(), fileName);
#else
	snprintf(path,256,"%s/nenuzhno-engine/%s/%s", getenv("EXTERNAL_STORAGE"), gamedir.c_str(), fileName);
#endif
	if(ifstream(path)){
		return true;
	}
	if(gamedirOnly)
		return false;
	for(int i = 0; i<searchPaths.size; i++)
	{
		if(!searchPaths[i][0])
			break;
#ifdef WIN32
		snprintf(path,256,"%s/%s", searchPaths[i].c_str(), fileName);
#else
		snprintf(path,256,"%s/nenuzhno-engine/%s/%s", getenv("EXTERNAL_STORAGE"),  searchPaths[i].c_str(), fileName);
#endif
		if(ifstream(path))
		{
			return true;
		}
	}
	return false;
}

bool FileSystem::DirExists(const char *name){
	struct stat info;

	if(stat(name,&info)!=0)
		return 0;
	if(info.st_mode&S_IFDIR)
		return 1;

	return 0;
}

void FileSystem::AddArchive(IArchive *archive)
{
	arc = archive;
}


bool FileSystem::GetFilePath(const char *fileName, char *path, bool gamedirOnly)
{
#ifdef WIN32
	snprintf(path,256,"%s/%s", gamedir.c_str(), fileName);
#else
	snprintf(path,256,"%s/nenuzhno-engine/%s/%s", getenv("EXTERNAL_STORAGE"), gamedir.c_str(), fileName);
	replace(path,'\\','/');
#endif
	if(ifstream(path)){
		return true;
	}
	if(gamedirOnly)
		return false;
	for(int i = 0; i<searchPaths.size; i++)
	{
		if(!searchPaths[i][0])
			break;
#ifdef WIN32
		snprintf(path,256,"%s/%s", searchPaths[i].c_str(), fileName);
#else
		snprintf(path,256,"%s/nenuzhno-engine/%s/%s", getenv("EXTERNAL_STORAGE"),  searchPaths[i].c_str(), fileName);
#endif
		if(ifstream(path))
		{
			return true;
		}
	}
	LOG( "File: %s not found\n", path);
	fslog << path << " not found\n";
	return false;
}


void FileSystem::LoadSearchPaths()
{
	char temp[256];
#ifdef WIN32
	snprintf(temp,256,"%s/SearchPaths.txt", gamedir.c_str());
#else
	snprintf(temp,256,"%s/nenuzhno-engine/%s/SearchPaths.txt", getenv("EXTERNAL_STORAGE"), gamedir.c_str());
#endif
	ifstream input(temp);
	if(input){
		int numSearchPaths=0;
		input >> numSearchPaths;
		searchPaths.Resize(numSearchPaths);
		for(int i = 0; i < numSearchPaths; i++){
			input >> temp;
			searchPaths[i] = temp;
		}
		input.close();
		Log("FS: Loaded %d search paths\n",numSearchPaths);
	}
}

neFile::~neFile(){
	strm.close();
}

bool neFile::Open(const char *name)
{
	char path[1024];
	g_fs.GetFilePath(name,path);
	strm.open(path,ios::binary);
	if(strm){
		strm.seekg(0, ios::end);
		len = (int)strm.tellg();
		strm.seekg(0);
		return true;
	}
	return false;
}

int neFile::GetLen()
{
	return len;
}

int neFile::GetPos()
{
	return (int)strm.tellg();
}

bool neFile::eof()
{
	return strm.eof();
}

void neFile::Seek(int ofs, eOrig_t orig)
{
	ios_base::seekdir dir = ios_base::beg;
	if(orig==eCur)
		dir = ios_base::cur;
	else if(orig==eEnd)
		dir = ios_base::end;

	strm.seekg(ofs,dir);
}

void neFile::Read(void *d, int l)
{
	strm.read((char*)d,l);
}

void neFile::GetLine(string &str)
{
	getline(strm,str);
}

void neFile::GetString(char *s, int l)
{
	if(l>len-(int)strm.tellg())
		l = len-(int)strm.tellg();
	strm.getline(s,l,'\0');
}


#pragma pack(2)
struct zipEntryHeader_t{
	uint32_t signature;
	uint16_t version;
	uint16_t flags;
	uint16_t compression;
	uint16_t last_mod_file_time;
	uint16_t last_mod_file_date;
	uint32_t crc;
	uint32_t compressed_size;
	uint32_t uncompressed_size;
	uint16_t file_name_length;
	uint16_t extra_field_length;
};
#pragma pack()


#include "neZip.h"
zipArchive::zipArchive(){
	data = 0;
	len = 0;
	//Log("zipArchive()\n");
}
zipArchive::zipArchive(char *d, int l){
	//Log("zipArchive(%p, %d)\n",d,l);
	data = d;
	len = l;

	zipEntryHeader_t *entry;
	char entryName[1024];
	char *curOffs = data;
	int n = 0;
	while(true){
		entry = (zipEntryHeader_t*)curOffs;
		if(entry->signature!=0x04034b50){
			Log("zip: signature %X\n",entry->signature);
			break;
		}
		curOffs += sizeof(zipEntryHeader_t);
		strncpy(entryName,curOffs,entry->file_name_length);
		entryName[entry->file_name_length] = 0;
		curOffs += entry->file_name_length;
		curOffs += entry->extra_field_length;
		//Log("entry %d: %s, size %d, ver %d\n",n,entryName,entry->uncompressed_size,entry->version);

		entries[entryName] = zipEntry(curOffs,entry->uncompressed_size);

		curOffs += entry->uncompressed_size;
		n++;
		if((curOffs-data) >= len){
			Log("zip: eof\n");
			break;
		}
	}
	Log("Loaded %d zip files\n",n);
}

zipArchive::~zipArchive()
{
	//Log("~zipArchive() data %p len %d\n",data,len);
	//delete[] data;
}

bool zipArchive::FileExists(const char *name)
{
	return (entries.find(std::string(name))!=entries.end());
}

IFile *zipArchive::Open(const char *name)
{
	std::string temp(name);
	if(entries.find(temp)!=entries.end())
		return &(entries[temp]);

	/*
	for(uint32_t i=0;i<entries.size();i++){
		if(entries[i].name==name)
			return &(entries[i]);
	}*/
	return 0;
}

zipEntry::zipEntry(){
	data = 0;
	size = 0;
	p = 0;
	flags = FILE_ZIP_ENTRY;
}
zipEntry::zipEntry(char *d, int l){
	data = d;
	size = l;
	p = 0;
	flags = FILE_ZIP_ENTRY;
}
void zipEntry::Seek(int ofs, eOrig_t orig){
	if(orig==eBeg)
		p = ofs;
	else if(orig==eCur)
		p += ofs;
	else if(orig==eEnd)
		p = size-ofs;
}
void zipEntry::Read(void *d,int l){
	//Log("zipEntry::Read(%p, %d) p %d, data %p %s\n",d,l,p,data,data);
	memcpy(d,data+p,l);
}
void zipEntry::GetLine(std::string &str){
	char *t = strchr(data+p,'\n');
	if(!t){
		char *t = strchr(data+p,0);
		if(!t){
			str = "";
			return;
		}
	}
	str = std::string(data+p,t-data);
}
void zipEntry::GetString(char *s, int l){
	int t = strlen(data+p);
	if(t>=l)
		t = l-1;
	strncpy(s,data+p,t);
	data[t] = 0;
}

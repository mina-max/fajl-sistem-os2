#pragma once
#include "fs.h"
#include "FileEntry.h"
#include "KernelFile.h"

class KernelFile;
class FileEntry;
class Directory;
class File {
	friend class Directory;
public:
	~File(); //zatvaranje fajla
	char write(BytesCnt, char* buffer);
	BytesCnt read(BytesCnt, char* buffer);
	char seek(BytesCnt);
	BytesCnt filePos();
	char eof();
	BytesCnt getFileSize();
	char truncate();
private:
	friend class FS;
	friend class KernelFS;
	friend class Directory;
	File(FileEntry* f, Directory* d, char mode); //objekat fajla se može kreirati samo otvaranjem
	KernelFile *myImpl;
};

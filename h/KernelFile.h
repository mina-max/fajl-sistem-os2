#pragma once
#include "file.h"
#include "FileEntry.h"
class FileEntry;
class Directory;
const ClusterNo clusInd = 512ul;
class KernelFile
{
	friend class Directory;
public:
	~KernelFile(); //zatvaranje fajla
	void checkCache();
	bool writeByte(char * ch);
	char write(BytesCnt, char* buffer);
	bool readByte(char * b);
	BytesCnt read(BytesCnt, char* buffer);
	char seek(BytesCnt);
	BytesCnt filePos();
	char eof();
	BytesCnt getFileSize();
	char truncate();
private:

	Directory* myDirectory;
	FileEntry* fileEntry;
	char mode;
	BytesCnt size;

	BytesCnt cursor, cluster, offset;
	bool dirtyFirst, dirtySecond, dirtyData;

	char firstLevel[ClusterSize], secondLevel[ClusterSize], data[ClusterSize];
	ClusterNo clusterLvl1, clusterLvl2, clusterData;
	friend class File;
	KernelFile(FileEntry* f, Directory* d, char mode); //objekat fajla se može kreirati samo otvaranjem
};


#pragma once
#include "Directory.h"
struct Entry;
class FileEntry
{
	friend class Directory;
	friend class KernelFile;
private:
	HANDLE semR;
	HANDLE semW;
	HANDLE mutex;
	unsigned long readers, writers, readersWaiting, writersWaiting;
	char* fname;
	Entry* entry;
	ClusterNo lvl2Cluster;
	ClusterNo entryIndex;
	char mode;
public:
	FileEntry(char mode, char* fname, Entry* entries, ClusterNo cl, BytesCnt);
};

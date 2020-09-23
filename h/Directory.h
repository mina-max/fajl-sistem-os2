#pragma once
#include "fs.h"
#include "part.h"
#include "Sem.h"
#include "FileEntry.h"
#include "KernelFile.h"
#include "BitVector.h"
#include <iostream> 
#include <iterator> 
#include <map> 
#include <list>

using namespace std;

const unsigned int numOfEntries = 512;			//broj ulaza u indeksni blok
const unsigned int numOfDirectoryEntries = 64;	//broj ulaza u direktorijum 

struct Entry {
	char name[FNAMELEN];
	char ext[FEXTLEN];
	char reserved = 0;
	int first_index;
	BytesCnt size;
	char free[FREELEN];
}; //velicina ulaza u direktorijum je 32 bajta

class FileEntry;
class Directory
{
	friend class KernelFile;
private:

	HANDLE mutex;
	
	char root[ClusterSize];
	bool want;
	
	BitVector* bitVect;
	list<FileEntry*> openFiles; //lista otvorenih fajlova

public:
	HANDLE filesClosed;
	Partition* myPartition;
	Directory(Partition* myPartition, BitVector* bv);

	char doesExist(char * fname, FileEntry ** myFile, char mode, bool status);

	bool fileNameCompare(char *, Entry* ent);

	FileCnt numOfFiles();

	File* open(char * fname, char mode);

	void close(KernelFile * file);

	void wantUnmount();

	void free(ClusterNo clno);

	FileEntry * fileOpened(char * fname);

	void format();

	FileEntry* newFile(char* fname, char mode);

	void addEntry(char * fname, Entry * entry);

	ClusterNo allocCluster(ClusterNo *rootDir, char* buff, ClusterNo pos, ClusterNo off);

	bool freeEntry(Entry * ent);

	char deleteFile(char* fname);

};



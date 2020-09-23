#pragma once
#include "fs.h"
#include "Directory.h"
#include "BitVector.h"

class Partition;
class File;

static HANDLE partUnmount;

class KernelFS
{
public:

	~KernelFS();
	char mount(Partition* partition); //montira particiju
						// vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha
	char unmount(); //demontira particiju
						// vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha
	char format(); //formatira particiju;
						// vraca 0 u slucaju neuspeha ili 1 u slucaju uspeha
	FileCnt readRootDir();
						// vraca -1 u slucaju neuspeha ili broj fajlova u slucaju uspeha
	char doesExist(char* fname); //argument je naziv fajla sa apsolutnom putanjom

	File* open(char* fname, char mode);
	char deleteFile(char* fname);
	KernelFS();

private:

	Partition* myPartition;
	Directory* myDirectory;
	BitVector* bitVector;
	
	boolean canOpen = true;

};


#include "KernelFS.h"
#include "part.h"
#include "Sem.h"

KernelFS::KernelFS()
{
	partUnmount = CreateSemaphoreA(NULL, 0, 32, NULL);
}


char KernelFS::mount(Partition * partition)
{
	if (myPartition != nullptr) {
		wait(partUnmount);
	}
	myPartition = partition;
	bitVector = new BitVector(myPartition);
	myDirectory = new Directory(myPartition, bitVector);
	return 1;
}

char KernelFS::unmount()
{
	myDirectory->wantUnmount();
	wait(myDirectory->filesClosed);
	myPartition = nullptr;
	signal(partUnmount);
	return 1;
}

char KernelFS::format()
{ 
	if (myPartition == nullptr) return 0;

	bitVector->format();	
	myDirectory->format();

	return 1;
}

FileCnt KernelFS::readRootDir()
{
	return myDirectory->numOfFiles();
}

char KernelFS::doesExist(char * fname)
{
	return myDirectory->doesExist(fname, nullptr, 'r', false);
}

File * KernelFS::open(char * fname, char mode)
{
	return myDirectory->open(fname, mode);
}

char KernelFS::deleteFile(char * fname)
{
	return myDirectory->deleteFile(fname);
}

KernelFS::~KernelFS()
{
	unmount();
}

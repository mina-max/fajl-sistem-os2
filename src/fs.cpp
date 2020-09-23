#include "fs.h"
#include "KernelFS.h"
KernelFS* FS::myImpl = new KernelFS();

FS::FS()
{
}

char FS::mount(Partition* partition)
{
	 return FS::myImpl->mount(partition);
}

char FS::unmount()
{
	return FS::myImpl->unmount();
}

char FS::format()
{
	return FS::myImpl->format();
}

FileCnt FS::readRootDir()
{
	return FS::myImpl->readRootDir();
}

char FS::doesExist(char* fname)
{
	return FS::myImpl->doesExist(fname);
}

File* FS::open(char* fname, char mode)
{
	return FS::myImpl->open(fname, mode);
}

char FS::deleteFile(char* fname)
{
	return FS::myImpl->deleteFile(fname);
}

FS::~FS()
{
	delete myImpl;
}

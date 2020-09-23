#include "file.h"
#include "KernelFile.h"

File::~File()
{
	delete myImpl;
}

char File::write(BytesCnt size, char * buffer)
{
	return myImpl->write(size, buffer);
}

BytesCnt File::read(BytesCnt size, char * buffer)
{
	return myImpl->read(size, buffer);
}

char File::seek(BytesCnt size)
{
	return myImpl->seek(size);
}

BytesCnt File::filePos()
{
	return myImpl->filePos();
}

char File::eof()
{
	return myImpl->eof();
}

BytesCnt File::getFileSize()
{
	return myImpl->getFileSize();
}

char File::truncate()
{
	return myImpl->truncate();
}

File::File(FileEntry* f, Directory* d, char mode) {
	myImpl = new KernelFile(f, d, mode);
}
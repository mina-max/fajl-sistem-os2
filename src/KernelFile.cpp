#include "KernelFile.h"

KernelFile::KernelFile(FileEntry *f, Directory *d, char m) {
	fileEntry = f;
	myDirectory = d;
	mode = m;
	cursor = cluster = offset = 0;
	dirtyFirst = dirtySecond = dirtyData = true;
	clusterLvl1 = fileEntry->entry->first_index;
	myDirectory->myPartition->readCluster(clusterLvl1, firstLevel);
	size = fileEntry->entry->size;
	if (mode == 'r') {
		seek(0);
		return;
	}
	if (mode == 'a') {
		seek(size);
		return;
	}
	if (mode == 'w') {
		truncate();
	}
}


KernelFile::~KernelFile()
{
	myDirectory->close(this);
}

void KernelFile::checkCache() {
	if (cursor % (numOfEntries * ClusterSize) == 0) {
		dirtyFirst = dirtySecond = dirtyData = true;
	}
	if (cursor % ClusterSize == 0) {
		dirtySecond = dirtyData = true;
	}
}


bool KernelFile::readByte(char* b) {
	if (cursor < size) {
		checkCache();
		//LEVEL 1

		ClusterNo ind1 = cluster / numOfEntries; //indeks u prvom nivou

		if (dirtyFirst) {
			myDirectory->myPartition->readCluster(clusterLvl1, firstLevel);
			dirtyFirst = false;
		}

		//LEVEL 2

		clusterLvl2 = ((ClusterNo*)firstLevel)[ind1];
		ClusterNo ind2 = cluster % numOfEntries; //indeks u drugom nivou
		
		if (dirtySecond) {
			myDirectory->myPartition->readCluster(clusterLvl2, secondLevel);
			dirtySecond = false;
		}
		//DATA

		clusterData = ((ClusterNo*)secondLevel)[ind2];

		if (dirtyData) {
			myDirectory->myPartition->readCluster(clusterData, data);
			dirtyData = false;
		}
		*b = data[cursor % ClusterSize];
		seek(cursor + 1);
		return true;
	}
	else {
		return false;
	}
}

bool KernelFile::writeByte(char* ch) {

	//LEVEL 1

	ClusterNo ind1 = cluster / numOfEntries; //ulaz u indeksu prvog nivoa
	
	if (dirtyFirst) {
		myDirectory->myPartition->readCluster(clusterLvl1, firstLevel);
		dirtyFirst = false;
	}

	if (/*(size == cursor)&& */ (size % (numOfEntries * ClusterSize) == 0)) { 
		clusterLvl2 = myDirectory->bitVect->getFirstEmpty();	
		((ClusterNo*)firstLevel)[ind1] = clusterLvl2;
		myDirectory->myPartition->writeCluster(clusterLvl1, firstLevel);
		dirtyFirst = true;
	}
	else {
		clusterLvl2 = ((ClusterNo*)firstLevel)[ind1];
	}

	//LEVEL 2

	ClusterNo ind2 = cluster % numOfEntries; //ulaz u indeksu drugog nivoa
	
	if (dirtySecond) {
		myDirectory->myPartition->readCluster(clusterLvl2, secondLevel);
		dirtySecond = false;
	}
	
	if (/*cursor == size &&*/ size % ClusterSize == 0) {
		clusterData = myDirectory->bitVect->getFirstEmpty();
		((ClusterNo*)secondLevel)[ind2] = clusterData;
		myDirectory->myPartition->writeCluster(clusterLvl2, secondLevel);
		dirtySecond = true;
		dirtyData = true;
	}
	else {
		clusterData = ((ClusterNo*)secondLevel)[ind2];
	}

	//DATA
	
	if (dirtyData) {
		myDirectory->myPartition->readCluster(clusterData, data);
		dirtyData = false;
	}
	data[cursor % ClusterSize] = *ch;
	myDirectory->myPartition->writeCluster(clusterData, data);

	/*if (size == cursor)*/ size++;
	seek(cursor + 1);
	return true;
}

char KernelFile::write(BytesCnt b, char* buffer) {
	
	BytesCnt cnt = 0;
	for (cnt = 0; cnt < b; cnt++) {
		if (writeByte(buffer + cnt) == false) break;
	}
	return (cnt == b);
}

BytesCnt KernelFile::read(BytesCnt numOfBytes, char* buffer) {
	
	int cnt = 0;
	for (BytesCnt i = 0; i < numOfBytes; i++) {
		if (!readByte(buffer + i)) break;
		cnt++;
	}
	return cnt;
}

char KernelFile::seek(BytesCnt size)
{
	cursor = size;
	cluster = cursor / ClusterSize; //u kom klasteru sa podacima se nalazimo
	//offset = cursor % ClusterSize;
	return 1;
}

BytesCnt KernelFile::filePos()
{
	return cursor;
}

char KernelFile::eof()
{
	if (filePos() < size) return 0;
	if (filePos() == size) return 2;
	return 1;
}

BytesCnt KernelFile::getFileSize()
{
	return size;
}

char KernelFile::truncate(){
	ClusterNo data = size / ClusterSize; //broj data klastera
	if (size % ClusterSize != 0) data++;

	myDirectory->myPartition->readCluster(clusterLvl1, firstLevel);
	ClusterNo ind1 = 0, ind2 = 0;
	for (ClusterNo cl = 0; cl < data; cl++) { 
		ind2 = cl % numOfEntries;
		if (ind2 == 0) {
			ind1 = cl / numOfEntries;
			myDirectory->myPartition->readCluster(firstLevel[ind1], secondLevel);
		}
		myDirectory->free(((ClusterNo*)secondLevel)[ind2]); //oslobadjamo ulaz u 2. nivou
	}
	ClusterNo lvl2 = data / numOfEntries; //broj klastera za indekse drugog nivoa
	if (data % numOfEntries != 0) lvl2++;
	for (ClusterNo cl = 0; cl < lvl2; cl++) {
		myDirectory->free(((ClusterNo*)firstLevel)[cl]);
	}
	size = 0;
	seek(0);
	return 1;
	
}
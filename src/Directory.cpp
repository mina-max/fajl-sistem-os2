#include "Directory.h"
#include "FileEntry.h"
#include "file.h"
#include "BitVector.h"


Directory::Directory(Partition * myP, BitVector* v)
{
	myPartition = myP;
	bitVect = v;
	want = false;
	myPartition->readCluster(1, root);
	mutex = CreateSemaphoreA(NULL, 1, 1, NULL);
	filesClosed = CreateSemaphoreA(NULL, 1, 32, NULL);
}

char Directory::doesExist(char * fname, FileEntry** myFile, char mode, bool status)
{
	//ako smo pozvali ovu metodu, fajl sigurno nije otvoren (znaci ne postoji njegov fileEntry)
	char lvl2[ClusterSize], entryBuffer[ClusterSize];
	ClusterNo* lvl2cluster;
	Entry* entries;

	myPartition->readCluster(1, root);
	ClusterNo* rootDir = (ClusterNo*)root;
	
	for (ClusterNo i = 0; i < numOfEntries; i++) {
		if (rootDir[i] != 0) {
			myPartition->readCluster(rootDir[i], lvl2);
			lvl2cluster = (ClusterNo*)lvl2;
			for (ClusterNo j = 0; j < numOfEntries; j++) {
				if (lvl2cluster[j] != 0) {
					myPartition->readCluster(lvl2cluster[j], entryBuffer);
					entries = (Entry*)entryBuffer;
					for (int k = 0; k < numOfDirectoryEntries; k++) {
						if (fileNameCompare(fname, entries + k)) {
							if (status) {
								*myFile = new FileEntry(mode, fname, entries + k, lvl2[j], k);
							}
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}	

bool Directory::fileNameCompare(char* path, Entry* ent) {
	unsigned int k = 0;
	bool help = false;
	int i = 0;
	while(i < FNAMELEN) {
		if (i < strlen(path) && path[i] == '.') {
			help = true;
			if (ent->name[i] != ' ') return false;
			k = i + 1;
		}
		if (!help && ent->name[i] != path[i])
			return false;
		i++;
	}
	i = 0;
	while(i < FEXTLEN) {
		if (k >= strlen(path) || ent->ext[i] != path[k++])
			return false;
		i++;
	}
	return true;
}

FileCnt Directory::numOfFiles()
{
	Entry *entryArray;
	ClusterNo *clusterArray;
	char lvl1Buffer[ClusterSize];
	char lvl2Buffer[ClusterSize];
	int cnt = 0;
	for (ClusterNo i = 0; i < numOfEntries; i++)
		if (root[i] != 0)
		{
			myPartition->readCluster(root[i], lvl1Buffer);
			clusterArray = (ClusterNo *)lvl1Buffer;
			for (ClusterNo j = 0; j < numOfEntries; j++)
				if (clusterArray[j] != 0)
				{
					myPartition->readCluster(clusterArray[j], lvl2Buffer);
					entryArray = (Entry *)lvl2Buffer;
					for (ClusterNo k = 0; k < numOfDirectoryEntries; k++)
						if (!freeEntry(entryArray + k))
							cnt++;
				}
		}
	return (FileCnt)cnt;
}

File * Directory::open(char * fname, char mode)
{
	wait(mutex);
	FileEntry* myFile = fileOpened(fname);

	if (myFile != nullptr) {
		//fajl je otvoren
		if (mode == 'r') {
			if (myFile->writers == 1) {
				myFile->readersWaiting++;
				signalAndWait(mutex, myFile->semR);
				wait(mutex);
			}
			myFile->readers++;			
			signal(mutex);
			return new File(myFile, this, mode);
		}
		if (mode == 'w' || mode == 'a') {
			if (myFile->readers > 0 || myFile->writers == 1) {
				myFile->writersWaiting++;
				signalAndWait(mutex, myFile->semW);
				wait(mutex);
			}
			myFile->writers++;
			signal(mutex);
			return new File(myFile, this, mode);
		}
	}
	else {
		if (doesExist(fname, &myFile, mode, true)) {
			//fajl nije otvoren ali postoji
			openFiles.push_back(myFile);
			signal(mutex);
			return new File(myFile, this, mode);
		}
		else {
			//fajl ne postoji
			if (mode == 'r' || mode == 'a') {
				signal(mutex);
				return nullptr;
			}
			else if (mode == 'w') {
				//napravi novi fileEntry
				myFile = newFile(fname, mode);
				openFiles.push_back(myFile);
				signal(mutex);
				return new File(myFile, this, mode);
			}
		}
	}
	signal(mutex);
	return nullptr;
}

void Directory::close(KernelFile* file)
{
	wait(mutex);

	int size = file->getFileSize();
	if (size != file->fileEntry->entry->size) {
		file->fileEntry->entry->size = size;
		char temp[ClusterSize];
		myPartition->readCluster(file->fileEntry->lvl2Cluster, temp);
		((Entry*)(temp))[file->fileEntry->entryIndex].size = size;
		myPartition->writeCluster(file->fileEntry->lvl2Cluster, temp);
	}

	FileEntry* myFile = fileOpened(file->fileEntry->fname);
	if (myFile != nullptr) {
		if (myFile->readers == 1 && myFile->writers == 0) {
			myFile->readers--;
			if (myFile->readersWaiting > 0) {
				while (myFile->readersWaiting) {
					myFile->readersWaiting--;
					signal(myFile->semR);
				}
			}
			else if (myFile->writersWaiting > 0) {
				myFile->writersWaiting--;
				signal(myFile->semW);
			}
			else {
				openFiles.remove(myFile);
				if (openFiles.empty() && want) {
					want = false;
					signal(filesClosed);
				}
			}
		}
		else if (myFile->readers > 1) {
			myFile->readers--;
		}
		else if (myFile->writers == 1) {
			myFile->writers--;
			if (myFile->readersWaiting > 0) {
				while (myFile->readersWaiting) {
					myFile->readersWaiting--;
					signal(myFile->semR);
				}
			}
			else if (myFile->writersWaiting) {
				myFile->writersWaiting--;
				signal(myFile->semW);
			}
			else {
				openFiles.remove(myFile);
				if (openFiles.empty() && want) {
					want = false;
					signal(filesClosed);
				}
			}
		}
	}
	if (openFiles.empty() && want) {
		want = false;
		signal(filesClosed);
	}

	signal(mutex);
}

void Directory::wantUnmount() {
	want = true;
}

void Directory::free(ClusterNo clno) {
	bitVect->free(clno);
}

FileEntry* Directory::fileOpened(char* fname) {					//trazim da li je taj fajl vec otvoren
	list<FileEntry*>::iterator i;
	FileEntry* file = nullptr;
	for (i = openFiles.begin(); i != openFiles.end(); i++) {
		if (strcmp((*i)->fname, fname) == 0) {
			file = *i;
			return file;
		}
	}
	return nullptr;
}

void Directory::format() {
	for (int i = 0; i < numOfEntries; i++)
		root[i] = 0;
	myPartition->writeCluster(1, root);
}

FileEntry* Directory::newFile(char* fname, char mode)
{
	ClusterNo* lvl2Cluster;
	Entry* entries;

	char lvl2[ClusterSize], entryBuffer[ClusterSize];

	myPartition->readCluster(1, root);
	ClusterNo* rootDir = (ClusterNo*)root;
	Entry* entry;

	lvl2Cluster = (ClusterNo*)lvl2;
	entries = (Entry*)entryBuffer;

	for (int i = 0; i < numOfEntries; i++) {
		if (rootDir[i] != 0) {
			myPartition->readCluster(rootDir[i], lvl2);
			lvl2Cluster = (ClusterNo*)lvl2;
			for (int j = 0; j < numOfEntries; j++) {
				if (lvl2Cluster[j] != 0) {
					myPartition->readCluster(lvl2Cluster[i], entryBuffer);
					entries = (Entry*)entryBuffer;
					for (int k = 0; k < numOfDirectoryEntries; k++) {
						entry = entries + k;
						if (freeEntry(entry)) {
							addEntry(fname, entry);
							myPartition->writeCluster(lvl2Cluster[j], (const char*)entries);

							return new FileEntry(mode, fname, entry, lvl2Cluster[j], k);
						}
					}
				}
				else {
					ClusterNo entryCluster = allocCluster(lvl2Cluster, entryBuffer,rootDir[i], j);
					entries = (Entry*)entryBuffer;
					addEntry(fname, entries);
					myPartition->writeCluster(entryCluster, (const char*)entries);

					return new FileEntry(mode, fname, entries, entryCluster, j);	
				}
			}
		}
		else {
			ClusterNo lvl2Entry = allocCluster(rootDir, lvl2, 1, i); //vraca broj klastera u kom se nalazi lvl2
			ClusterNo entryCluster = allocCluster(lvl2Cluster, entryBuffer, lvl2Entry, 0); //vraca broj klastera u kom se nalaze entry-ji
			entries = (Entry*)entryBuffer;
			addEntry(fname, entries);
			myPartition->writeCluster(entryCluster, (const char*)entries);

			return new FileEntry(mode, fname, entries, entryCluster, 0);
		}
	}
	return nullptr;
}

void Directory::addEntry(char *fname, Entry *entry) {

	unsigned int k = 0;
	unsigned int i = 0;
	bool flag = false;
	while(i < FNAMELEN)
	{
		if (i < strlen(fname) && fname[i] == '.')
		{
			flag = true;
			k = i + 1;
		}
		entry->name[i] = flag ? ' ' : fname[i];
		
		i++;
	}
	i = 0;
	while(i < FEXTLEN){
		entry->ext[i] = k >= strlen(fname) ? ' ' : fname[k++];
		
		i++;
	}

	entry->first_index = bitVect->getFirstEmpty();
	entry->size = 0;

}

ClusterNo Directory::allocCluster(ClusterNo* buffer, char* buff, ClusterNo pos, ClusterNo off) {
	ClusterNo k = bitVect->getFirstEmpty();
	buffer[off] = k;
	myPartition->writeCluster(pos, (const char*)buffer);
	for (int i = 0; i < ClusterSize; i++)
		buff[i] = 0;
	return k;
}

bool Directory::freeEntry(Entry* entry) {
	for (int i = 0; i < FEXTLEN; i++)
		if (entry->ext[i] != 0)
			return false;
	return true;
}

char Directory::deleteFile(char* fname)
{
	FileEntry* myFile = fileOpened(fname);
	if (myFile == nullptr) {
		//fajl nije otvoren
		if (doesExist(fname, &myFile, 'r', false)) {
			bitVect->free(myFile->entry->first_index);
			char buffer[ClusterSize];
			myPartition->readCluster(myFile->lvl2Cluster, buffer);
			for (ClusterNo i = myFile->entryIndex*32; i < myFile->entryIndex*32 + 8; i++)
				buffer[i] = 0;
			myPartition->writeCluster(myFile->lvl2Cluster, buffer);

			delete myFile;
			return 1;
		}
	}
	return 0;
}

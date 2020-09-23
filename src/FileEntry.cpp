#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include "FileEntry.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

FileEntry::FileEntry(char m, char * n, Entry * ent, ClusterNo pos, BytesCnt off)
{
	fname = new char[strlen(n) + 1];
	strncpy(fname, n, strlen(n) + 1);
	entry = new Entry();
	strncpy(entry->name, ent->name, 8);
	strncpy(entry->ext, ent->ext, 3);
	entry->size = ent->size;
	entry->first_index = ent->first_index;
	entry->reserved = ent->reserved;
	lvl2Cluster = pos; //zapravo entryCluster (klaster u kom se nalazi taj entry)
	entryIndex = off;	
	mode = m;

	semR = CreateSemaphoreA(NULL, 0, 32, NULL);
	semW = CreateSemaphoreA(NULL, 0, 32, NULL);
	mutex = CreateSemaphoreA(NULL, 1, 1, NULL);
	if (mode == 'r')
	{
		readers++;
	}
	if (mode == 'w' || mode == 'a')
	{
		writers++;
	}
}
#endif
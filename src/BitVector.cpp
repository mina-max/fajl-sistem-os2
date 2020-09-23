#include "BitVector.h"
#include <iostream>

BitVector::BitVector(Partition * p)
{
	myPart = p;
	vect = new char[ClusterSize];
	myPart->readCluster(0, vect);
}

void BitVector::format()
{
	for (int i = 0; i < ClusterSize; i++) {
		vect[i] = ~0;
	}
	vect[0] = (~0) ^ (3);  //11111111 xor 00000011 = 11111100
	myPart->writeCluster(0, vect);
}

ClusterNo BitVector::getFirstEmpty()
{
	ClusterNo num = 0;
	//char mask1 = 0b11111111;
	char mask = 0;
	for (int i = 0; i < ClusterSize; i++) {
		char oneByte = vect[i];
		for (int j = 0; j < 8; j++) {
			if (oneByte & 1) {
				/*mask2 = (1 << j);
				mask3 = ~(mask1 & mask2);
				vect[i] &= mask3;*/
				mask = 1 << j;
				vect[i] &= ~mask;
				return num;
			}
			oneByte >>= 1;
			num++;
		}
	}
	
}


void BitVector::free(ClusterNo cl)
{
	int index = cl / 8;
	int offset = cl % 8;
	char oneByte = vect[index];
	oneByte |= 1 << offset;
	vect[index] = oneByte;
}
/*

BitVector::BitVector(Partition* p) {
	part = p;
	size = part->getNumOfClusters();
	bitVect = new char[ClusterSize];
	part->readCluster(0, (char*)bitVect);
	freeCl = 2;
	findFree();
}

BitVector::~BitVector() {
	part->writeCluster(0, bitVect);
	delete bitVect;
}

void BitVector::findFree() {
	ClusterNo ref = freeCl;
	do {
		if ((bitVect[freeCl / 8] & ((1 << (7 - freeCl % 8)))) == 1) {
			//nasao je slobodan
			return;
		}
		freeCl = (freeCl + 1) % size;
	} while (freeCl != ref);
	//Zauzeti su svi klasteri
	exit(0);
}

ClusterNo BitVector::getFirstEmpty() {
	ClusterNo ret = freeCl;
	bitVect[freeCl / 8] ^= (1 << (7 - freeCl % 8));
	findFree();
	return ret;
}

void BitVector::free(ClusterNo cl) {
	bitVect[cl / 8] |= (1 << (7 - cl % 8));
}

void BitVector::format() {
	for (int j = 0; j < ClusterSize; j++) bitVect[j] = -1;
	bitVect[0] ^= (1 << 7); //za 0. klaster(tu je bitVect)
	bitVect[0] ^= (1 << 6); //za 1. klaster(tu je root)
	freeCl = 2;
	//ne mora findFree jer vec znamo da je 2 slobodan
}*/
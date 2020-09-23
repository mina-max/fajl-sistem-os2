#pragma once
#include "part.h"
#include <bitset>
using namespace std;

const unsigned long BitVectorSize = ClusterSize * 8;

class BitVector {
private:
	char* vect;
	int size;
	Partition* myPart;
	ClusterNo freeCl;
public:
	BitVector(Partition*);
	//~BitVector();
	void format();
	//void findFree();
	ClusterNo getFirstEmpty();
	void free(ClusterNo cl);
};


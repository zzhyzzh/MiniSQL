#pragma once
#include"stdafx.h"
#include"BufferManager.h"
#include"CatalogManager.h"
#include<cmath>

class BPlusTree
{
private:
	int leaf_least;
	int nonleaf_least;
	int order;
	int rootBlockNum;
	Index myIndexInfo;
public:
	BPlusTree(Index indexInfo);
	void insert(string key, int blockOff, int offset); //pass
	int searchLeaf(string key); //pass
	string intToStr(int value); //pass
	string toLength(string str, int length);  //pass
	void write(Block *b, string s);  //pass
	Offset searchEqual(string key); //pass
	void writeRootBlock(Block b); //unused
	int getValueNum(string snum); //pass
	void fatherUpdate(int blockNum); //pass
	void insertInLeaf(Block *node, string key, int blockNum, int offset); //pass
	void insertInParent(int blockNum, int blockNum1, string key); //pass
	void deleteOne(string key); //pass
	void deleteEntry(int blockNum,string key, int deletedNum); //pass
	void updateKey(string key, int blockOffset, int offset);
};
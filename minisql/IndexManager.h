#pragma once
//#include "BlockInfo.h"
#include "BufferManager.h"
#include"BPlusTree.h"
#include"CatalogManager.h"
#include "stdafx.h"
#include<iostream>
class IndexManager
{
private:
	int m_str;
public:
	void createIndex(Table tableInfo, Index indexInfo);
	void dropIndex(Index indexinfo);
	void insertKey(Index indexInfo, string key, int blockOffset, int offset);
	void searchEqual(Table tableInfo, Index indexInfo, string key);
	void deleteKey(Index indexInfo, string deletedKey);
	void updateKey(Index indexInfo, string key,int blockOffset, int offset);
	void deleteAllIndex(Table tableInfo,Index indexInfo);
};
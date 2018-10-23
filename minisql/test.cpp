#include<iostream>
#include<string>
#include"catalogManager.h"
#include"BufferManager.h"
#include"indexManager.h"
#include"BPlusTree.h"
#include"stdafx.h"
using namespace std;
BufferManager bufferManager;
CatalogManager *catalogManager;
IndexManager indexManager;
void bufferTest();
void indexTest();
void createTest();
int main()
{
	//createTest();
	indexTest();
	

	system("pause");
	return 0;
}

void createTest()
{
	Table tableInfo;
	Index indexInfo;
	Record res;
	Attr attrInfo;
	attrInfo.name = "a0";
	attrInfo.length = 4;

	tableInfo.blockNum = 1;
	tableInfo.attrNum = 2;
	tableInfo.name = "test";
	tableInfo.attributes.push_back(attrInfo);
	attrInfo.name = "a1";
	attrInfo.length = 6;
	tableInfo.attributes.push_back(attrInfo);
	tableInfo.totalLength = 10;
	
	indexInfo.blockNum = 1;
	indexInfo.length = 4;
	indexInfo.name = "a0";
	indexInfo.tableName = "test";
	indexInfo.type = 2;
	indexInfo.value = "test_index";

	string fileName = "file/record/" + tableInfo.name + ".txt";   //从记录文件名中获取数据
	Block *newBlock = new Block;
	newBlock = bufferManager.getFile(fileName);
	string info = newBlock->CBlock;
	info = "0003zhao000001liu#000002zhou000003";
	newBlock->CBlock = new char[info.size() + 1];
	strcpy(newBlock->CBlock, info.c_str());
	newBlock->charNum = strlen(newBlock->CBlock);
	newBlock->isDirty = 1;
	
	indexManager.createIndex(tableInfo,indexInfo);

	fileName = "file/index/" + tableInfo.name + "_" + indexInfo.name + ".txt"; //文件名格式: 表名_索引名
	//Block *bk = bufferManager.readBlock(fileName, 0);
	//cout << bk->CBlock << endl;
	indexManager.searchEqual(tableInfo, indexInfo, "zhao");
	//cout << res.columns[1] << endl;

	//indexManager.dropIndex(indexInfo);
	//Block *newbk = bufferManager.head->next;
	//while (newbk)
	//{
	//	cout << newbk->blockNum << endl;
	//	newbk = newbk->next;
	//}
}

string extStr(string str, int length)
{
	while (str.length() < length)
	{
		str = str + "#";
	}
	return str;
}
void indexTest()
{
	Index indexInfo;
	Block *newBlock = new Block;
	indexInfo.blockNum = 3;
	indexInfo.name = "testName";
	indexInfo.length = 1200;
	indexInfo.tableName = "test";
	indexInfo.type = 2;
	indexInfo.value = "test_index";
	string key = "testa";
	string key2 = "sakura";
	string key3 = "sally";
	string key4 = "Brown";
	string key5 = "Zhao";
	string key6 = "liu";
	string key7 = "titan";
	string key8 = "maria";
	string key9 = "leo";
	string key10 = "guojiang";
	string key11 = "papi";
	string key12 = "Andy";
	string fileName = "file/index/" + indexInfo.tableName + "_" + indexInfo.name + ".txt";
	bufferManager.getFile(fileName);  //初始化
	key = extStr(key, indexInfo.length); //位扩展
	key2 = extStr(key2, indexInfo.length);
	key3 = extStr(key3, indexInfo.length);
	key4 = extStr(key4, indexInfo.length);
	key5 = extStr(key5, indexInfo.length);
	key6 = extStr(key6, indexInfo.length);
	key7 = extStr(key7, indexInfo.length);
	key8 = extStr(key8, indexInfo.length);
	key9 = extStr(key9, indexInfo.length);
	key10 = extStr(key10, indexInfo.length);
	key11 = extStr(key11, indexInfo.length);
	key12 = extStr(key12, indexInfo.length);

	BPlusTree bp(indexInfo);
	bp.insert(key, 2, 3);
	bp.insert(key2, 2, 4);
	bp.insert(key3, 2, 5);
	bp.insert(key4, 2, 6);
	bp.insert(key5, 2, 7);
	bp.insert(key6, 2, 8);
	bp.insert(key7, 2, 9);
	bp.insert(key8, 2, 10);
	bp.insert(key9, 2, 11);
	bp.insert(key10, 2, 12);
	bp.insert(key11, 2, 13);
	bp.insert(key12, 2, 14);
/*	bp.deleteOne(key4);
	bp.deleteOne(key5);
	bp.deleteOne(key10);
	bp.deleteOne(key12);
	bp.deleteOne(key6);
	bp.deleteOne(key9);
	bp.deleteOne(key8);
	bp.deleteOne(key11);
	bp.deleteOne(key2);
	bp.deleteOne(key3);*/
	//bp.updateKey(key4, 2, 18);
	//indexManager.dropIndex(indexInfo);
	//newBlock = bufferManager.readBlock(fileName, 2);
	//cout << newBlock->CBlock << endl;

	Block *newbk = bufferManager.head->next;
	while (newbk)
	{
		cout << newbk->blockNum << endl;
		newbk = newbk->next;
	}
	//Offset off = bp.searchEqual(key3);
	//cout << off.blockNum << endl;
	//cout << off.offset[0] << endl;
	//cout << newBlock->CBlock << endl;
	//bp.deleteOne(key);
	//cout << bp.findLeaf(key3) << endl;
}
void bufferTest()
{
	string fileName = "file/index/a.txt";
	Block *newblock = bufferManager.getFile(fileName);
	string info = newblock->CBlock;
	info = "hello";
	//newblock->CBlock = new char[info.size() + 1];
	strcpy(newblock->CBlock, info.c_str());
	newblock->charNum = strlen(newblock->CBlock);
	newblock->isDirty = 1;

	newblock = bufferManager.getBlankBlock(fileName);
	newblock = bufferManager.getBlankBlock(fileName);
	newblock = bufferManager.getBlankBlock(fileName);
	newblock = bufferManager.getBlankBlock(fileName);
	bufferManager.deleteBlock(fileName, 3);
	Block *newbk = bufferManager.head->next;
	while (newbk)
	{
		cout << newbk->blockNum << endl;
		newbk = newbk->next;
	}
	Block *testBlock = bufferManager.readBlock(fileName, newblock->blockNum);
//	cout << testBlock->CBlock << endl;

}
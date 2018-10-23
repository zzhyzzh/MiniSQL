#pragma once
#include<stdio.h>
#include<string>
#include<vector>
#include"stdafx.h"
#define MAXBLOCKNUM 1024						//定义缓冲区最多缓冲块数目
using namespace std;

typedef Block *pointer;
typedef File *filePtr;

class BufferManager
{
public:
	Block *head = new Block;//存储所有缓冲块的链表头
	Block *tail = new Block;//存储所有缓冲块的链表头
	File *fileHead = new File;//存储文件信息的链表
	int number;//bufferlist总长度

	BufferManager();

	~BufferManager();

	//获取文件尾块号
	//int getlastEmptyBlock(string fileName);

	//将块写回文件
	void writeBlockToFile(pointer bufferBlock);
	
	//将所有块写回到文件中去
	void writeBufferToFile();
	
	//将所有块写回到文件中去，并释放buffer区
	void releaseBufferBlocks();
	
	//根据文件名和偏移量获取块
	pointer readBlock(string FileName, int offset);
	
	// 查看buff是否为满
	bool isFull();
	
	//获取索引的可插入的位置，如果仍有空间则返回块，如果没有则返回NULL
	pointer getBlankBlock(string filename);
	
	//传入一个filename先判断是否存在，如果不存在就新建一个file，然后将第一个block读出来，如果存在，则将最近的可插入块读出来。
	pointer getFile(string filename);

	//从文件中和bufferlist里删除一个块
	void deleteBlock(string fileName, int offset);
};

/*class InsPos 
{
private:
int bufferNum;  //缓存中块位置
int blockOff;   //块内偏移量

public:
InsPos() { bufferNum = 0; blockOff = 0; }
int getBufferNum() { return bufferNum; }
int getBlockOff() { return blockOff; }
void setbufferNum(int num) { bufferNum = num; }
void setBlockOff(int off) { blockOff = off; }
};*/
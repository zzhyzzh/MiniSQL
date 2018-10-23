#pragma once
#include<stdio.h>
#include<string>
#include<vector>
#include"stdafx.h"
#define MAXBLOCKNUM 1024						//���建������໺�����Ŀ
using namespace std;

typedef Block *pointer;
typedef File *filePtr;

class BufferManager
{
public:
	Block *head = new Block;//�洢���л���������ͷ
	Block *tail = new Block;//�洢���л���������ͷ
	File *fileHead = new File;//�洢�ļ���Ϣ������
	int number;//bufferlist�ܳ���

	BufferManager();

	~BufferManager();

	//��ȡ�ļ�β���
	//int getlastEmptyBlock(string fileName);

	//����д���ļ�
	void writeBlockToFile(pointer bufferBlock);
	
	//�����п�д�ص��ļ���ȥ
	void writeBufferToFile();
	
	//�����п�д�ص��ļ���ȥ�����ͷ�buffer��
	void releaseBufferBlocks();
	
	//�����ļ�����ƫ������ȡ��
	pointer readBlock(string FileName, int offset);
	
	// �鿴buff�Ƿ�Ϊ��
	bool isFull();
	
	//��ȡ�����Ŀɲ����λ�ã�������пռ��򷵻ؿ飬���û���򷵻�NULL
	pointer getBlankBlock(string filename);
	
	//����һ��filename���ж��Ƿ���ڣ���������ھ��½�һ��file��Ȼ�󽫵�һ��block��������������ڣ�������Ŀɲ�����������
	pointer getFile(string filename);

	//���ļ��к�bufferlist��ɾ��һ����
	void deleteBlock(string fileName, int offset);
};

/*class InsPos 
{
private:
int bufferNum;  //�����п�λ��
int blockOff;   //����ƫ����

public:
InsPos() { bufferNum = 0; blockOff = 0; }
int getBufferNum() { return bufferNum; }
int getBlockOff() { return blockOff; }
void setbufferNum(int num) { bufferNum = num; }
void setBlockOff(int off) { blockOff = off; }
};*/
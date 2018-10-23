#include"IndexManager.h"
#include"RecordManager.h"
#include <sstream>
extern RecordManager recordManager;
extern BufferManager bufferManager;  //定义bufferManager 类的全局变量
extern CatalogManager *catalogManager;
/*
create index若发现已有index,则报错,index初始块由内部buffer分配
api需要建立tableInfo和indexInfo
*/
using namespace std;
void IndexManager::createIndex(Table tableInfo, Index indexInfo) //创建索引, indexInfo->blocknum = 已建立的索引数
{
	Block *newblock = new Block;
	Block *recordBlock = new Block; //测试
	int order, i, j, numOfRecord, offset, m, index_offset, tableBlock;
	string tmp; //存储临时字符串
	j = 0;
	offset = 0;
	string value = indexInfo.value;
	string fileName = "file/index/" + tableInfo.name + "_" + indexInfo.name + ".txt"; //文件名格式: 表名_索引名

	newblock = bufferManager.getFile(fileName); //BM待定义,indexInfo->blockNum为最新的block偏移量
												//catalogManager->addIndexBlockNum(indexInfo.value);
	index_offset = newblock->blockNum; //记录所分配的block偏移量
									   //	indexInfo->blockNum = index_offset;
	fileName = "file/record/" + tableInfo.name + ".txt";   //从记录文件名中获取数据
	for (i = 0; i < tableInfo.attrNum; i++) //寻找索引在属性中的位置
	{

		if (indexInfo.name == tableInfo.attributes[i].name)
		{
			j = offset + 4;  //找到索引的位置,此处暂定 offset +　4，因为不确定record头文件
			break;
		}
		offset += tableInfo.attributes[i].length;
	}

	for (tableBlock = 0; tableBlock < tableInfo.blockNum; tableBlock++)
	{
		numOfRecord = 0;
		j = offset + 4;
		recordBlock = bufferManager.readBlock(fileName, tableBlock);  //BM待定义,文件偏移量待考虑,
																	  //cout << recordBlock->CBlock << endl;
		for (i = 0; i < 4; i++)
		{
			numOfRecord = 10 * numOfRecord + recordBlock->CBlock[i] - '0';  //计算总记录数
		}
		//cout << numOfRecord << endl;
		for (i = 0; i < numOfRecord; i++) // 
		{
			tmp.clear();
			for (m = j; m < j + indexInfo.length; m++)   //将index循环插入B+树中
			{
				tmp = tmp + recordBlock->CBlock[m];
			}

			insertKey(indexInfo, tmp, tableBlock, j);
			j += tableInfo.totalLength;
		}
	}

	//record前4个记录数

}
void IndexManager::dropIndex(Index indexInfo)
{

	string fileName = "file/index/" + indexInfo.tableName + "_" + indexInfo.name + ".txt";

	Block* temp = bufferManager.head;

	while (temp->next)
	{
		if (fileName == temp->next->fileName)
		{
			Block* blockToDelete = new Block;
			blockToDelete = temp->next;
			temp->next = temp->next->next;
			delete blockToDelete;
			bufferManager.number--;
		}
		else temp = temp->next;
	}

	File *fileTemp = bufferManager.fileHead;
	//从文件空块记录中删除该块文件
	while (fileTemp->next)
	{
		if (fileTemp->next->fileName == fileName)//在文件链表中找到相应文件
		{
			File *temp2 = new File;
			temp2 = fileTemp->next;
			fileTemp->next = fileTemp->next->next;
			delete temp2;
		}
		else fileTemp = fileTemp->next;
	}
	remove(fileName.c_str());
}
void IndexManager::insertKey(Index indexInfo, string key, int blockOffset, int offset)
{
	Block newblock;
	string fileName = "file/index/" + indexInfo.tableName + "_" + indexInfo.name + ".txt";
	BPlusTree BPtree(indexInfo);
	BPtree.insert(key, blockOffset, offset);

}
void IndexManager::searchEqual(Table tableInfo, Index indexInfo, string key)
{
	int i, bnum, length, start, tmpLength;
	Record res;
	Block *newblock = new Block;
	Block *recordBlock = new Block;
	Offset off;
	string fileName = "file/record/" + tableInfo.name + ".txt";   //从记录文件名中获取数据
	BPlusTree BPtree(indexInfo);
	off = BPtree.searchEqual(key);
	if (off.blockNum == -1 && off.offset == -1)
	{
		recordManager.titleOutput(tableInfo);
		return;
	}
	recordBlock = bufferManager.readBlock(fileName, off.blockNum);

	string info = recordBlock->CBlock;
	res.blockNum = off.blockNum;
	res.offset = off.offset;
	tmpLength = 0;
	for (i = 0; i < tableInfo.attrNum; i++)
	{
		if (tableInfo.attributes[i].name == indexInfo.name)
		{
			break;
		}
		tmpLength += tableInfo.attributes[i].length;
	}

	start = res.offset - tmpLength;
	for (i = 0; i < tableInfo.attrNum; i++)
	{
		
		length = tableInfo.attributes[i].length;
		string tmpStr = info.substr(start, length);
		res.columns.push_back(tmpStr);
		start += length;
	}
	
	//输出表头
	recordManager.titleOutput(tableInfo);

	string tempRecord = res.getString();

	//输出记录
	recordManager.recordOutput(tableInfo, tempRecord);
}

void IndexManager::deleteKey(Index indexInfo, string deletedKey)
{
	BPlusTree BPtree(indexInfo);
	BPtree.deleteOne(deletedKey);
}

void IndexManager::updateKey(Index indexInfo, string key, int blockOffset, int offset)
{
	BPlusTree BPtree(indexInfo);
	BPtree.updateKey(key, blockOffset, offset);
}

void IndexManager::deleteAllIndex(Table tableInfo, Index indexInfo)
{
	string fileName = "file/index/" + indexInfo.tableName + "_" + indexInfo.name + ".txt";

	Block* temp = bufferManager.head;

	while (temp->next)
	{
		if (fileName == temp->next->fileName)
		{
			Block* blockToDelete = new Block;
			blockToDelete = temp->next;
			temp->next = temp->next->next;
			delete blockToDelete;
			bufferManager.number--;
		}
		else temp = temp->next;
	}

	File *fileTemp = bufferManager.fileHead;
	//从文件空块记录中删除该块文件
	while (fileTemp->next)
	{
		if (fileTemp->next->fileName == fileName)//在文件链表中找到相应文件
		{
			File *temp2 = new File;
			temp2 = fileTemp->next;
			fileTemp->next = fileTemp->next->next;
			delete temp2;
		}
		else fileTemp = fileTemp->next;
	}
	remove(fileName.c_str());
	createIndex(tableInfo, indexInfo);
	catalogManager->indexBlockNumRefresh(indexInfo);
}

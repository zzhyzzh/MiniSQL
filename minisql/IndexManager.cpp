#include"IndexManager.h"
#include"RecordManager.h"
#include <sstream>
extern RecordManager recordManager;
extern BufferManager bufferManager;  //����bufferManager ���ȫ�ֱ���
extern CatalogManager *catalogManager;
/*
create index����������index,�򱨴�,index��ʼ�����ڲ�buffer����
api��Ҫ����tableInfo��indexInfo
*/
using namespace std;
void IndexManager::createIndex(Table tableInfo, Index indexInfo) //��������, indexInfo->blocknum = �ѽ�����������
{
	Block *newblock = new Block;
	Block *recordBlock = new Block; //����
	int order, i, j, numOfRecord, offset, m, index_offset, tableBlock;
	string tmp; //�洢��ʱ�ַ���
	j = 0;
	offset = 0;
	string value = indexInfo.value;
	string fileName = "file/index/" + tableInfo.name + "_" + indexInfo.name + ".txt"; //�ļ�����ʽ: ����_������

	newblock = bufferManager.getFile(fileName); //BM������,indexInfo->blockNumΪ���µ�blockƫ����
												//catalogManager->addIndexBlockNum(indexInfo.value);
	index_offset = newblock->blockNum; //��¼�������blockƫ����
									   //	indexInfo->blockNum = index_offset;
	fileName = "file/record/" + tableInfo.name + ".txt";   //�Ӽ�¼�ļ����л�ȡ����
	for (i = 0; i < tableInfo.attrNum; i++) //Ѱ�������������е�λ��
	{

		if (indexInfo.name == tableInfo.attributes[i].name)
		{
			j = offset + 4;  //�ҵ�������λ��,�˴��ݶ� offset +��4����Ϊ��ȷ��recordͷ�ļ�
			break;
		}
		offset += tableInfo.attributes[i].length;
	}

	for (tableBlock = 0; tableBlock < tableInfo.blockNum; tableBlock++)
	{
		numOfRecord = 0;
		j = offset + 4;
		recordBlock = bufferManager.readBlock(fileName, tableBlock);  //BM������,�ļ�ƫ����������,
																	  //cout << recordBlock->CBlock << endl;
		for (i = 0; i < 4; i++)
		{
			numOfRecord = 10 * numOfRecord + recordBlock->CBlock[i] - '0';  //�����ܼ�¼��
		}
		//cout << numOfRecord << endl;
		for (i = 0; i < numOfRecord; i++) // 
		{
			tmp.clear();
			for (m = j; m < j + indexInfo.length; m++)   //��indexѭ������B+����
			{
				tmp = tmp + recordBlock->CBlock[m];
			}

			insertKey(indexInfo, tmp, tableBlock, j);
			j += tableInfo.totalLength;
		}
	}

	//recordǰ4����¼��

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
	//���ļ��տ��¼��ɾ���ÿ��ļ�
	while (fileTemp->next)
	{
		if (fileTemp->next->fileName == fileName)//���ļ��������ҵ���Ӧ�ļ�
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
	string fileName = "file/record/" + tableInfo.name + ".txt";   //�Ӽ�¼�ļ����л�ȡ����
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
	
	//�����ͷ
	recordManager.titleOutput(tableInfo);

	string tempRecord = res.getString();

	//�����¼
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
	//���ļ��տ��¼��ɾ���ÿ��ļ�
	while (fileTemp->next)
	{
		if (fileTemp->next->fileName == fileName)//���ļ��������ҵ���Ӧ�ļ�
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

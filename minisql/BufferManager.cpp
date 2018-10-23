#include<iostream> 
#include<fstream>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sstream>
#include"BufferManager.h"
extern BufferManager buffer;

BufferManager::BufferManager()
{
	string path = "file/EmptyBlockRecord.txt";

	number = 0;
	head->next = NULL;
	tail->next = head;
	fileHead->next = NULL;

	filePtr tempFile = fileHead;
	FILE *fp;
	if (fp = fopen(path.c_str(), "rb+"))
	{
		char* ch = new char[1];
		char* temp = new char;
		char* temp2 = new char;
		string totalRecord = "";//��ȡ�ļ���ȫ����Ϣ
		vector<string> records;//��ִ洢������Ϣ

		*ch = fgetc(fp);


		//��ȡ�ļ���ȫ����Ϣ
		while (*ch != '$')
		{
			totalRecord.append(1, *ch);
			//fseek(fp, 1, SEEK_CUR);
			*ch = fgetc(fp);
		}

		int i = 0;
		//��ִ洢������Ϣ
		while (i < totalRecord.length())
		{
			*temp = totalRecord[i];
			string tempFileRecord = "";
			while (*temp != ';')//��Ϊ��;��
			{
				tempFileRecord += totalRecord[i];
				i++;
				*temp = totalRecord[i];
			}
			records.push_back(tempFileRecord);
			i++;
		}

		//�������м�¼
		for (int j = 0; j < records.size(); j++)
		{
			tempFile->next = new File;
			string tempRecord = records[j];
			vector<string> emptyBlocks;
			int m = 0;
			//������ǰ��¼�ڲ�
			*temp2 = tempRecord[m];
			string fileName = "";
			string lastBlock = "";
			while (*temp2 != '@')
			{
				fileName.append(1, *temp2);
				m++;
				*temp2 = tempRecord[m];
			}

			m++;
			*temp2 = tempRecord[m];
			while (*temp2 != '!')
			{
				lastBlock.append(1, *temp2);
				m++;
				*temp2 = tempRecord[m];
			}
			m++;
			*temp2 = tempRecord[m];
			int n = 0;
			while (m < tempRecord.length())
			{
				emptyBlocks.push_back("");
				while (*temp2 != '#')
				{
					emptyBlocks[n].append(1, *temp2);
					m++;
					*temp2 = tempRecord[m];
				}
				m++;
				*temp2 = tempRecord[m];
			}
			tempFile->next->fileName = fileName;
			tempFile->next->lastEmptyBlock = atoi(lastBlock.c_str());
			for (int i = 0; i < emptyBlocks.size(); i++)
			{
				tempFile->next->emptyBlocks.push_back(atoi(emptyBlocks[i].c_str()));
			}
			tempFile = tempFile->next;
		}
		tempFile->next = NULL;
	}
}

BufferManager::~BufferManager()
{
	releaseBufferBlocks();
	string path = "file/EmptyBlockRecord.txt";
	string input = "";
	vector<filePtr> emptyRecord;

	ofstream f1(path);//���ļ�
	f1.close();

	filePtr temp = fileHead;
	while (temp->next)
	{
		emptyRecord.push_back(temp->next);
		temp = temp->next;
	}

	FILE *fp;
	if (fp = fopen(path.c_str(), "rb+"))
	{
		for (int i = 0; i < emptyRecord.size(); i++)
		{
			input += emptyRecord[i]->fileName + "@";

			stringstream number;
			string str;
			number << emptyRecord[i]->lastEmptyBlock;
			number >> str;

			input += str + "!";

			for (int j = 0; j < emptyRecord[i]->emptyBlocks.size(); j++)
			{
				stringstream ss;
				string str;
				ss << emptyRecord[i]->emptyBlocks[j];
				ss >> str;
				input += str + "#";
			}
			input += ";";
		}
		input += "$";
		fwrite(input.c_str(), input.length(), 1, fp);
		fclose(fp);
	}
}

//����д���ļ�
void BufferManager::writeBlockToFile(pointer bufferBlock)
{
	if (bufferBlock->isDirty)
	{
		FILE *fp;
		if (fp = fopen(bufferBlock->fileName.c_str(), "rb+"))
		{
			fseek(fp, BLOCK_SIZE * bufferBlock->blockNum, SEEK_SET);
			fwrite(bufferBlock->CBlock, BLOCK_SIZE, 1, fp);
			fclose(fp);
		}
	}
}

//�����п�д���ļ�
void BufferManager::writeBufferToFile()
{
	pointer temp = head;
	while (temp->next)
	{
		writeBlockToFile(temp->next);
		temp = temp->next;
	}
}

//�����п�д���ļ������ͷ�buffer������������
void BufferManager::releaseBufferBlocks()
{
	writeBufferToFile();
	pointer temp = head;
	while (temp->next)
	{
		pointer temp2 = temp;
		temp = temp->next;
		delete temp2;
	}
	//delete temp->next;
}

//�����ļ�����ƫ������ȡ��
pointer BufferManager::readBlock(string FileName, int offset)
{
	pointer temp = head;
	pointer tempReserved = new Block;
	//�������ȡ����buffer�д��ڣ����ÿ��������ͷ�������ظÿ�
	while (temp->next)
	{
		if (temp->next->fileName == FileName && temp->next->blockNum == offset)
		{
			if (temp == head)//���bufferlist��ֻ��һ���飬Ҫ��ȡ�����
				return head->next;
			else//bufferlist���ж����
			{
				if (temp->next == tail->next)
					tail->next = temp;//Ҫ��ȡĩβ��ʱ��ĩβ����ֲͷ����tailָ����Ҫ����ָ��ĩβ�����һ����
				pointer temp2 = temp->next->next;
				temp->next->next = head->next;
				head->next = temp->next;
				temp->next = temp2;//��������ָ�룬������ȡ�Ŀ���������ͷ��
				return head->next;
			}
		}
		else
		{
			tempReserved = temp;
			temp = temp->next;
		}
	}

	//��������ڣ����ļ��ж�ȡ�ÿ�
	FILE *fp;
	if (fp = fopen(FileName.c_str(), "rb+"))
	{
		filePtr temp = fileHead;
		while (temp->next)
		{
			if (temp->next->fileName == FileName)//���ļ��������ҵ���Ӧ�ļ�
			{
				int i;
				for (i = 0; i < temp->next->emptyBlocks.size(); i++)//�ڿտ��¼�в��ң���ǰ��ȡ�Ŀ��Ƿ�Ϊ�տ�
					if (offset == temp->next->emptyBlocks[i])
					{
						//cout << "���ڶ�ȡ�Ŀ�Ϊ" << FileName << "�еĵ�" << offset << "�飬�ÿ鵱ǰΪ�տ飬readBlock���������Զ�ȡ������NULL����ʹ��getBlankBlock������ȡ�տ顣" << endl;
						fclose(fp);
						return NULL;
					}
				if (i == temp->next->emptyBlocks.size() && offset >= temp->next->lastEmptyBlock)//�ڿտ��¼�в��ң���ǰ��ȡ�Ŀ��Ƿ�Ϊ��λ���ļ�ĩβ֮��ģ��տ�
				{
					//cout << "���ڶ�ȡ�Ŀ�Ϊ" << FileName << "�еĵ�" << offset << "�飬�ÿ鵱ǰΪ�ļ�β���տ飬readBlock���������Զ�ȡ������NULL����ʹ��getBlankBlock������ȡ�տ顣" << endl;
					fclose(fp);
					return NULL;
				}
				break;
			}
			else temp = temp->next;
		}

		//���buffer��ʣ��Ŀռ�
		if (!isFull())
		{
			pointer newBlock = new Block;
			fseek(fp, BLOCK_SIZE * offset, SEEK_SET);
			fread(newBlock->CBlock, BLOCK_SIZE, 1, fp);	//���ļ��еĿ�д��newBlock����newBlock��������ͷ��
			newBlock->setBlockLocation(FileName, offset);
			if (newBlock->isDirty)
				newBlock->charNum = strlen(newBlock->CBlock);
			else//index��Ϊ��
				newBlock->charNum = 0;
			if (!head->next)//����Ϊ��ʱ����Ҫ�ı�tail��λ��
				tail->next = newBlock;
			newBlock->next = head->next;
			head->next = newBlock;
			number++;
			fclose(fp);
			return head->next;
		}
		//buffer��ʣ��ռ䣬��ĩβ������Ϣд�أ����ļ��еĿ�д��ĩβ�飬��������ͷ��
		else
		{
			writeBlockToFile(tail->next);//��ĩβ������Ϣд��

			tail->next->setBlockLocation(FileName, offset);//���¶�ȡ�Ŀ�д��ĩβ�飬ĩβ�����Ϣ����Ӧ����
			fseek(fp, BLOCK_SIZE * offset, SEEK_SET);
			fread(tail->next->CBlock, BLOCK_SIZE, 1, fp);
			tail->next->charNum = strlen(tail->next->CBlock);

			tail->next = tempReserved;			//��tailָ��ָ��ĩβ�����һ���飺tempReserved
			tail->next->next->next = head->next;
			head->next = tail->next->next;
			tail->next->next = NULL;
			fclose(fp);
			return head->next;
		}
	}
	else if (fp == NULL)
	{
		//cout << "���ڶ�ȡ���ļ���Ϊ" << FileName << "����ǰ����������ļ���readBlock���������Զ�ȡ������NULL����ʹ��getFile���������ļ���" << endl;
		return NULL;
	}
}

// �鿴buff�Ƿ�Ϊ��
bool BufferManager::isFull()
{
	if (number == MAXBLOCKNUM)
		return true;
	else return false;
}

//��ȡ�����Ŀɲ����λ�ã�������пռ��򷵻ؿ飬���û���򷵻�NULL
pointer BufferManager::getBlankBlock(string filename)
{
	int chosenBlock;
	filePtr temp = fileHead;

	while (temp->next)
	{
		if (temp->next->fileName == filename)//���ļ��������ҵ���Ӧ�ļ�
		{
			if (temp->next->emptyBlocks.size())//����ļ��м䲿���пտ飨����Ϣ��emptyBlocks��¼��ÿ��deleteʱ׷����Ϣ��
			{
				chosenBlock = temp->next->emptyBlocks[temp->next->emptyBlocks.size() - 1];
				temp->next->emptyBlocks.pop_back();
				goto next;
			}
			else//����ļ��м䲿��û�пտ飬�տ���ļ�ĩβ��á�ĩβ����lastEmptyBlock��¼
			{
				chosenBlock = temp->next->lastEmptyBlock;
				temp->next->lastEmptyBlock++;
				goto next;
			}
		}
		else temp = temp->next;
	}
	if (!temp->next)//���ļ�������û����Ӧ�ļ�����Ϣ
	{
		temp->next = new File(uncertain, filename);//������Ӧ��Ϣ
		chosenBlock = temp->next->lastEmptyBlock;
		temp->next->lastEmptyBlock++;
	}

next:
	FILE *fp;
	if (fp = fopen(filename.c_str(), "rb+"))
	{
		fseek(fp, BLOCK_SIZE * chosenBlock, SEEK_SET);
		if (!isFull())
		{
			pointer blankBlock = new Block;
			fread(blankBlock->CBlock, BLOCK_SIZE, 1, fp);

			blankBlock->setBlockLocation(filename, chosenBlock);

			if (!head->next)//����Ϊ��ʱ����Ҫ�ı�tail��λ��
				tail->next = blankBlock;
			blankBlock->next = head->next;
			head->next = blankBlock;
			number++;
			fclose(fp);
			return head->next;
		}
		else//bufferlist����
		{
			writeBlockToFile(tail->next);//��ĩβ������Ϣд��

			tail->next->setBlockLocation(filename, chosenBlock);//���¶�ȡ�Ŀտ�д��ĩβ�飬ĩβ�����Ϣ����Ӧ����
			fread(tail->next->CBlock, BLOCK_SIZE, 1, fp);

			pointer temp = head;
			pointer tempReserved = new Block;
			while (temp->next)
			{
				tempReserved = temp;
				temp = temp->next;
			}//�ҵ�ĩβ�����һ���飨�������ǵ������ǵ�������ֻ�������ַ�ʽ�ҵ���
			tail->next = tempReserved;//��tailָ��ָ��ĩβ�����һ���飺tempReserved
			tail->next->next->next = head->next;
			head->next = tail->next->next;
			tail->next->next = NULL;
			fclose(fp);
			return head->next;
		}
	}
}

//�½�һ���ļ���Ȼ�󽫵�һ��block������
pointer BufferManager::getFile(string filename)
{
	ofstream f1(filename);//���ļ������ļ������ھʹ�����
	f1.close();
	return getBlankBlock(filename);
}

//���ļ��к�bufferlist��ɾ��һ����
void BufferManager::deleteBlock(string fileName, int offset)
{
	//��bufferlist��ɾ����
	if (readBlock(fileName, offset))
	{
		pointer blockToDelete = new Block;
		blockToDelete = head->next;
		head->next = head->next->next;
		delete blockToDelete;
		number--;
	}
	File *filetemp = fileHead;
	//���ļ���ɾ���ÿ�
	FILE *fp;

	if (fp = fopen(fileName.c_str(), "rb+"))
	{
		while (filetemp->next)
		{
			if (filetemp->next->fileName == fileName)//���ļ��������ҵ���Ӧ�ļ�
			{
				filetemp->next->emptyBlocks.push_back(offset);
				break;
			}
			else filetemp = filetemp->next;
		}
		fclose(fp);
	}
}
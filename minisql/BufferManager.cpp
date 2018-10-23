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
		string totalRecord = "";//提取文件中全部信息
		vector<string> records;//拆分存储各条信息

		*ch = fgetc(fp);


		//提取文件中全部信息
		while (*ch != '$')
		{
			totalRecord.append(1, *ch);
			//fseek(fp, 1, SEEK_CUR);
			*ch = fgetc(fp);
		}

		int i = 0;
		//拆分存储各条信息
		while (i < totalRecord.length())
		{
			*temp = totalRecord[i];
			string tempFileRecord = "";
			while (*temp != ';')//不为“;”
			{
				tempFileRecord += totalRecord[i];
				i++;
				*temp = totalRecord[i];
			}
			records.push_back(tempFileRecord);
			i++;
		}

		//遍历所有记录
		for (int j = 0; j < records.size(); j++)
		{
			tempFile->next = new File;
			string tempRecord = records[j];
			vector<string> emptyBlocks;
			int m = 0;
			//遍历当前记录内部
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

	ofstream f1(path);//打开文件
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

//将块写回文件
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

//将所有块写回文件
void BufferManager::writeBufferToFile()
{
	pointer temp = head;
	while (temp->next)
	{
		writeBlockToFile(temp->next);
		temp = temp->next;
	}
}

//将所有块写回文件，并释放buffer区（销毁链表）
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

//根据文件名和偏移量获取块
pointer BufferManager::readBlock(string FileName, int offset)
{
	pointer temp = head;
	pointer tempReserved = new Block;
	//如果所获取块在buffer中存在，将该块放置链表头部，返回该块
	while (temp->next)
	{
		if (temp->next->fileName == FileName && temp->next->blockNum == offset)
		{
			if (temp == head)//如果bufferlist中只有一个块，要获取这个块
				return head->next;
			else//bufferlist中有多个块
			{
				if (temp->next == tail->next)
					tail->next = temp;//要获取末尾块时，末尾块移植头部，tail指针需要重新指向末尾块的上一个块
				pointer temp2 = temp->next->next;
				temp->next->next = head->next;
				head->next = temp->next;
				temp->next = temp2;//调整链表指针，将所获取的块移至链表头部
				return head->next;
			}
		}
		else
		{
			tempReserved = temp;
			temp = temp->next;
		}
	}

	//如果不存在，从文件中读取该块
	FILE *fp;
	if (fp = fopen(FileName.c_str(), "rb+"))
	{
		filePtr temp = fileHead;
		while (temp->next)
		{
			if (temp->next->fileName == FileName)//在文件链表中找到相应文件
			{
				int i;
				for (i = 0; i < temp->next->emptyBlocks.size(); i++)//在空块记录中查找，当前读取的块是否为空块
					if (offset == temp->next->emptyBlocks[i])
					{
						//cout << "正在读取的块为" << FileName << "中的第" << offset << "块，该块当前为空块，readBlock函数不予以读取并返回NULL。请使用getBlankBlock函数获取空块。" << endl;
						fclose(fp);
						return NULL;
					}
				if (i == temp->next->emptyBlocks.size() && offset >= temp->next->lastEmptyBlock)//在空块记录中查找，当前读取的块是否为（位于文件末尾之后的）空块
				{
					//cout << "正在读取的块为" << FileName << "中的第" << offset << "块，该块当前为文件尾部空块，readBlock函数不予以读取并返回NULL。请使用getBlankBlock函数获取空块。" << endl;
					fclose(fp);
					return NULL;
				}
				break;
			}
			else temp = temp->next;
		}

		//如果buffer有剩余的空间
		if (!isFull())
		{
			pointer newBlock = new Block;
			fseek(fp, BLOCK_SIZE * offset, SEEK_SET);
			fread(newBlock->CBlock, BLOCK_SIZE, 1, fp);	//将文件中的块写入newBlock，将newBlock放置链表头部
			newBlock->setBlockLocation(FileName, offset);
			if (newBlock->isDirty)
				newBlock->charNum = strlen(newBlock->CBlock);
			else//index块为空
				newBlock->charNum = 0;
			if (!head->next)//链表为空时，需要改变tail的位置
				tail->next = newBlock;
			newBlock->next = head->next;
			head->next = newBlock;
			number++;
			fclose(fp);
			return head->next;
		}
		//buffer无剩余空间，将末尾块内信息写回，将文件中的块写入末尾块，放置链表头部
		else
		{
			writeBlockToFile(tail->next);//将末尾块内信息写回

			tail->next->setBlockLocation(FileName, offset);//将新读取的块写入末尾块，末尾块的信息做相应调整
			fseek(fp, BLOCK_SIZE * offset, SEEK_SET);
			fread(tail->next->CBlock, BLOCK_SIZE, 1, fp);
			tail->next->charNum = strlen(tail->next->CBlock);

			tail->next = tempReserved;			//将tail指针指向末尾块的上一个块：tempReserved
			tail->next->next->next = head->next;
			head->next = tail->next->next;
			tail->next->next = NULL;
			fclose(fp);
			return head->next;
		}
	}
	else if (fp == NULL)
	{
		//cout << "正在读取的文件名为" << FileName << "。当前不存在这个文件，readBlock函数不予以读取并返回NULL。请使用getFile函数创建文件。" << endl;
		return NULL;
	}
}

// 查看buff是否为满
bool BufferManager::isFull()
{
	if (number == MAXBLOCKNUM)
		return true;
	else return false;
}

//获取索引的可插入的位置，如果仍有空间则返回块，如果没有则返回NULL
pointer BufferManager::getBlankBlock(string filename)
{
	int chosenBlock;
	filePtr temp = fileHead;

	while (temp->next)
	{
		if (temp->next->fileName == filename)//在文件链表中找到相应文件
		{
			if (temp->next->emptyBlocks.size())//如果文件中间部分有空块（此信息由emptyBlocks记录，每次delete时追加信息）
			{
				chosenBlock = temp->next->emptyBlocks[temp->next->emptyBlocks.size() - 1];
				temp->next->emptyBlocks.pop_back();
				goto next;
			}
			else//如果文件中间部分没有空块，空块从文件末尾获得。末尾块由lastEmptyBlock记录
			{
				chosenBlock = temp->next->lastEmptyBlock;
				temp->next->lastEmptyBlock++;
				goto next;
			}
		}
		else temp = temp->next;
	}
	if (!temp->next)//若文件链表中没有相应文件的信息
	{
		temp->next = new File(uncertain, filename);//创建对应信息
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

			if (!head->next)//链表为空时，需要改变tail的位置
				tail->next = blankBlock;
			blankBlock->next = head->next;
			head->next = blankBlock;
			number++;
			fclose(fp);
			return head->next;
		}
		else//bufferlist已满
		{
			writeBlockToFile(tail->next);//将末尾块内信息写回

			tail->next->setBlockLocation(filename, chosenBlock);//将新读取的空块写入末尾块，末尾块的信息做相应调整
			fread(tail->next->CBlock, BLOCK_SIZE, 1, fp);

			pointer temp = head;
			pointer tempReserved = new Block;
			while (temp->next)
			{
				tempReserved = temp;
				temp = temp->next;
			}//找到末尾块的上一个块（由于我们的链表是单向链表，只能用这种方式找到）
			tail->next = tempReserved;//将tail指针指向末尾块的上一个块：tempReserved
			tail->next->next->next = head->next;
			head->next = tail->next->next;
			tail->next->next = NULL;
			fclose(fp);
			return head->next;
		}
	}
}

//新建一个文件，然后将第一个block读出来
pointer BufferManager::getFile(string filename)
{
	ofstream f1(filename);//打开文件，若文件不存在就创建它
	f1.close();
	return getBlankBlock(filename);
}

//从文件中和bufferlist里删除一个块
void BufferManager::deleteBlock(string fileName, int offset)
{
	//从bufferlist中删除块
	if (readBlock(fileName, offset))
	{
		pointer blockToDelete = new Block;
		blockToDelete = head->next;
		head->next = head->next->next;
		delete blockToDelete;
		number--;
	}
	File *filetemp = fileHead;
	//从文件中删除该块
	FILE *fp;

	if (fp = fopen(fileName.c_str(), "rb+"))
	{
		while (filetemp->next)
		{
			if (filetemp->next->fileName == fileName)//在文件链表中找到相应文件
			{
				filetemp->next->emptyBlocks.push_back(offset);
				break;
			}
			else filetemp = filetemp->next;
		}
		fclose(fp);
	}
}
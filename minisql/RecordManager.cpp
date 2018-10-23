#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include<string.h>
#include"BufferManager.h"
#include"RecordManager.h"

using namespace std;

extern BufferManager bufferManager;
extern CatalogManager *catalogManager;
extern IndexManager indexManager;

//创建表
void RecordManager::createTable(Table &tableInfo) //空表的块数记做1
{
	string path = "file/record/" + tableInfo.name + ".txt";

	bufferManager.getFile(path);//文件创建，返回空块
	bufferManager.head->next->type = record;
	bufferManager.head->next->setRecordNum(0);//初始化块中记录数为0
											  //tableInfo.blockNum = 1;	
}

//删除表，即删除表文件
void RecordManager::dropTable(Table &tableInfo)
{
	string path;
	path = "file/record/" + tableInfo.name + ".txt";

	//if (remove(path.c_str()))//删除失败
	//{
	//	//cout << "The table" << tableInfo.name << "does not exist." << endl;
	//	perror("remove");
	//	//system("pause");
	//}//cout << "The table" << tableInfo.name << "has been deleted successfully" << endl;
	//

	pointer temp = bufferManager.head;
	//从bufferlist中删除该文件所属的块
	while (temp->next)
	{
		if (path == temp->next->fileName)
		{
			pointer blockToDelete = new Block;
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
		if (fileTemp->next->fileName == path)//在文件链表中找到相应文件
		{
			File *temp2 = new File;
			temp2 = fileTemp->next;
			fileTemp->next = fileTemp->next->next;
			delete temp2;
		}
		else fileTemp = fileTemp->next;
	}
}

//内部调用：计算块中能存储的最大记录数
int RecordManager::maxRecordNumberPerBlock(Table tableInfo)
{
	return (BLOCK_SIZE - 4) / tableInfo.totalLength;//单条记录的长度
}

//内部调用：如果表文件中某个块已满，返回当前块中最后一条记录
insertPointer RecordManager::lastRecordInBlock(Table tableInfo, int blockNum)
{
	string fileName = "file/record/" + tableInfo.name + ".txt";

	if (bufferManager.readBlock(fileName, blockNum))//如果读取成功，不为空块
	{
		string temp = bufferManager.head->next->CBlock + 4 + (maxRecordNumberPerBlock(tableInfo) - 1)*tableInfo.totalLength;
		//从块中读出记录数信息最后一条记录
		string str = temp.substr(0, tableInfo.totalLength);

		insertPointer head = new insertVal;
		head->next = NULL;
		int currentPosition = 0;
		insertPointer temp1 = head;
		for (int i = 0; i < tableInfo.attributes.size(); i++)
		{
			insertPointer temp2 = new insertVal;
			temp2->next = temp1->next;
			temp1->next = temp2;
			temp2->value = str.substr(currentPosition, tableInfo.attributes[i].length);
			currentPosition += tableInfo.attributes[i].length;
			temp1 = temp1->next;
		}
		return head->next;
	}
	else cout << "当前块为空块" << endl;
}

//内部调用：返回当前块中第一条记录
insertPointer RecordManager::firstRecordInBlock(Table tableInfo, int blockNum)
{
	string fileName = "file/record/" + tableInfo.name + ".txt";

	if (bufferManager.readBlock(fileName, blockNum))//如果读取成功，不为空块
	{
		string temp = bufferManager.head->next->CBlock + 4;
		string str = temp.substr(0, tableInfo.totalLength);//从块中读出第一条记录

		insertPointer head = new insertVal;
		head->next = NULL;
		int currentPosition = 0;
		insertPointer temp1 = head;
		for (int i = 0; i < tableInfo.attributes.size(); i++)
		{
			insertPointer temp2 = new insertVal;
			temp2->next = temp1->next;
			temp1->next = temp2;
			temp2->value = str.substr(currentPosition, tableInfo.attributes[i].length);
			currentPosition += tableInfo.attributes[i].length;
			temp1 = temp1->next;
		}
		return head->next;
	}
	else cout << "当前块为空块" << endl;
}

//内部调用：判断块中记录是否已满
bool RecordManager::isFull(Table tableInfo, int blockNum)
{
	string fileName = "file/record/" + tableInfo.name + ".txt";

	if (bufferManager.readBlock(fileName, blockNum))//如果文件中该块不为空
	{
		int temp = bufferManager.head->next->getRecordNum();//从块中读出记录数信息
		if (temp == maxRecordNumberPerBlock(tableInfo))//与最大块数比较
			return true;
		else return false;
	}
}

//插入记录
void RecordManager::insertValue(Table &tableInfo, insertPointer InfoLine)
{
	Offset* record = insert(tableInfo, InfoLine);
	if (!record)
	{
		return;
	}
	else
	{
		cout << "Insert successfully." << endl;
		indexUpdateAfterInsert(tableInfo, *record);
	}
}

//内部调用：插入记录，不更新索引值
Offset* RecordManager::insert(Table &tableInfo, insertPointer InfoLine)
{
	string record = "";		//存储插入的完整记录
	insertPointer temp = InfoLine;
	while (temp)
	{
		record += temp->value;
		temp = temp->next;
	}

	//插入前的重复判断
	//扫描一遍所有属性，对于primary和unique就扫描整条记录，若发现插入的值与表中已有记录重复，则拒绝插入操作
	primaryKey inputPrimary(tableInfo, InfoLine);//用primaryKey类构造函数获取输入记录的主键值

	vector<unique> uniqueRecord;
	for (int i = 0; i < tableInfo.attributes.size(); i++)
	{
		unique inputUnique(tableInfo, i, record);//获取输入记录的unique值。如果不为unique，没有信息会被压入
		uniqueRecord.push_back(inputUnique);
	}

	Offset position;
	string fileName = "file/record/" + tableInfo.name + ".txt";
	string tempValue;		//存储某一个块中的全部值
	string tempRecord;		//存储某一个记录
	string tempElement;
	string tempPrimaryKey;
	string tempUnique;
	string tempIndexedValue; //存储某一个块中的带索引的属性值

							 //主键和unique重复判断
	bool ifRepeated = false;//标记是否重复。初始置为无重复
	for (int i = 0; i < tableInfo.blockNum; i++)
	{
		if (bufferManager.readBlock(fileName, i))
		{
			tempValue = bufferManager.head->next->CBlock + 4;//将块中记录暂存至tempString（除去前4个字节）

			for (int j = 0; j < bufferManager.head->next->getRecordNum()*tableInfo.totalLength; j += tableInfo.totalLength)
			{
				tempRecord = tempValue.substr(j, tableInfo.totalLength);//暂存一条记录至tempRecord
																		//int currentPosition = 0;
				for (int m = 0; m < tableInfo.attributes.size(); m++)
				{
					tempElement = tempRecord.substr(uniqueRecord[m].uniquePosition, tableInfo.attributes[m].length);//当前属性有条件判断
					if (tableInfo.attributes[m].isUnique)
					{
						ifRepeated = !strcmp(tempElement.c_str(), uniqueRecord[m].uniqueValue.c_str());
						if (ifRepeated)//有重复
						{
							if (tableInfo.attributes[m].isPrimeryKey)
								cout << "The value of the primary key already exists in the table." << endl;
							else
								cout << "The value of the unique attribute already exists in the table." << endl;
							return NULL;
						}
					}

				}
			}
		}
	}
	//执行插入
	for (int i = 0; i <= tableInfo.blockNum; i++)			//按照文件中块的顺序，将表文件中的块依次读入bufferList
															//record类文件中，保证块是顺序存储的
	{
		if (bufferManager.readBlock(fileName, i))				//如果文件中该块不为空
		{
			tempValue = bufferManager.head->next->CBlock + 4;//将块中记录暂存至tempString（除去前4个字节）
			int upBoundary;
			if (!isFull(tableInfo, i))
			{
				//if (bufferManager.head->next->getRecordNum() > 0)
				//	{
				upBoundary = bufferManager.head->next->getRecordNum()*tableInfo.totalLength;
				//	}
				//else//块存在但记录为空
				//{
				//	upBoundary = 0;
				//	bufferManager.head->next->setRecordNum(0);
				//}
			}
			else
			{
				upBoundary = (bufferManager.head->next->getRecordNum() - 1)*tableInfo.totalLength;
			}
			for (int j = 0; j <= upBoundary; j += tableInfo.totalLength)
			{
				tempRecord = tempValue.substr(j, tableInfo.totalLength);
				tempPrimaryKey = tempRecord.substr(inputPrimary.primaryPosition, inputPrimary.primaryLength);//该条记录的主键信息

				if (inputPrimary.PrimaryValue.compare(tempPrimaryKey) < 0)//要插入的主键值在字典序中排序位于当前记录主键值前
				{
					if (!isFull(tableInfo, i))//块中记录未满
					{
						tempValue.insert(j, record);//直接插入记录
						position.blockNum = i;
						position.offset = j + 4;//record类信息定义
						bufferManager.head->next->setRecordNumAddOne();//记录数加一
						bufferManager.head->next->setBlockValue(tempValue.substr(0, BLOCK_SIZE).c_str());
						goto Inserted;
					}
					else//如果块已满，需要向下一个块传递最后一条记录
					{
						insertPointer lastRecord = lastRecordInBlock(tableInfo, i);//获取最后一条记录
						tempValue.insert(j, record);//直接插入记录
						position.blockNum = i;
						position.offset = j + 4;//record类信息定义
						bufferManager.head->next->setBlockValue(tempValue.substr(0, BLOCK_SIZE).c_str());

						insert(tableInfo, lastRecord);//递归调用插入函数，将满块末尾处的记录重新插入
						goto Inserted;
					}
				}
			}
		}
		else//插入空块，直接写入，记录数至1
		{
			bufferManager.getBlankBlock(fileName);//获取空块

			catalogManager->addTableBlockNum(tableInfo.name); //表中块数+1

			bufferManager.head->next->setRecordNum(1);//记录数置一
			bufferManager.head->next->setBlockValue(record.c_str());//将单条记录写入空块
			position.blockNum = i;
			position.offset = 4;//record类信息定义
			goto Inserted;
		}
	}

Inserted://插入完成
	Offset* returnValue = new Offset;
	returnValue->blockNum = position.blockNum;
	returnValue->offset = position.offset;
	return returnValue;
}

//无条件查找全部属性输出
void RecordManager::select(Table tableInfo)
{
	string fileName = "file/record/" + tableInfo.name + ".txt";
	string tempValue;		//存储某一个块中的全部值
	string tempRecord;		//存储某一个记录
	string tempElement;

	titleOutput(tableInfo);

	for (int i = 0; i < tableInfo.blockNum; i++)			//按照文件中块的顺序，将表文件中的块依次读入bufferList
															//record类文件中，保证块是顺序存储的
	{
		if (bufferManager.readBlock(fileName, i))
		{
			tempValue = bufferManager.head->next->CBlock + 4;//将块中记录暂存至tempString（除去前4个字节）
			for (int j = 0; j < bufferManager.head->next->getRecordNum()*tableInfo.totalLength; j += tableInfo.totalLength)
			{
				tempRecord = tempValue.substr(j, tableInfo.totalLength);
				recordOutput(tableInfo, tempRecord);
			}
		}
	}
}

//无条件查找部分属性输出
void RecordManager::select(Table tableInfo, attrPointer selections)
{
	string fileName = "file/record/" + tableInfo.name + ".txt";
	string tempValue;		//存储某一个块中的全部值
	string tempRecord;		//存储某一个记录
	string tempElement;

	titleOutput(tableInfo, selections);

	for (int i = 0; i < tableInfo.blockNum; i++)			//按照文件中块的顺序，将表文件中的块依次读入bufferList
															//record类文件中，保证块是顺序存储的
	{
		if (bufferManager.readBlock(fileName, i))
		{
			tempValue = bufferManager.head->next->CBlock + 4;//将块中记录暂存至tempString（除去前4个字节）
			vector<string> items;
			for (int j = 0; j < bufferManager.head->next->getRecordNum()*tableInfo.totalLength; j += tableInfo.totalLength)
			{
				tempRecord = tempValue.substr(j, tableInfo.totalLength);

				int position = 0;
				for (int m = 0; m < tableInfo.attributes.size(); m++)
				{
					items.push_back(tempRecord.substr(position, tableInfo.attributes[m].length));
					position += tableInfo.attributes[m].length;
				}

				recordOutput(tableInfo, items, selections);

				for (int m = 0; m < tableInfo.attributes.size(); m++)
				{
					items.pop_back();
				}
			}
		}
	}
}

//条件查找全部属性输出
void RecordManager::select(Table tableInfo, condiPointer conditions)
{
	for (int i = 0; i < tableInfo.attributes.size(); i++)
	{
		condiPointer temp = conditions;
		while (temp)
		{
			if (temp->attrName == tableInfo.attributes[i].name)
			{
				tableInfo.attributes[i].factor.push_back(*temp);
			}
			temp = temp->next;
		}
	}

	string fileName = "file/record/" + tableInfo.name + ".txt";
	string tempValue;		//存储某一个块中的全部值
	string tempRecord;		//存储某一个记录
	string tempElement;


	titleOutput(tableInfo);

	for (int i = 0; i < tableInfo.blockNum; i++)			//按照文件中块的顺序，将表文件中的块依次读入bufferList
															//record类文件中，保证块是顺序存储的
	{
		if (bufferManager.readBlock(fileName, i))
		{
			tempValue = bufferManager.head->next->CBlock + 4;//将块中记录暂存至tempString（除去前4个字节）

			for (int j = 0; j < bufferManager.head->next->getRecordNum()*tableInfo.totalLength; j += tableInfo.totalLength)
			{
				tempRecord = tempValue.substr(j, tableInfo.totalLength);//暂存一条记录至tempRecord
				int n = 0;
				bool judge;

				//条件之间关系全部为and
				if (conditions->andor)
				{
					judge = true;//初始化为真。
					for (int m = 0; m < tableInfo.totalLength; m += tableInfo.attributes[n].length, n++)//逐一判断当前记录各属性值
					{
						if (tableInfo.attributes[n].factor.size())//判断当前属性是否有条件值
						{
							tempElement = tempRecord.substr(m, tableInfo.attributes[n].length);//当前属性有条件判断
																							   //将属性值取出，进行判断

							for (int p = 0; p < tableInfo.attributes[n].factor.size(); p++)//逐一访问定义在当前属性下的条件
							{
								int outcome;
								outcome = strcmp(tempElement.c_str(), tableInfo.attributes[n].factor[p].value.c_str());
								//outcome>0：大于；
								//outcome<0：小于；
								//outcome=0：等于；
								if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), ">"))
									judge = outcome > 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "<"))
									judge = outcome < 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), ">="))
									judge = outcome >= 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "<="))
									judge = outcome <= 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "="))
									judge = outcome == 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "<>"))
									judge = outcome != 0 ? true : false;
								if (!judge)
								{
									break;
								}
							}
							if (!judge)//扫描到当前属性值，已经不符合条件
							{
								break;
							}
						}
					}
				}
				//条件之间关系全部为or
				else
				{
					judge = false;//初始化为假。
					for (int m = 0; m < tableInfo.totalLength; m += tableInfo.attributes[n].length, n++)//逐一判断当前记录各属性值
					{
						if (tableInfo.attributes[n].factor.size())//判断当前属性是否有条件值
						{
							tempElement = tempRecord.substr(m, tableInfo.attributes[n].length);//当前属性有条件判断
																							   //将属性值取出，进行判断

							for (int p = 0; p < tableInfo.attributes[n].factor.size(); p++)//逐一访问定义在当前属性下的条件
							{
								int outcome;
								outcome = strcmp(tempElement.c_str(), tableInfo.attributes[n].factor[p].value.c_str());
								//outcome>0：大于；
								//outcome<0：小于；
								//outcome=0：等于；
								if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), ">"))
									judge = outcome > 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "<"))
									judge = outcome < 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), ">="))
									judge = outcome >= 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "<="))
									judge = outcome <= 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "="))
									judge = outcome == 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "<>"))
									judge = outcome != 0 ? true : false;
								if (judge)//扫描到当前属性值，已经符合条件
								{
									break;
								}
							}
							if (judge)//扫描到当前属性值，已经不符合条件
							{
								break;
							}
						}
					}
				}
				if (judge)
				{
					recordOutput(tableInfo, tempRecord);
				}
			}
		}
	}
}

// 条件查找部分属性输出
void RecordManager::select(Table tableInfo, condiPointer conditions, attrPointer selections)
{
	for (int i = 0; i < tableInfo.attributes.size(); i++)
	{
		condiPointer temp = conditions;
		while (temp)
		{
			if (temp->attrName == tableInfo.attributes[i].name)
			{
				tableInfo.attributes[i].factor.push_back(*temp);
			}
			temp = temp->next;
		}
	}

	string fileName = "file/record/" + tableInfo.name + ".txt";
	string tempValue;		//存储某一个块中的全部值
	string tempRecord;		//存储某一个记录
	string tempElement;
	attrPointer temp = selections;

	titleOutput(tableInfo, selections);

	for (int i = 0; i < tableInfo.blockNum; i++)			//按照文件中块的顺序，将表文件中的块依次读入bufferList
															//record类文件中，保证块是顺序存储的
	{
		if (bufferManager.readBlock(fileName, i))
		{
			tempValue = bufferManager.head->next->CBlock + 4;//将块中记录暂存至tempString（除去前4个字节）

			for (int j = 0; j < bufferManager.head->next->getRecordNum()*tableInfo.totalLength; j += tableInfo.totalLength)
			{
				tempRecord = tempValue.substr(j, tableInfo.totalLength);//暂存一条记录至tempRecord
				int n = 0;
				bool judge = true;//初始化为真。
				for (int m = 0; m < tableInfo.totalLength; m += tableInfo.attributes[n].length, n++)//逐一判断当前记录各属性值
				{
					if (tableInfo.attributes[n].factor.size())//判断当前属性是否有条件值
					{
						tempElement = tempRecord.substr(m, tableInfo.attributes[n].length);//当前属性有条件判断，将属性值取出，进行判断

						for (int p = 0; p < tableInfo.attributes[n].factor.size(); p++)//逐一访问定义在当前属性下的条件
						{
							int outcome;
							outcome = strcmp(tempElement.c_str(), tableInfo.attributes[n].factor[p].value.c_str());
							//outcome>0：大于；
							//outcome<0：小于；
							//outcome=0：等于；
							if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), ">"))
								judge = outcome > 0 ? true : false;
							else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "<"))
								judge = outcome < 0 ? true : false;
							else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), ">="))
								judge = outcome >= 0 ? true : false;
							else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "<="))
								judge = outcome <= 0 ? true : false;
							else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "="))
								judge = outcome == 0 ? true : false;
							else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "<>"))
								judge = outcome != 0 ? true : false;
							if (!judge)
							{
								break;
							}
						}
						if (!judge)//扫描到当前属性值，已经不符合条件
						{
							break;
						}
					}
				}
				if (judge)
				{
					vector<string> items;
					tempRecord = tempValue.substr(j, tableInfo.totalLength);

					int position = 0;
					for (int m = 0; m < tableInfo.attributes.size(); m++)
					{
						items.push_back(tempRecord.substr(position, tableInfo.attributes[m].length));
						position += tableInfo.attributes[m].length;
					}

					recordOutput(tableInfo, items, selections);

					for (int m = 0; m < tableInfo.attributes.size(); m++)
					{
						items.pop_back();
					}
				}
			}
		}
	}
}

//无条件删除
void RecordManager::deleteTable(Table tableInfo)
{
	string path;
	path = "file/record/" + tableInfo.name + ".txt";

	remove(path.c_str());

	pointer temp = bufferManager.head;
	//从bufferlist中删除该文件所属的块
	while (temp->next)
	{
		if (path == temp->next->fileName)
		{
			pointer blockToDelete = new Block;
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
		if (fileTemp->next->fileName == path)//在文件链表中找到相应文件
		{
			File *temp2 = new File;
			temp2 = fileTemp->next;
			fileTemp->next = fileTemp->next->next;
			delete temp2;
		}
		else fileTemp = fileTemp->next;
	}
	createTable(tableInfo);

	catalogManager->tableBlockNumRefresh(tableInfo);
	//tableInfo.blockNum = 1;
}

//条件删除
void RecordManager::deleteTable(Table &tableInfo, condiPointer conditions)
{
	Offset record;
	for (int i = 0; i < tableInfo.attributes.size(); i++)
	{
		condiPointer temp = conditions;
		while (temp)
		{
			if (temp->attrName == tableInfo.attributes[i].name)
			{
				tableInfo.attributes[i].factor.push_back(*temp);
			}
			temp = temp->next;
		}
	}

	string fileName = "file/record/" + tableInfo.name + ".txt";
	string tempValue;		//存储某一个块中的全部值
	string tempRecord;		//存储某一个记录
	string tempElement;
	int deleteTime = 0;		//标记删除次数

	for (int i = 0; i < tableInfo.blockNum; i++)			//按照文件中块的顺序，将表文件中的块依次读入bufferList
															//record类文件中，保证块是顺序存储的
	{
		if (bufferManager.readBlock(fileName, i))
		{
			int upBoundary = bufferManager.head->next->getRecordNum()*tableInfo.totalLength;
			for (int j = 0; j < upBoundary; j += tableInfo.totalLength)
			{
				tempValue = bufferManager.head->next->CBlock + 4;//将块中记录暂存至tempString（除去前4个字节）
				tempRecord = tempValue.substr(j, tableInfo.totalLength);//暂存一条记录至tempRecord
				int n = 0;
				bool judge = true;//初始化为真。

								  //条件之间关系全部为and
				if (conditions->andor)
				{
					judge = true;//初始化为真。
					for (int m = 0; m < tableInfo.totalLength; m += tableInfo.attributes[n].length, n++)//逐一判断当前记录各属性值
					{
						if (tableInfo.attributes[n].factor.size())//判断当前属性是否有条件值
						{
							tempElement = tempRecord.substr(m, tableInfo.attributes[n].length);//当前属性有条件判断
																							   //将属性值取出，进行判断

							for (int p = 0; p < tableInfo.attributes[n].factor.size(); p++)//逐一访问定义在当前属性下的条件
							{
								int outcome;
								outcome = strcmp(tempElement.c_str(), tableInfo.attributes[n].factor[p].value.c_str());
								//outcome>0：大于；
								//outcome<0：小于；
								//outcome=0：等于；
								if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), ">"))
									judge = outcome > 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "<"))
									judge = outcome < 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), ">="))
									judge = outcome >= 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "<="))
									judge = outcome <= 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "="))
									judge = outcome == 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "<>"))
									judge = outcome != 0 ? true : false;
								if (!judge)
								{
									break;
								}
							}
							if (!judge)//扫描到当前属性值，已经不符合条件
							{
								break;
							}
						}
					}
				}
				//条件之间关系全部为or
				else
				{
					judge = false;//初始化为假。
					for (int m = 0; m < tableInfo.totalLength; m += tableInfo.attributes[n].length, n++)//逐一判断当前记录各属性值
					{
						if (tableInfo.attributes[n].factor.size())//判断当前属性是否有条件值
						{
							tempElement = tempRecord.substr(m, tableInfo.attributes[n].length);//当前属性有条件判断
																							   //将属性值取出，进行判断

							for (int p = 0; p < tableInfo.attributes[n].factor.size(); p++)//逐一访问定义在当前属性下的条件
							{
								int outcome;
								outcome = strcmp(tempElement.c_str(), tableInfo.attributes[n].factor[p].value.c_str());
								//outcome>0：大于；
								//outcome<0：小于；
								//outcome=0：等于；
								if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), ">"))
									judge = outcome > 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "<"))
									judge = outcome < 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), ">="))
									judge = outcome >= 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "<="))
									judge = outcome <= 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "="))
									judge = outcome == 0 ? true : false;
								else if (!strcmp(tableInfo.attributes[n].factor[p].op.c_str(), "<>"))
									judge = outcome != 0 ? true : false;
								if (judge)//扫描到当前属性值，已经符合条件
								{
									break;
								}
							}
							if (judge)//扫描到当前属性值，已经不符合条件
							{
								break;
							}
						}
					}
				}

				if (judge)
				{
					record.blockNum = i;
					record.offset = j + 4;
					string recordToDelete = deleteRecord(tableInfo, record);
					indexUpdateAfterDelete(tableInfo, record, recordToDelete);
					j -= tableInfo.totalLength;
					//upBoundary -= tableInfo.totalLength;
					deleteTime++;
					bufferManager.readBlock(fileName, i);
				}
				if (bufferManager.readBlock(fileName, i))
					upBoundary = bufferManager.head->next->getRecordNum()*tableInfo.totalLength;
				else upBoundary = 0;
			}
		}
	}

	for (int i = 0; i < tableInfo.attributes.size(); i++)
	{
		for (int j = 0; j < tableInfo.attributes[i].factor.size(); j++)
		{
			tableInfo.attributes[i].factor.pop_back();
		}
	}//由于使用了引用，需要将tableInfo里对各属性的条件判断记录信息清空
}

//内部调用：删除某一条记录
string RecordManager::deleteRecord(Table &tableInfo, Offset record)
{
	string fileName = "file/record/" + tableInfo.name + ".txt";
	string tempValue;		//存储某一个块中的全部值
	string tempRecordToDelete;	//存储要删除的记录
	string recordToDelete;
	string tempRecord;

	tempValue = bufferManager.readBlock(fileName, record.blockNum)->CBlock;
	recordToDelete = tempValue.substr(record.offset, tableInfo.totalLength);

	if (bufferManager.readBlock(fileName, record.blockNum))//如果文件中该块不为空
	{
		tempValue = bufferManager.head->next->CBlock + 4;//将块中记录暂存至tempString（除去前4个字节）
		if (bufferManager.head->next->getRecordNum() == 1)//如果该块只有一条记录，直接删除块
		{
			bufferManager.head->next->setBlockValue("");
			bufferManager.head->next->setRecordNum(0);
			if (tableInfo.blockNum > 1)
			{
				bufferManager.deleteBlock(fileName, record.blockNum);
				//tableInfo.blockNum--;
				catalogManager->subTableBlockNum(tableInfo.name);
			}
		}
		else if (!isFull(tableInfo, record.blockNum))//要删除的记录所在的块不是满块
		{
			tempValue = tempValue.erase(record.offset - 4, tableInfo.totalLength);//删除从 offset 开始的一条
			bufferManager.head->next->setRecordNumSubOne();
			bufferManager.head->next->setBlockValue(tempValue.c_str());
		}
		else//当前块为满
		{
			tempValue = tempValue.erase(record.offset - 4, tableInfo.totalLength);//删除从 offset 开始的一条 
			bufferManager.head->next->setRecordNumSubOne();
			bufferManager.head->next->setBlockValue(tempValue.substr(0, (maxRecordNumberPerBlock(tableInfo) - 1)*tableInfo.totalLength).c_str());
			if (bufferManager.readBlock(fileName, record.blockNum + 1))//如果满块的下一个块非空
			{
				insertPointer firstRecord = firstRecordInBlock(tableInfo, record.blockNum + 1);//获取第一条记录

				string first_Record = "";		//存储插入的完整记录

				insertPointer temp = firstRecord;
				while (temp)
				{
					first_Record += temp->value;
					temp = temp->next;
				}
				tempValue.insert((maxRecordNumberPerBlock(tableInfo) - 1)*tableInfo.totalLength, first_Record);//直接插入记录
				bufferManager.readBlock(fileName, record.blockNum);
				bufferManager.head->next->setRecordNumAddOne();

				Offset temp2 = record;
				temp2.blockNum++;
				temp2.offset = 4;
				deleteRecord(tableInfo, temp2);
			}//如果满块的下一个块为空，则无需进一步操作
		}
		return recordToDelete;
	}
	else
	{
		cout << "偏移量信息有误，该块不存在" << endl;
		return NULL;
	}
}

//内部调用：删除后，用于表中记录索引值的更新
void RecordManager::indexUpdateAfterDelete(Table tableInfo, Offset record, string recordToDelete)
{
	Index *index = new Index;
	string indexValue;
	string fileName = "file/record/" + tableInfo.name + ".txt";
	string tempValue;
	string tempRecord;
	string tempIndexedValue;

	int indexPosition = 0;
	for (int i = 0; i < tableInfo.attributes.size(); i++)
	{
		if (tableInfo.attributes[i].isIndexed)
		{
			index = catalogManager->getIndexfromTable(tableInfo.name, tableInfo.attributes[i].name);
			indexValue = recordToDelete.substr(indexPosition, tableInfo.attributes[i].length);
			indexManager.deleteKey(*index, indexValue);
			for (int m = record.blockNum; m < tableInfo.blockNum; m++)
				//从删除记录所在块的所在位置开始遍历之后的所有记录
			{
				if (bufferManager.readBlock(fileName, m))				//如果文件中该块不为空
				{
					tempValue = bufferManager.head->next->CBlock + 4;//将块中记录暂存至tempString（除去前4个字节）
					int lowerBoundary;
					if (m == record.blockNum)
					{
						lowerBoundary = record.offset - 4;//offset中包含了头4字节长度
					}
					else lowerBoundary = 0;
					for (int j = lowerBoundary; j <= (bufferManager.head->next->getRecordNum() - 1)*tableInfo.totalLength; j += tableInfo.totalLength)
					{
						tempRecord = tempValue.substr(j, tableInfo.totalLength);//截取当前一条记录
						tempIndexedValue = tempRecord.substr(indexPosition, tableInfo.attributes[i].length);
						//截取当前索引值

						index = catalogManager->getIndexfromTable(tableInfo.name, tableInfo.attributes[i].name);
						//通过catalogmaanger的getIndexFromTable获取indexInfo
						indexManager.updateKey(*index, tempIndexedValue, m, j + indexPosition + 4);
						bufferManager.readBlock(fileName, m);//重新将表中块推至buffer头节点
					}
				}
			}
		}
		indexPosition += tableInfo.attributes[i].length;
	}
}

//内部调用：插入后，用于表中记录索引值的更新
void RecordManager::indexUpdateAfterInsert(Table tableInfo, Offset position)
{
	string fileName = "file/record/" + tableInfo.name + ".txt";
	string tempValue;
	string tempRecord;
	string tempIndexedValue;

	int length, start;
	int indexPosition = 0;
	Index *index = new Index;
	start = position.offset;
	//valueStart = 0;
	Block *newBlock = bufferManager.readBlock(fileName, position.blockNum);
	string info = newBlock->CBlock;

	//遍历剩余表文件，执行索引更新
	for (int i = 0; i < tableInfo.attrNum; i++)  //循环扫描属性，如果发现有索引，则插入index
	{
		length = tableInfo.attributes[i].length; //属性长度		
		string value = info.substr(start, length);
		if (tableInfo.attributes[i].isIndexed)
		{

			index = catalogManager->getIndexfromTable(tableInfo.name, tableInfo.attributes[i].name);
			//通过catalogmaanger的getIndexFromTable获取indexInfo
			indexManager.insertKey(*index, value, position.blockNum, start);

			for (int m = position.blockNum; m <= tableInfo.blockNum; m++)
				//从插入记录所在块的所在位置开始遍历之后的所有记录
			{
				if (bufferManager.readBlock(fileName, m))				//如果文件中该块不为空
				{
					tempValue = bufferManager.head->next->CBlock + 4;//将块中记录暂存至tempString（除去前4个字节）
					int lowerBoundary;
					if (m == position.blockNum)
					{
						lowerBoundary = position.offset + tableInfo.totalLength - 4;//offset中包含了头4字节长度
					}
					else lowerBoundary = 0;

					for (int j = lowerBoundary; j <= (bufferManager.head->next->getRecordNum() - 1)*tableInfo.totalLength; j += tableInfo.totalLength)
					{
						tempRecord = tempValue.substr(j, tableInfo.totalLength);//截取当前一条记录
						tempIndexedValue = tempRecord.substr(indexPosition, length);//截取当前索引值

						index = catalogManager->getIndexfromTable(tableInfo.name, tableInfo.attributes[i].name);
						//通过CatalogManger的getIndexFromTable获取indexInfo
						indexManager.updateKey(*index, tempIndexedValue, m, j + 4 + indexPosition);

						bufferManager.readBlock(fileName, m);//重新将表中块推至buffer头节点
					}
				}
			}
		}
		start += length;
		indexPosition += length;//下一条属性在记录中的起始位置，如果下一条属性有索引，该信息会被调用
	}
}

//内部调用：全部表头输出
void RecordManager::titleOutput(Table tableInfo)
{
	//输出表头
	for (int i = 0; i < tableInfo.attributes.size(); i++)
	{
		int length;
		if (tableInfo.attributes[i].type == 0)//int型，右对齐
		{
			length = 10;
			if (length > tableInfo.attributes[i].name.size())//int型，属性名短于属性值
			{
				for (int j = 0; j < length - tableInfo.attributes[i].name.size(); j++)
				{
					cout << " ";
				}
			}
			cout << tableInfo.attributes[i].name << " ";
		}
		else//非int型，左对齐
		{
			cout << tableInfo.attributes[i].name;

			length = tableInfo.attributes[i].length;
			if (length >= tableInfo.attributes[i].name.size())//属性名长于属性值
			{
				for (int j = 0; j < length - tableInfo.attributes[i].name.size(); j++)
				{
					cout << " ";
				}
			}
			cout << " ";
		}
	}
	cout << endl;
}

//内部调用：部分表头输出
void RecordManager::titleOutput(Table tableInfo, attrPointer selections)
{
	attrPointer temp = selections;

	while (temp)//输出表头
	{
		for (int i = 0; i < tableInfo.attributes.size(); i++)
		{
			if (temp->attrName == tableInfo.attributes[i].name)
			{
				int length;
				if (tableInfo.attributes[i].type == 0)//int型，右对齐
				{
					length = 10;
					if (length > tableInfo.attributes[i].name.size())//int型，属性名短于属性值
					{
						for (int j = 0; j < length - tableInfo.attributes[i].name.size(); j++)
						{
							cout << " ";
						}
					}
					cout << tableInfo.attributes[i].name << " ";
				}
				else//非int型，左对齐
				{
					cout << tableInfo.attributes[i].name;

					length = tableInfo.attributes[i].length;
					if (length >= tableInfo.attributes[i].name.size())//属性名长于属性值
					{
						for (int j = 0; j < length - tableInfo.attributes[i].name.size(); j++)
						{
							cout << " ";
						}
					}
					cout << " ";
				}
				break;
			}
		}
		temp = temp->next;
	}
	cout << endl;
}

//内部调用：全部输出一条记录
void RecordManager::recordOutput(Table tableInfo, string tempRecord)
{
	string tempElement;
	int n = 0;

	for (int m = 0; m < tableInfo.totalLength; m += tableInfo.attributes[n].length, n++)
	{
		tempElement = tempRecord.substr(m, tableInfo.attributes[n].length);

		if (tableInfo.attributes[n].type == 0)//当前属性为int型
		{
			tempElement = "0x" + tempElement;
			char* endptr;
			int lnumber = strtol(tempElement.c_str(), &endptr, 16);
			stringstream ss;
			tempElement = "";
			ss << lnumber;
			ss >> tempElement;
			for (int i = 0; i < 10 - tempElement.length(); i++)
			{
				cout << " ";
			}
			if (tableInfo.attributes[n].name.length() > 10)
			{
				for (int i = 0; i < 10 - tableInfo.attributes[n].length; i++)
					cout << " ";
			}
			cout << tempElement << " ";
		}
		else
		{
			cout << tempElement << " ";
			if (tableInfo.attributes[n].name.length() > tableInfo.attributes[n].length)
			{
				for (int i = 0; i < tableInfo.attributes[n].name.length() - tableInfo.attributes[n].length; i++)
					cout << " ";
			}
		}

	}
	cout << endl;
}

//内部调用：部分输出一条记录
void RecordManager::recordOutput(Table tableInfo, vector<string> tempRecord, attrPointer selections)
{
	attrPointer temp = new attribute;
	temp = selections;

	while (temp)
	{
		for (int m = 0; m < tempRecord.size(); m++)
		{
			if (temp->attrName == tableInfo.attributes[m].name)
			{
				if (tableInfo.attributes[m].type == 0)//当前属性为int型
				{
					tempRecord[m] = "0x" + tempRecord[m];
					char* endptr;
					int lnumber = strtol(tempRecord[m].c_str(), &endptr, 16);
					stringstream ss;
					tempRecord[m] = "";
					ss << lnumber;
					ss >> tempRecord[m];
					for (int i = 0; i < 10 - tempRecord[m].length(); i++)
					{
						cout << " ";
					}
					if (tableInfo.attributes[m].name.length() > 10)
					{
						for (int i = 0; i < 10 - tableInfo.attributes[m].length; i++)
							cout << " ";
					}
					cout << tempRecord[m] << " ";
				}
				else
				{
					cout << tempRecord[m] << " ";
					if (tableInfo.attributes[m].name.length() > tableInfo.attributes[m].length)
					{
						for (int i = 0; i < tableInfo.attributes[m].name.length() - tableInfo.attributes[m].length; i++)
							cout << " ";
					}
				}
				break;
			}
		}
		temp = temp->next;
	}
	cout << endl;
}
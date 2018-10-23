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

//������
void RecordManager::createTable(Table &tableInfo) //�ձ�Ŀ�������1
{
	string path = "file/record/" + tableInfo.name + ".txt";

	bufferManager.getFile(path);//�ļ����������ؿտ�
	bufferManager.head->next->type = record;
	bufferManager.head->next->setRecordNum(0);//��ʼ�����м�¼��Ϊ0
											  //tableInfo.blockNum = 1;	
}

//ɾ������ɾ�����ļ�
void RecordManager::dropTable(Table &tableInfo)
{
	string path;
	path = "file/record/" + tableInfo.name + ".txt";

	//if (remove(path.c_str()))//ɾ��ʧ��
	//{
	//	//cout << "The table" << tableInfo.name << "does not exist." << endl;
	//	perror("remove");
	//	//system("pause");
	//}//cout << "The table" << tableInfo.name << "has been deleted successfully" << endl;
	//

	pointer temp = bufferManager.head;
	//��bufferlist��ɾ�����ļ������Ŀ�
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
	//���ļ��տ��¼��ɾ���ÿ��ļ�
	while (fileTemp->next)
	{
		if (fileTemp->next->fileName == path)//���ļ��������ҵ���Ӧ�ļ�
		{
			File *temp2 = new File;
			temp2 = fileTemp->next;
			fileTemp->next = fileTemp->next->next;
			delete temp2;
		}
		else fileTemp = fileTemp->next;
	}
}

//�ڲ����ã���������ܴ洢������¼��
int RecordManager::maxRecordNumberPerBlock(Table tableInfo)
{
	return (BLOCK_SIZE - 4) / tableInfo.totalLength;//������¼�ĳ���
}

//�ڲ����ã�������ļ���ĳ�������������ص�ǰ�������һ����¼
insertPointer RecordManager::lastRecordInBlock(Table tableInfo, int blockNum)
{
	string fileName = "file/record/" + tableInfo.name + ".txt";

	if (bufferManager.readBlock(fileName, blockNum))//�����ȡ�ɹ�����Ϊ�տ�
	{
		string temp = bufferManager.head->next->CBlock + 4 + (maxRecordNumberPerBlock(tableInfo) - 1)*tableInfo.totalLength;
		//�ӿ��ж�����¼����Ϣ���һ����¼
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
	else cout << "��ǰ��Ϊ�տ�" << endl;
}

//�ڲ����ã����ص�ǰ���е�һ����¼
insertPointer RecordManager::firstRecordInBlock(Table tableInfo, int blockNum)
{
	string fileName = "file/record/" + tableInfo.name + ".txt";

	if (bufferManager.readBlock(fileName, blockNum))//�����ȡ�ɹ�����Ϊ�տ�
	{
		string temp = bufferManager.head->next->CBlock + 4;
		string str = temp.substr(0, tableInfo.totalLength);//�ӿ��ж�����һ����¼

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
	else cout << "��ǰ��Ϊ�տ�" << endl;
}

//�ڲ����ã��жϿ��м�¼�Ƿ�����
bool RecordManager::isFull(Table tableInfo, int blockNum)
{
	string fileName = "file/record/" + tableInfo.name + ".txt";

	if (bufferManager.readBlock(fileName, blockNum))//����ļ��иÿ鲻Ϊ��
	{
		int temp = bufferManager.head->next->getRecordNum();//�ӿ��ж�����¼����Ϣ
		if (temp == maxRecordNumberPerBlock(tableInfo))//���������Ƚ�
			return true;
		else return false;
	}
}

//�����¼
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

//�ڲ����ã������¼������������ֵ
Offset* RecordManager::insert(Table &tableInfo, insertPointer InfoLine)
{
	string record = "";		//�洢�����������¼
	insertPointer temp = InfoLine;
	while (temp)
	{
		record += temp->value;
		temp = temp->next;
	}

	//����ǰ���ظ��ж�
	//ɨ��һ���������ԣ�����primary��unique��ɨ��������¼�������ֲ����ֵ��������м�¼�ظ�����ܾ��������
	primaryKey inputPrimary(tableInfo, InfoLine);//��primaryKey�๹�캯����ȡ�����¼������ֵ

	vector<unique> uniqueRecord;
	for (int i = 0; i < tableInfo.attributes.size(); i++)
	{
		unique inputUnique(tableInfo, i, record);//��ȡ�����¼��uniqueֵ�������Ϊunique��û����Ϣ�ᱻѹ��
		uniqueRecord.push_back(inputUnique);
	}

	Offset position;
	string fileName = "file/record/" + tableInfo.name + ".txt";
	string tempValue;		//�洢ĳһ�����е�ȫ��ֵ
	string tempRecord;		//�洢ĳһ����¼
	string tempElement;
	string tempPrimaryKey;
	string tempUnique;
	string tempIndexedValue; //�洢ĳһ�����еĴ�����������ֵ

							 //������unique�ظ��ж�
	bool ifRepeated = false;//����Ƿ��ظ�����ʼ��Ϊ���ظ�
	for (int i = 0; i < tableInfo.blockNum; i++)
	{
		if (bufferManager.readBlock(fileName, i))
		{
			tempValue = bufferManager.head->next->CBlock + 4;//�����м�¼�ݴ���tempString����ȥǰ4���ֽڣ�

			for (int j = 0; j < bufferManager.head->next->getRecordNum()*tableInfo.totalLength; j += tableInfo.totalLength)
			{
				tempRecord = tempValue.substr(j, tableInfo.totalLength);//�ݴ�һ����¼��tempRecord
																		//int currentPosition = 0;
				for (int m = 0; m < tableInfo.attributes.size(); m++)
				{
					tempElement = tempRecord.substr(uniqueRecord[m].uniquePosition, tableInfo.attributes[m].length);//��ǰ�����������ж�
					if (tableInfo.attributes[m].isUnique)
					{
						ifRepeated = !strcmp(tempElement.c_str(), uniqueRecord[m].uniqueValue.c_str());
						if (ifRepeated)//���ظ�
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
	//ִ�в���
	for (int i = 0; i <= tableInfo.blockNum; i++)			//�����ļ��п��˳�򣬽����ļ��еĿ����ζ���bufferList
															//record���ļ��У���֤����˳��洢��
	{
		if (bufferManager.readBlock(fileName, i))				//����ļ��иÿ鲻Ϊ��
		{
			tempValue = bufferManager.head->next->CBlock + 4;//�����м�¼�ݴ���tempString����ȥǰ4���ֽڣ�
			int upBoundary;
			if (!isFull(tableInfo, i))
			{
				//if (bufferManager.head->next->getRecordNum() > 0)
				//	{
				upBoundary = bufferManager.head->next->getRecordNum()*tableInfo.totalLength;
				//	}
				//else//����ڵ���¼Ϊ��
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
				tempPrimaryKey = tempRecord.substr(inputPrimary.primaryPosition, inputPrimary.primaryLength);//������¼��������Ϣ

				if (inputPrimary.PrimaryValue.compare(tempPrimaryKey) < 0)//Ҫ���������ֵ���ֵ���������λ�ڵ�ǰ��¼����ֵǰ
				{
					if (!isFull(tableInfo, i))//���м�¼δ��
					{
						tempValue.insert(j, record);//ֱ�Ӳ����¼
						position.blockNum = i;
						position.offset = j + 4;//record����Ϣ����
						bufferManager.head->next->setRecordNumAddOne();//��¼����һ
						bufferManager.head->next->setBlockValue(tempValue.substr(0, BLOCK_SIZE).c_str());
						goto Inserted;
					}
					else//�������������Ҫ����һ���鴫�����һ����¼
					{
						insertPointer lastRecord = lastRecordInBlock(tableInfo, i);//��ȡ���һ����¼
						tempValue.insert(j, record);//ֱ�Ӳ����¼
						position.blockNum = i;
						position.offset = j + 4;//record����Ϣ����
						bufferManager.head->next->setBlockValue(tempValue.substr(0, BLOCK_SIZE).c_str());

						insert(tableInfo, lastRecord);//�ݹ���ò��뺯����������ĩβ���ļ�¼���²���
						goto Inserted;
					}
				}
			}
		}
		else//����տ飬ֱ��д�룬��¼����1
		{
			bufferManager.getBlankBlock(fileName);//��ȡ�տ�

			catalogManager->addTableBlockNum(tableInfo.name); //���п���+1

			bufferManager.head->next->setRecordNum(1);//��¼����һ
			bufferManager.head->next->setBlockValue(record.c_str());//��������¼д��տ�
			position.blockNum = i;
			position.offset = 4;//record����Ϣ����
			goto Inserted;
		}
	}

Inserted://�������
	Offset* returnValue = new Offset;
	returnValue->blockNum = position.blockNum;
	returnValue->offset = position.offset;
	return returnValue;
}

//����������ȫ���������
void RecordManager::select(Table tableInfo)
{
	string fileName = "file/record/" + tableInfo.name + ".txt";
	string tempValue;		//�洢ĳһ�����е�ȫ��ֵ
	string tempRecord;		//�洢ĳһ����¼
	string tempElement;

	titleOutput(tableInfo);

	for (int i = 0; i < tableInfo.blockNum; i++)			//�����ļ��п��˳�򣬽����ļ��еĿ����ζ���bufferList
															//record���ļ��У���֤����˳��洢��
	{
		if (bufferManager.readBlock(fileName, i))
		{
			tempValue = bufferManager.head->next->CBlock + 4;//�����м�¼�ݴ���tempString����ȥǰ4���ֽڣ�
			for (int j = 0; j < bufferManager.head->next->getRecordNum()*tableInfo.totalLength; j += tableInfo.totalLength)
			{
				tempRecord = tempValue.substr(j, tableInfo.totalLength);
				recordOutput(tableInfo, tempRecord);
			}
		}
	}
}

//���������Ҳ����������
void RecordManager::select(Table tableInfo, attrPointer selections)
{
	string fileName = "file/record/" + tableInfo.name + ".txt";
	string tempValue;		//�洢ĳһ�����е�ȫ��ֵ
	string tempRecord;		//�洢ĳһ����¼
	string tempElement;

	titleOutput(tableInfo, selections);

	for (int i = 0; i < tableInfo.blockNum; i++)			//�����ļ��п��˳�򣬽����ļ��еĿ����ζ���bufferList
															//record���ļ��У���֤����˳��洢��
	{
		if (bufferManager.readBlock(fileName, i))
		{
			tempValue = bufferManager.head->next->CBlock + 4;//�����м�¼�ݴ���tempString����ȥǰ4���ֽڣ�
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

//��������ȫ���������
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
	string tempValue;		//�洢ĳһ�����е�ȫ��ֵ
	string tempRecord;		//�洢ĳһ����¼
	string tempElement;


	titleOutput(tableInfo);

	for (int i = 0; i < tableInfo.blockNum; i++)			//�����ļ��п��˳�򣬽����ļ��еĿ����ζ���bufferList
															//record���ļ��У���֤����˳��洢��
	{
		if (bufferManager.readBlock(fileName, i))
		{
			tempValue = bufferManager.head->next->CBlock + 4;//�����м�¼�ݴ���tempString����ȥǰ4���ֽڣ�

			for (int j = 0; j < bufferManager.head->next->getRecordNum()*tableInfo.totalLength; j += tableInfo.totalLength)
			{
				tempRecord = tempValue.substr(j, tableInfo.totalLength);//�ݴ�һ����¼��tempRecord
				int n = 0;
				bool judge;

				//����֮���ϵȫ��Ϊand
				if (conditions->andor)
				{
					judge = true;//��ʼ��Ϊ�档
					for (int m = 0; m < tableInfo.totalLength; m += tableInfo.attributes[n].length, n++)//��һ�жϵ�ǰ��¼������ֵ
					{
						if (tableInfo.attributes[n].factor.size())//�жϵ�ǰ�����Ƿ�������ֵ
						{
							tempElement = tempRecord.substr(m, tableInfo.attributes[n].length);//��ǰ�����������ж�
																							   //������ֵȡ���������ж�

							for (int p = 0; p < tableInfo.attributes[n].factor.size(); p++)//��һ���ʶ����ڵ�ǰ�����µ�����
							{
								int outcome;
								outcome = strcmp(tempElement.c_str(), tableInfo.attributes[n].factor[p].value.c_str());
								//outcome>0�����ڣ�
								//outcome<0��С�ڣ�
								//outcome=0�����ڣ�
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
							if (!judge)//ɨ�赽��ǰ����ֵ���Ѿ�����������
							{
								break;
							}
						}
					}
				}
				//����֮���ϵȫ��Ϊor
				else
				{
					judge = false;//��ʼ��Ϊ�١�
					for (int m = 0; m < tableInfo.totalLength; m += tableInfo.attributes[n].length, n++)//��һ�жϵ�ǰ��¼������ֵ
					{
						if (tableInfo.attributes[n].factor.size())//�жϵ�ǰ�����Ƿ�������ֵ
						{
							tempElement = tempRecord.substr(m, tableInfo.attributes[n].length);//��ǰ�����������ж�
																							   //������ֵȡ���������ж�

							for (int p = 0; p < tableInfo.attributes[n].factor.size(); p++)//��һ���ʶ����ڵ�ǰ�����µ�����
							{
								int outcome;
								outcome = strcmp(tempElement.c_str(), tableInfo.attributes[n].factor[p].value.c_str());
								//outcome>0�����ڣ�
								//outcome<0��С�ڣ�
								//outcome=0�����ڣ�
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
								if (judge)//ɨ�赽��ǰ����ֵ���Ѿ���������
								{
									break;
								}
							}
							if (judge)//ɨ�赽��ǰ����ֵ���Ѿ�����������
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

// �������Ҳ����������
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
	string tempValue;		//�洢ĳһ�����е�ȫ��ֵ
	string tempRecord;		//�洢ĳһ����¼
	string tempElement;
	attrPointer temp = selections;

	titleOutput(tableInfo, selections);

	for (int i = 0; i < tableInfo.blockNum; i++)			//�����ļ��п��˳�򣬽����ļ��еĿ����ζ���bufferList
															//record���ļ��У���֤����˳��洢��
	{
		if (bufferManager.readBlock(fileName, i))
		{
			tempValue = bufferManager.head->next->CBlock + 4;//�����м�¼�ݴ���tempString����ȥǰ4���ֽڣ�

			for (int j = 0; j < bufferManager.head->next->getRecordNum()*tableInfo.totalLength; j += tableInfo.totalLength)
			{
				tempRecord = tempValue.substr(j, tableInfo.totalLength);//�ݴ�һ����¼��tempRecord
				int n = 0;
				bool judge = true;//��ʼ��Ϊ�档
				for (int m = 0; m < tableInfo.totalLength; m += tableInfo.attributes[n].length, n++)//��һ�жϵ�ǰ��¼������ֵ
				{
					if (tableInfo.attributes[n].factor.size())//�жϵ�ǰ�����Ƿ�������ֵ
					{
						tempElement = tempRecord.substr(m, tableInfo.attributes[n].length);//��ǰ�����������жϣ�������ֵȡ���������ж�

						for (int p = 0; p < tableInfo.attributes[n].factor.size(); p++)//��һ���ʶ����ڵ�ǰ�����µ�����
						{
							int outcome;
							outcome = strcmp(tempElement.c_str(), tableInfo.attributes[n].factor[p].value.c_str());
							//outcome>0�����ڣ�
							//outcome<0��С�ڣ�
							//outcome=0�����ڣ�
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
						if (!judge)//ɨ�赽��ǰ����ֵ���Ѿ�����������
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

//������ɾ��
void RecordManager::deleteTable(Table tableInfo)
{
	string path;
	path = "file/record/" + tableInfo.name + ".txt";

	remove(path.c_str());

	pointer temp = bufferManager.head;
	//��bufferlist��ɾ�����ļ������Ŀ�
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
	//���ļ��տ��¼��ɾ���ÿ��ļ�
	while (fileTemp->next)
	{
		if (fileTemp->next->fileName == path)//���ļ��������ҵ���Ӧ�ļ�
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

//����ɾ��
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
	string tempValue;		//�洢ĳһ�����е�ȫ��ֵ
	string tempRecord;		//�洢ĳһ����¼
	string tempElement;
	int deleteTime = 0;		//���ɾ������

	for (int i = 0; i < tableInfo.blockNum; i++)			//�����ļ��п��˳�򣬽����ļ��еĿ����ζ���bufferList
															//record���ļ��У���֤����˳��洢��
	{
		if (bufferManager.readBlock(fileName, i))
		{
			int upBoundary = bufferManager.head->next->getRecordNum()*tableInfo.totalLength;
			for (int j = 0; j < upBoundary; j += tableInfo.totalLength)
			{
				tempValue = bufferManager.head->next->CBlock + 4;//�����м�¼�ݴ���tempString����ȥǰ4���ֽڣ�
				tempRecord = tempValue.substr(j, tableInfo.totalLength);//�ݴ�һ����¼��tempRecord
				int n = 0;
				bool judge = true;//��ʼ��Ϊ�档

								  //����֮���ϵȫ��Ϊand
				if (conditions->andor)
				{
					judge = true;//��ʼ��Ϊ�档
					for (int m = 0; m < tableInfo.totalLength; m += tableInfo.attributes[n].length, n++)//��һ�жϵ�ǰ��¼������ֵ
					{
						if (tableInfo.attributes[n].factor.size())//�жϵ�ǰ�����Ƿ�������ֵ
						{
							tempElement = tempRecord.substr(m, tableInfo.attributes[n].length);//��ǰ�����������ж�
																							   //������ֵȡ���������ж�

							for (int p = 0; p < tableInfo.attributes[n].factor.size(); p++)//��һ���ʶ����ڵ�ǰ�����µ�����
							{
								int outcome;
								outcome = strcmp(tempElement.c_str(), tableInfo.attributes[n].factor[p].value.c_str());
								//outcome>0�����ڣ�
								//outcome<0��С�ڣ�
								//outcome=0�����ڣ�
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
							if (!judge)//ɨ�赽��ǰ����ֵ���Ѿ�����������
							{
								break;
							}
						}
					}
				}
				//����֮���ϵȫ��Ϊor
				else
				{
					judge = false;//��ʼ��Ϊ�١�
					for (int m = 0; m < tableInfo.totalLength; m += tableInfo.attributes[n].length, n++)//��һ�жϵ�ǰ��¼������ֵ
					{
						if (tableInfo.attributes[n].factor.size())//�жϵ�ǰ�����Ƿ�������ֵ
						{
							tempElement = tempRecord.substr(m, tableInfo.attributes[n].length);//��ǰ�����������ж�
																							   //������ֵȡ���������ж�

							for (int p = 0; p < tableInfo.attributes[n].factor.size(); p++)//��һ���ʶ����ڵ�ǰ�����µ�����
							{
								int outcome;
								outcome = strcmp(tempElement.c_str(), tableInfo.attributes[n].factor[p].value.c_str());
								//outcome>0�����ڣ�
								//outcome<0��С�ڣ�
								//outcome=0�����ڣ�
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
								if (judge)//ɨ�赽��ǰ����ֵ���Ѿ���������
								{
									break;
								}
							}
							if (judge)//ɨ�赽��ǰ����ֵ���Ѿ�����������
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
	}//����ʹ�������ã���Ҫ��tableInfo��Ը����Ե������жϼ�¼��Ϣ���
}

//�ڲ����ã�ɾ��ĳһ����¼
string RecordManager::deleteRecord(Table &tableInfo, Offset record)
{
	string fileName = "file/record/" + tableInfo.name + ".txt";
	string tempValue;		//�洢ĳһ�����е�ȫ��ֵ
	string tempRecordToDelete;	//�洢Ҫɾ���ļ�¼
	string recordToDelete;
	string tempRecord;

	tempValue = bufferManager.readBlock(fileName, record.blockNum)->CBlock;
	recordToDelete = tempValue.substr(record.offset, tableInfo.totalLength);

	if (bufferManager.readBlock(fileName, record.blockNum))//����ļ��иÿ鲻Ϊ��
	{
		tempValue = bufferManager.head->next->CBlock + 4;//�����м�¼�ݴ���tempString����ȥǰ4���ֽڣ�
		if (bufferManager.head->next->getRecordNum() == 1)//����ÿ�ֻ��һ����¼��ֱ��ɾ����
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
		else if (!isFull(tableInfo, record.blockNum))//Ҫɾ���ļ�¼���ڵĿ鲻������
		{
			tempValue = tempValue.erase(record.offset - 4, tableInfo.totalLength);//ɾ���� offset ��ʼ��һ��
			bufferManager.head->next->setRecordNumSubOne();
			bufferManager.head->next->setBlockValue(tempValue.c_str());
		}
		else//��ǰ��Ϊ��
		{
			tempValue = tempValue.erase(record.offset - 4, tableInfo.totalLength);//ɾ���� offset ��ʼ��һ�� 
			bufferManager.head->next->setRecordNumSubOne();
			bufferManager.head->next->setBlockValue(tempValue.substr(0, (maxRecordNumberPerBlock(tableInfo) - 1)*tableInfo.totalLength).c_str());
			if (bufferManager.readBlock(fileName, record.blockNum + 1))//����������һ����ǿ�
			{
				insertPointer firstRecord = firstRecordInBlock(tableInfo, record.blockNum + 1);//��ȡ��һ����¼

				string first_Record = "";		//�洢�����������¼

				insertPointer temp = firstRecord;
				while (temp)
				{
					first_Record += temp->value;
					temp = temp->next;
				}
				tempValue.insert((maxRecordNumberPerBlock(tableInfo) - 1)*tableInfo.totalLength, first_Record);//ֱ�Ӳ����¼
				bufferManager.readBlock(fileName, record.blockNum);
				bufferManager.head->next->setRecordNumAddOne();

				Offset temp2 = record;
				temp2.blockNum++;
				temp2.offset = 4;
				deleteRecord(tableInfo, temp2);
			}//����������һ����Ϊ�գ��������һ������
		}
		return recordToDelete;
	}
	else
	{
		cout << "ƫ������Ϣ���󣬸ÿ鲻����" << endl;
		return NULL;
	}
}

//�ڲ����ã�ɾ�������ڱ��м�¼����ֵ�ĸ���
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
				//��ɾ����¼���ڿ������λ�ÿ�ʼ����֮������м�¼
			{
				if (bufferManager.readBlock(fileName, m))				//����ļ��иÿ鲻Ϊ��
				{
					tempValue = bufferManager.head->next->CBlock + 4;//�����м�¼�ݴ���tempString����ȥǰ4���ֽڣ�
					int lowerBoundary;
					if (m == record.blockNum)
					{
						lowerBoundary = record.offset - 4;//offset�а�����ͷ4�ֽڳ���
					}
					else lowerBoundary = 0;
					for (int j = lowerBoundary; j <= (bufferManager.head->next->getRecordNum() - 1)*tableInfo.totalLength; j += tableInfo.totalLength)
					{
						tempRecord = tempValue.substr(j, tableInfo.totalLength);//��ȡ��ǰһ����¼
						tempIndexedValue = tempRecord.substr(indexPosition, tableInfo.attributes[i].length);
						//��ȡ��ǰ����ֵ

						index = catalogManager->getIndexfromTable(tableInfo.name, tableInfo.attributes[i].name);
						//ͨ��catalogmaanger��getIndexFromTable��ȡindexInfo
						indexManager.updateKey(*index, tempIndexedValue, m, j + indexPosition + 4);
						bufferManager.readBlock(fileName, m);//���½����п�����bufferͷ�ڵ�
					}
				}
			}
		}
		indexPosition += tableInfo.attributes[i].length;
	}
}

//�ڲ����ã���������ڱ��м�¼����ֵ�ĸ���
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

	//����ʣ����ļ���ִ����������
	for (int i = 0; i < tableInfo.attrNum; i++)  //ѭ��ɨ�����ԣ���������������������index
	{
		length = tableInfo.attributes[i].length; //���Գ���		
		string value = info.substr(start, length);
		if (tableInfo.attributes[i].isIndexed)
		{

			index = catalogManager->getIndexfromTable(tableInfo.name, tableInfo.attributes[i].name);
			//ͨ��catalogmaanger��getIndexFromTable��ȡindexInfo
			indexManager.insertKey(*index, value, position.blockNum, start);

			for (int m = position.blockNum; m <= tableInfo.blockNum; m++)
				//�Ӳ����¼���ڿ������λ�ÿ�ʼ����֮������м�¼
			{
				if (bufferManager.readBlock(fileName, m))				//����ļ��иÿ鲻Ϊ��
				{
					tempValue = bufferManager.head->next->CBlock + 4;//�����м�¼�ݴ���tempString����ȥǰ4���ֽڣ�
					int lowerBoundary;
					if (m == position.blockNum)
					{
						lowerBoundary = position.offset + tableInfo.totalLength - 4;//offset�а�����ͷ4�ֽڳ���
					}
					else lowerBoundary = 0;

					for (int j = lowerBoundary; j <= (bufferManager.head->next->getRecordNum() - 1)*tableInfo.totalLength; j += tableInfo.totalLength)
					{
						tempRecord = tempValue.substr(j, tableInfo.totalLength);//��ȡ��ǰһ����¼
						tempIndexedValue = tempRecord.substr(indexPosition, length);//��ȡ��ǰ����ֵ

						index = catalogManager->getIndexfromTable(tableInfo.name, tableInfo.attributes[i].name);
						//ͨ��CatalogManger��getIndexFromTable��ȡindexInfo
						indexManager.updateKey(*index, tempIndexedValue, m, j + 4 + indexPosition);

						bufferManager.readBlock(fileName, m);//���½����п�����bufferͷ�ڵ�
					}
				}
			}
		}
		start += length;
		indexPosition += length;//��һ�������ڼ�¼�е���ʼλ�ã������һ������������������Ϣ�ᱻ����
	}
}

//�ڲ����ã�ȫ����ͷ���
void RecordManager::titleOutput(Table tableInfo)
{
	//�����ͷ
	for (int i = 0; i < tableInfo.attributes.size(); i++)
	{
		int length;
		if (tableInfo.attributes[i].type == 0)//int�ͣ��Ҷ���
		{
			length = 10;
			if (length > tableInfo.attributes[i].name.size())//int�ͣ���������������ֵ
			{
				for (int j = 0; j < length - tableInfo.attributes[i].name.size(); j++)
				{
					cout << " ";
				}
			}
			cout << tableInfo.attributes[i].name << " ";
		}
		else//��int�ͣ������
		{
			cout << tableInfo.attributes[i].name;

			length = tableInfo.attributes[i].length;
			if (length >= tableInfo.attributes[i].name.size())//��������������ֵ
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

//�ڲ����ã����ֱ�ͷ���
void RecordManager::titleOutput(Table tableInfo, attrPointer selections)
{
	attrPointer temp = selections;

	while (temp)//�����ͷ
	{
		for (int i = 0; i < tableInfo.attributes.size(); i++)
		{
			if (temp->attrName == tableInfo.attributes[i].name)
			{
				int length;
				if (tableInfo.attributes[i].type == 0)//int�ͣ��Ҷ���
				{
					length = 10;
					if (length > tableInfo.attributes[i].name.size())//int�ͣ���������������ֵ
					{
						for (int j = 0; j < length - tableInfo.attributes[i].name.size(); j++)
						{
							cout << " ";
						}
					}
					cout << tableInfo.attributes[i].name << " ";
				}
				else//��int�ͣ������
				{
					cout << tableInfo.attributes[i].name;

					length = tableInfo.attributes[i].length;
					if (length >= tableInfo.attributes[i].name.size())//��������������ֵ
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

//�ڲ����ã�ȫ�����һ����¼
void RecordManager::recordOutput(Table tableInfo, string tempRecord)
{
	string tempElement;
	int n = 0;

	for (int m = 0; m < tableInfo.totalLength; m += tableInfo.attributes[n].length, n++)
	{
		tempElement = tempRecord.substr(m, tableInfo.attributes[n].length);

		if (tableInfo.attributes[n].type == 0)//��ǰ����Ϊint��
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

//�ڲ����ã��������һ����¼
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
				if (tableInfo.attributes[m].type == 0)//��ǰ����Ϊint��
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
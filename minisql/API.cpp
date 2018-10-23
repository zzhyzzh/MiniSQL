#include"API.h"
#include<iostream>
using namespace std;
#include<string>
#include<fstream>
#include"interpreter.h"
#include"indexManager.h"
#include"stdafx.h"
#include<time.h>

//create table 1 index 2
//drop table 3,drop index 4 
//select * 5 select ���ԣ���Ϊ������ԣ� 6
//insert 7
//delete 8  ��where���� 9
//execfile 10
//quit 0
//error 99

extern CatalogManager *catalogManager;
extern IndexManager indexManager;
extern RecordManager recordManager;

void API::execCreateTable(Interpret SQL)
{
	if (catalogManager->isTable(SQL.m_strTablename))//������Ѿ�����
		cout << "The table " << SQL.m_strTablename << " already exists! Please try another table name." << endl;
	//�½������
	else
	{
		Table newT;
		Index newI;//�������ϴ�����
		newT.name = SQL.m_strTablename;
		newT.blockNum = 1;
		newT.attrNum = 0;//���Ը���
		newT.totalLength = 0;//�ܳ���

		newI.blockNum = 1;
		newI.tableName = SQL.m_strTablename;

		//����������Ϣ
		while (SQL.m_attr != NULL)
		{
			newT.attrNum++;
			Attr attr;
			attr.name = SQL.m_attr->attrName;
			attr.isPrimeryKey = SQL.m_attr->isPrimary;
			attr.isUnique = SQL.m_attr->isUnique;
			//���������
			if (attr.isPrimeryKey)
			{
				attr.isIndexed = true;
				attr.indexName = SQL.m_attr->attrName;
				newI.name = SQL.m_attr->attrName;//������������
				newI.value = SQL.m_attr->attrName;//������������������ͬ
				if (SQL.m_attr->attrType == "int")
				{
					newI.type = 0;
					newI.length = 8;
				}
				else if (SQL.m_attr->attrType == "float")
				{
					newI.type = 1;
					newI.length = 8;
				}
				else if (SQL.m_attr->attrType == "char")
				{
					newI.type = 2;
					newI.length = SQL.m_attr->charLen;
				}
			}
			else
			{
				attr.isIndexed = false;
				attr.indexName = "0";
			}

			if (SQL.m_attr->attrType == "int")
			{
				attr.type = 0;
				attr.length = 8;
				newT.totalLength += 8;
			}
			else if (SQL.m_attr->attrType == "float")
			{
				attr.type = 1;
				attr.length = 8;
				newT.totalLength += 8;
			}
			else if (SQL.m_attr->attrType == "char")
			{
				attr.type = 2;
				attr.length = SQL.m_attr->charLen;
				newT.totalLength += SQL.m_attr->charLen;
			}

			newT.attributes.push_back(attr);
			//cout <<"attr.type"<< attr.type << endl;
			SQL.m_attr = SQL.m_attr->next;
		}
		catalogManager->Create_Table(newT);
		recordManager.createTable(newT);//��record manage����
		cout << "Create table successfully." << endl;

		catalogManager->Create_Index(newI);
		indexManager.createIndex(newT, newI);
	}
}

void API::execCreateIndex(Interpret SQL)
{
	if (catalogManager->isIndex(SQL.m_strIndexname))
		cout << "The index " << SQL.m_strIndexname << " already exists!" << endl;
	else
	{
		//�������Ե�isIndex��Ϊ1
		Table *T = catalogManager->getTable(SQL.m_strTablename);
		if (T == NULL)
		{
			cout << "error!The table doesn't exist." << endl;
			return;
		}
		int i;
		for (i = 0; i < T->attrNum; i++)
		{
			if (T->attributes[i].name == SQL.m_attr->attrName) //�õ�������
			{
				T->attributes[i].isIndexed = 1;
				T->attributes[i].indexName = SQL.m_strIndexname;
				break;
			}
		}
		if (i >= T->attrNum)//���û�и����������򱨴�
		{
			cout << "error! Can't find the attribute to create index." << endl;
		}
		else //�õ�������
		{
			Index newI;
			newI.blockNum = 0; //��ȷ����������
			newI.name = SQL.m_attr->attrName;
			newI.tableName = SQL.m_strTablename;
			newI.value = SQL.m_strIndexname;
			int type = catalogManager->getAttrType(SQL.m_strTablename, SQL.m_attr->attrName);
			if (type == 0)//int
			{
				newI.type = 0;
				newI.length = 4;
			}
			else if (type == 1)//float
			{
				newI.type = 1;
				newI.length = 8;
			}
			else if (type == 2) //char
			{
				newI.type = 2;
				newI.length = T->attributes[i].length;
				//cout << "charlen:" << newI.length << endl;
			}
			catalogManager->Create_Index(newI);
			cout << "Create index successfully." << endl;
			indexManager.createIndex(*(catalogManager->getTable(newI.tableName)), newI);
		}

	}
}

void API::execDropTable(Interpret SQL)
{
	if (!catalogManager->isTable(SQL.m_strTablename))//���������
		cout << "The table " << SQL.m_strTablename << " doesn't exist!" << endl;
	else
	{

		Table t = *(catalogManager->getTable(SQL.m_strTablename));
		Index* indexpointer = new Index;
		//��ȡ��ĸ������������Ӷ��ж������Ƿ�������
		for (int i = 0; i < t.attrNum; i++)
		{
			indexpointer = catalogManager->getIndexfromTable(SQL.m_strTablename, t.attributes[i].name);

			if (indexpointer)//�������������ɾ��s
			{
				indexManager.dropIndex(*indexpointer);
				catalogManager->Drop_Index(indexpointer->value);
			}
		}
		catalogManager->Drop_Table(SQL.m_strTablename);
		recordManager.dropTable(t);
		cout << "Drop table successfully." << endl;
	}
}

void API::execDropIndex(Interpret SQL)
{
	if (!catalogManager->isIndex(SQL.m_strIndexname))//�������������
		cout << "The index " << SQL.m_strIndexname << " doesn't exist" << endl;
	else
	{
		cout << "Drop index successfully." << endl;
		indexManager.dropIndex(catalogManager->getIndex(SQL.m_strIndexname));
		catalogManager->Drop_Index(SQL.m_strIndexname);
	}
}

void API::execSelectALL(Interpret SQL)
{
	clock_t start, end;
	if (!catalogManager->isTable(SQL.m_strTablename))//���������,����
		cout << "The table " << SQL.m_strTablename << " doesn't exist!" << endl;
	else
	{
		Table* T = catalogManager->getTable(SQL.m_strTablename);
		if (SQL.m_condi == NULL)//���������������
		{
			recordManager.select(*(catalogManager->getTable(SQL.m_strTablename)));//record manager����
		}
		else//����������
		{
			//�����������������indexmanager�ĺ���
			if (SQL.m_condi->next == NULL && SQL.m_condi->op == "=")
			{
				Index* indexpointer = new Index;
				//indexpointer = catalogManager->getIndexfromTable(SQL.m_strTablename, T->attributes[0].name);
				indexpointer = catalogManager->getIndexfromTable(SQL.m_strTablename, SQL.m_condi->attrName);
				if (indexpointer)//���������
				{
					//start = clock();
					indexManager.searchEqual(*T, *indexpointer, SQL.m_condi->value);
					//end = clock();
					//printf("Search time with index: %e\n", (double)(end - start));
				}
				else
				{
					//start = clock();
					recordManager.select(*T, SQL.m_condi);
					//end = clock();
					//printf("Search time without index: %e\n", (double)(end - start));
				}
			}
			else
			{
				recordManager.select(*T, SQL.m_condi);
			}

		}
	}
}

void API::execSelectAttr(Interpret SQL)
{
	if (!catalogManager->isTable(SQL.m_strTablename))//���������,����
		cout << "The table " << SQL.m_strTablename << " doesn't exist!" << endl;
	else
	{
		Table* T = catalogManager->getTable(SQL.m_strTablename);
		if (SQL.m_condi == NULL)//��������������Ҳ�������
		{
			attrPointer temp = new attribute;
			temp = SQL.m_attr;

			//cout << "���������Ҳ�������" << endl;
			recordManager.select(*T, SQL.m_attr);
		}
		else
		{
			attrPointer temp = new attribute;
			temp = SQL.m_attr;

			//�����������������indexmanager�ĺ���
			if (SQL.m_condi->next == NULL&&SQL.m_condi->op == "=")
			{
				Index* indexpointer = new Index;
				indexpointer = catalogManager->getIndexfromTable(SQL.m_strTablename, SQL.m_condi->attrName);
				if (indexpointer)//���������
				{
					indexManager.searchEqual(*T, *indexpointer, SQL.m_condi->value);
				}
				else
				{
					recordManager.select(*T, SQL.m_condi, SQL.m_attr);
				}
			}
			else
			{
				recordManager.select(*T, SQL.m_condi, SQL.m_attr);
			}

		}
	}
}

void API::execInsert(Interpret SQL)
{

	if (!catalogManager->isTable(SQL.m_strTablename))//���������
		cout << "The table " << SQL.m_strTablename << " doesn't exist!" << endl;
	else
	{
		Table T = *(catalogManager->getTable(SQL.m_strTablename));
		int i = 0;
		insertVal *tempval = NULL;
		tempval = SQL.m_values;
		//Ҫ�жϸ��������Ƿ���ͬ��������Բ�ƥ�䣬�򱨴�
		while (tempval != NULL&&i < T.attrNum)
		{
			//�����ֵ�����char��
			if (tempval->type == 1 && catalogManager->getAttrType(SQL.m_strTablename, i) != 2)
			{
				cout << "error!Insert type char doesn't match." << endl;
				return;
			}
			//float,int
			else if (!catalogManager->matchType(tempval->value, SQL.m_strTablename, i))
			{
				//cout << "tempval->value" << tempval->value << endl;

				cout << "error!Insert type int or float doesn't match." << endl;
				return;
			}
			i++;
			tempval = tempval->next;
		}
		//cout << "Insert successfully." << endl;
		recordManager.insertValue(T, SQL.m_values);

	}
}

void API::execDeleteAll(Interpret SQL)
{
	if (!catalogManager->isTable(SQL.m_strTablename))//���������,����
		cout << "The table " << SQL.m_strTablename << " doesn't exist!" << endl;
	else
	{
		Table* t = catalogManager->getTable(SQL.m_strTablename);
		Index* indexpointer;
		//��ȡ��ĸ������������Ӷ��ж������Ƿ�������
		cout << "Delete successfully." << endl;
		recordManager.deleteTable(*t);
		t = catalogManager->getTable(SQL.m_strTablename);
		for (int i = 0; i < t->attrNum; i++)
		{
			indexpointer = catalogManager->getIndexfromTable(SQL.m_strTablename, t->attributes[i].name);
			if (indexpointer)//�������������ɾ��s
			{
				indexManager.deleteAllIndex(*t, *indexpointer);
			}
		}

	}
}

void API::execDeletePart(Interpret SQL)
{
	if (!catalogManager->isTable(SQL.m_strTablename))//���������,����
		cout << "The table " << SQL.m_strTablename << " doesn't exist!" << endl;
	else
	{
		cout << "Delete successfully." << endl;
		recordManager.deleteTable(*(catalogManager->getTable(SQL.m_strTablename)), SQL.m_condi);
	}
}

void API::execFile(Interpret SQL)
{
	string fName = SQL.m_strFilename;
	string sql = "", temps = "";
	string s;
	ifstream file(fName);
	Interpret newI;
	API api;
	if (!file)
	{
		cout << "File open error!" << endl;
		return;
	}
	while (file)
	{
		if (file >> temps)
		{
			sql += temps + " ";
			for (int i = 0; i < sql.length(); i++)
			{
				if (sql.at(i) == ';')
				{
					s = sql.substr(0, i + 1);
					cout << sql << endl;
					newI.handle(s);
					api.execSQL(newI);
					if (newI.m_iOp == 0)
					{
						return;
					}
					sql.replace(0, i + 1, "");
					//cout << sql << endl;
				}
			}
		}
	}
	if (sql == " ")
		return;
	while (sql.length() != 0)
	{
		//cout << "aaa" << endl;
		for (int i = 0; i < sql.length(); i++)
		{
			if (sql.at(i) == ';')
			{
				s = sql.substr(0, i + 1);
				cout << sql << endl;
				newI.handle(s);
				api.execSQL(newI);
				if (newI.m_iOp == 0)
				{
					return;
				}
				sql.replace(0, i + 1, "");
				//cout <<"ʣ��"<< sql << endl;
			}
		}
	}
}

void API::execSQL(Interpret SQL)
{
	int op = SQL.m_iOp;
	switch (op)
	{
	case 1:
		execCreateTable(SQL);
		break;
	case 2:
		execCreateIndex(SQL);
		break;
	case 3:
		execDropTable(SQL);
		break;
	case 4:
		execDropIndex(SQL);
		break;
	case 5:
		execSelectALL(SQL);
		break;
	case 6:
		execSelectAttr(SQL);
		break;
	case 7:
		execInsert(SQL);
		break;
	case 8:
		execDeleteAll(SQL);
		break;
	case 9:
		execDeletePart(SQL);
		break;
	case 10:
		//cout << "ִ�нű��ļ�" << endl;
		execFile(SQL);
		break;
	case 0:
		//quit
		cout << "Exit." << endl;
		break;
	case 99:
		cout << "Check your input." << endl;
		break;
	default:
		cout << "wrong op!" << endl;
	}
}
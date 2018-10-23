#include"CatalogManager.h"
#include<fstream>
#include<iostream>

using namespace std;

CatalogManager::CatalogManager()
{
	int i, j;

	Index index;
	string str;
	string fileName = "file/catalog/tableCatalog.txt";
	ifstream  fin, finTB;
	finTB.open(fileName);
	finTB >> tableNum;
	for (i = 0; i < tableNum; i++)
	{
		Table table;
		finTB >> table.name;
		finTB >> table.blockNum;
		finTB >> table.attrNum;
		table.totalLength = 0;
		for (j = 0; j < table.attrNum; j++)
		{
			Attr tempAttr;
			finTB >> tempAttr.name;
			finTB >> tempAttr.type;
			finTB >> tempAttr.length;
			finTB >> tempAttr.indexName;
			finTB >> tempAttr.isPrimeryKey;
			finTB >> tempAttr.isIndexed;
			finTB >> tempAttr.isUnique;
			table.totalLength += tempAttr.length;
			table.attributes.push_back(tempAttr);
		}
		tables.push_back(table);
	}




	string fileName1 = "file/catalog/indexCatalog.txt";
	fin.open(fileName1);
	fin >> indexNum;
	for (i = 0; i < indexNum; i++)
	{
		fin >> index.name;
		fin >> index.value;
		fin >> index.type;
		fin >> index.length;
		fin >> index.blockNum;
		fin >> index.tableName;
		indices.push_back(index);
	}

	finTB.close();
	fin.close();

	remove(fileName.c_str());
	remove(fileName1.c_str());
	//for (i = 0; i < tableNum; i++)
	//{
	//	Table table;
	//	table = tables[i];
	//	cout<< table.name <<endl;
	//	cout << table.blockNum <<endl;
	//	cout << table.attrNum <<endl;
	//	for (j = 0; j < table.attrNum; j++)
	//	{
	//		Attr tempAttr;
	//		tempAttr = table.attributes[j];
	//		cout<< tempAttr.name << endl;
	//		cout << tempAttr.type << endl;
	//		cout << tempAttr.length << endl;
	//		cout << tempAttr.indexName << endl;
	//		cout << tempAttr.isPrimeryKey << endl;
	//		cout << tempAttr.isIndexed << endl;
	//		cout << tempAttr.isUnique << endl;
	//	}
	//	cout << table.totalLength << endl;
	//}

	//for (i = 0; i < indexNum; i++)
	//{
	//	index = indices[i];
	//	cout<< index.name <<endl;
	//	cout<< index.value <<endl;
	//	cout<< index.type <<endl;
	//	cout<< index.length<<endl;
	//	cout<< index.blockNum <<endl;
	//	cout<< index.tableName <<endl;
	//}


	//tableNum = 0;
	//indexNum = 0;
}
CatalogManager::~CatalogManager()
{
	int i, j;
	Table table;
	Index index;

	ofstream  fout, foutTB;

	string fileName = "file/catalog/tableCatalog.txt";
	foutTB.open(fileName, ios::trunc);
	foutTB << tableNum << " ";

	for (i = 0; i < tableNum; i++)
	{
		table = tables[i];
		foutTB << table.name.c_str() << " ";
		foutTB << table.blockNum << " ";
		foutTB << table.attrNum << " ";
		table.totalLength = 0;
		for (j = 0; j < table.attrNum; j++)
		{
			Attr tempAttr;
			tempAttr = table.attributes[j];
			foutTB << tempAttr.name.c_str() << " ";
			foutTB << tempAttr.type << " ";
			foutTB << tempAttr.length << " ";
			foutTB << tempAttr.indexName.c_str() << " ";
			foutTB << tempAttr.isPrimeryKey << " ";
			foutTB << tempAttr.isIndexed << " ";
			foutTB << tempAttr.isUnique << " ";
		}
	}


	string fileName1 = "file/catalog/indexCatalog.txt";
	fout.open(fileName1, ios::trunc);

	fout << indexNum << " ";
	for (i = 0; i < indexNum; i++)
	{
		index = indices[i];
		fout << index.name << " ";
		fout << index.value << " ";
		fout << index.type << " ";
		fout << index.length << " ";
		fout << index.blockNum << " ";
		fout << index.tableName << " ";
	}

	foutTB.close();
	fout.close();
}

void CatalogManager::Create_Table(Table table)
{
	tableNum++;
	tables.push_back(table);
}

void CatalogManager::Create_Index(Index index)
{
	indexNum++;
	indices.push_back(index);
}

bool CatalogManager::isTable(string tableName)
{
	int i;
	for (i = 0; i < tableNum; i++)
	{
		if (tables[i].name == tableName)
		{
			return true;
		}
	}
	return false;
}

bool CatalogManager::isIndex(string indexName)
{
	int i;
	for (i = 0; i < indexNum; i++)
	{
		if (indices[i].value == indexName)
		{
			return true;
		}
	}
	return false;
}

bool CatalogManager::isAttribution(string tableName, string attributeName)
{
	int i, j;
	for (i = 0; i < tableNum; i++)
	{
		if (tables[i].name == tableName)
		{
			for (j = 0; j < tables[i].attrNum; j++)
			{
				if (tables[i].attributes[j].name == attributeName)
				{
					return true;
				}
			}
			return false;
		}
	}
	return false;
}

void CatalogManager::Drop_Table(string tableName)
{
	int i;
	for (i = 0; i < tableNum; i++)
	{
		if (tables[i].name == tableName)
		{
			tableNum--;
			tables.erase(tables.begin() + i);
			return;
		}
	}
}

void CatalogManager::Drop_Index(string indexName)
{
	int i, j, n;
	string tName;
	string attrName;
	for (i = 0; i < indexNum; i++)
	{
		if (indices[i].value == indexName)
		{
			attrName = indices[i].name;
			tName = indices[i].tableName;
			indexNum--;
			indices.erase(indices.begin() + i);

			for (n = 0; n < tableNum; n++)
			{
				if (tables[n].name == tName)
				{
					for (j = 0; j < tables[n].attrNum; j++)
					{
						if (tables[n].attributes[j].name == attrName)
						{
							tables[n].attributes[j].isIndexed = 0;
						}
					}
				}
			}
			return;
		}
	}


}


Table* CatalogManager::getTable(string tableName)
{
	int i;
	for (i = 0; i < tableNum; i++)
	{
		if (tables[i].name == tableName)
		{
			return &tables[i];
		}
	}
	return NULL;
}

Index CatalogManager::getIndex(string indexName)
{
	int i;
	for (i = 0; i < indexNum; i++)
	{
		if (indices[i].value == indexName)
		{
			return indices[i];

		}
	}
}

void CatalogManager::addTableBlockNum(string tableName)
{
	int i;
	for (i = 0; i < tableNum; i++)
	{
		if (tables[i].name == tableName)
		{
			tables[i].blockNum++;
		}
	}
}
void CatalogManager::addIndexBlockNum(string indexName)
{
	int i;
	for (i = 0; i < indexNum; i++)
	{
		if (indices[i].value == indexName)
		{
			indices[i].blockNum++;
		}
	}
}

void CatalogManager::subTableBlockNum(string tableName)
{
	int i;
	for (i = 0; i < tableNum; i++)
	{
		if (tables[i].name == tableName)
		{
			tables[i].blockNum--;
		}
	}
}
void CatalogManager::subIndexBlockNum(string indexName)
{
	int i;
	for (i = 0; i < indexNum; i++)
	{
		if (indices[i].value == indexName)
		{
			indices[i].blockNum--;
		}
	}
}

int CatalogManager::getAttrType(string tableName, int n)
{
	int i, j;
	for (i = 0; i < tableNum; i++)
	{
		if (tables[i].name == tableName)
		{
			return tables[i].attributes[n].type;
		}
	}
	return -1;
}

int CatalogManager::getAttrType(string tableName, string attrName)
{
	int i, j;
	for (i = 0; i < tableNum; i++)
	{
		if (tables[i].name == tableName)
		{
			for (j = 0; j < tables[i].attrNum; j++)
			{
				if (tables[i].attributes[j].name == attrName)
				{
					return tables[i].attributes[j].type;
				}
			}
		}
	}
	return -1;
}

Index* CatalogManager::getIndexfromTable(string tableName, string attrName)
{
	int i, j;
	for (i = 0; i < tableNum; i++)
	{
		if (tables[i].name == tableName)
		{
			for (j = 0; j < tables[i].attrNum; j++)
			{
				if (tables[i].attributes[j].name == attrName)
				{
					int m, n;
					for (m = 0; m < indexNum; m++)
					{
						if (indices[m].name == attrName && indices[m].tableName == tableName)
						{
							//cout << "success" << endl;
							return &indices[m];
						}
					}
				}
			}
		}
	}
	return NULL;
}

bool CatalogManager::matchType(string word, string table, int n)
{
	int type = getAttrType(table, n);
	int i, flag;
	flag = 0;
	for (i = 0; i < word.length(); i++)
	{
		if (!((word[i] >= '0'&&word[i] <= '9') || (word[i] == '.')|| (word[i] >= 'a'&&word[i] <= 'f')))
		{
			flag = 2;
			break;
		}
		else if (word[i] == '.')
		{
			flag = 1;
			break;
		}
	}
	//cout << "flag " << flag << endl;
	//cout << "type " << type << endl;
	if (flag == type || flag == 0 && type == 1)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool CatalogManager::Type(string attr, string word, string table)
{
	int i, j, flag, type;
	type = -1;
	flag = 0;
	for (i = 0; i < word.length(); i++)
	{
		if (!((word[i] >= '0'&&word[i] <= '9') || (word[i] == '.')))
		{
			flag = 2;
			break;
		}
		else if (word[i] == '.')
		{
			flag = 1;
			break;
		}
	}

	for (i = 0; i < tableNum; i++)
	{
		if (tables[i].name == table)
		{
			for (j = 0; j < tables[i].attrNum; j++)
			{
				if (tables[i].attributes[j].name == attr)
				{
					type = tables[i].attributes[j].type;
					goto exit;
				}
			}
		}
	}
exit:
	if (type == flag)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CatalogManager::tableBlockNumRefresh(Table tableInfo)
{
	int i, j;
	for (i = 0; i < tableNum; i++)
	{
		if (tables[i].name == tableInfo.name)
		{
			tables[i].blockNum = 1;
			return;
		}
	}
}

void CatalogManager::indexBlockNumRefresh(Index indexInfo)
{
	int i, j;
	for (i = 0; i < indexNum; i++)
	{
		if (indices[i].name == indexInfo.name)
		{
			indices[i].blockNum = 1;
			return;
		}
	}
}
#pragma once
#include"stdafx.h"
class CatalogManager
{
public:
	CatalogManager();
	~CatalogManager();
	void Create_Table(Table table);
	void Create_Index(Index index);
	void Drop_Table(string tableName);
	void Drop_Index(string indexName);
	void addTableBlockNum(string tableName);
	void addIndexBlockNum(string indexName);
	void subTableBlockNum(string tableName);
	void subIndexBlockNum(string indexName);
	void tableBlockNumRefresh(Table tableInfo);
	void CatalogManager::indexBlockNumRefresh(Index indexInfo);
	int getAttrType(string tableName, int n);
	int getAttrType(string tableName, string attrName);
	Index* getIndexfromTable(string tableName, string attrName);
	bool isTable(string tableName);
	bool isIndex(string indexName);
	bool matchType(string word, string table, int n);
	Table *getTable(string tableName);
	Index getIndex(string indexName);
	bool isAttribution(string tableName, string attributeName);
	bool Type(string attr, string word, string table);
	vector <Table> tables;
	int tableNum;
	vector <Index> indices;
	int indexNum;
};
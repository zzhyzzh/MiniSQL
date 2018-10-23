#pragma once
#include"Interpreter.h"
#include"CatalogManager.h"
#include"IndexManager.h"
#include"RecordManager.h"
#include"stdafx.h"
class API {
public:
	API() {};
	virtual ~API() {};
	/*void createTable(Table table);
	static bool dropTable(Table table);
	static bool createIndex(Index index);
	static bool dropIndex(Index index);
	static bool selectRecord(string tableName, vector<string>& columns, vector<Condition>& conds);
	static bool insertRecord(string tableName, vector<string>& values);
	static bool deleteRecord(string tableName);
	static bool quit();*/
	void execSQL(Interpret SQL);
	void execCreateTable(Interpret SQL);
	void execCreateIndex(Interpret SQL);
	void execDropTable(Interpret SQL);
	void execDropIndex(Interpret SQL);
	void execSelectALL(Interpret SQL);
	void execSelectAttr(Interpret SQL);
	void execInsert(Interpret SQL);
	void execDeleteAll(Interpret SQL);
	void execDeletePart(Interpret SQL);
	void execFile(Interpret SQL);
};
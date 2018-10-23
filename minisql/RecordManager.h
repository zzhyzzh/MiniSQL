#pragma once
#include"stdafx.h"
#include"IndexManager.h"
#include"CatalogManager.h"

//record块中，每个块块头1—4字节标记当前块记录数
class RecordManager
{
public:
	//创建表
	void createTable(Table &tableInfo);

	//删除表，即删除表文件
	void dropTable(Table &tableInfo);

	//内部调用：计算块中能存储的最大记录数
	int maxRecordNumberPerBlock(Table tableInfo);

	//内部调用：如果表文件中某个块已满，返回当前块中最后一条记录
	insertPointer lastRecordInBlock(Table tableInfo, int blockNum);

	//内部调用：返回当前块中第一条记录
	insertPointer firstRecordInBlock(Table tableInfo, int blockNum);

	//内部调用：判断块中记录是否已满
	bool isFull(Table tableInfo, int blockNum);

	//插入记录
	void insertValue(Table &tableInfo, insertPointer InfoLine);

	//内部调用：插入记录，不更新索引值
	Offset* insert(Table &tableInfo, insertPointer InfoLine);

	//无条件查找全部属性输出
	void select(Table tableInfo);

	//无条件查找部分属性输出
	void select(Table tableInfo, attrPointer selections);

	//条件查找全部属性输出
	void select(Table tableInfo, condiPointer conditions);

	// 条件查找部分属性输出
	void select(Table tableInfo, condiPointer conditions, attrPointer selections);

	//无条件删除
	void deleteTable(Table tableInfo);

	//条件删除
	void deleteTable(Table &tableInfo, condiPointer conditions);

	//内部调用：删除某一条记录
	string deleteRecord(Table &tableInfo, Offset record);

	//内部调用：删除后，用于表中记录索引值的更新
	void indexUpdateAfterDelete(Table tableInfo, Offset record, string recordToDelete);

	//内部调用：插入后，用于表中记录索引值的更新
	void  indexUpdateAfterInsert(Table tableInfo, Offset position);

	//内部调用：全部表头输出
	void titleOutput(Table tableInfo);

	//内部调用：部分表头输出
	void titleOutput(Table tableInfo, attrPointer selections);

	//内部调用：全部输出一条记录
	void recordOutput(Table tableInfo, string tempRecord);

	//内部调用：部分输出一条记录
	void recordOutput(Table tableInfo, vector<string> tempRecord, attrPointer selections);
};

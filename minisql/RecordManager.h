#pragma once
#include"stdafx.h"
#include"IndexManager.h"
#include"CatalogManager.h"

//record���У�ÿ�����ͷ1��4�ֽڱ�ǵ�ǰ���¼��
class RecordManager
{
public:
	//������
	void createTable(Table &tableInfo);

	//ɾ������ɾ�����ļ�
	void dropTable(Table &tableInfo);

	//�ڲ����ã���������ܴ洢������¼��
	int maxRecordNumberPerBlock(Table tableInfo);

	//�ڲ����ã�������ļ���ĳ�������������ص�ǰ�������һ����¼
	insertPointer lastRecordInBlock(Table tableInfo, int blockNum);

	//�ڲ����ã����ص�ǰ���е�һ����¼
	insertPointer firstRecordInBlock(Table tableInfo, int blockNum);

	//�ڲ����ã��жϿ��м�¼�Ƿ�����
	bool isFull(Table tableInfo, int blockNum);

	//�����¼
	void insertValue(Table &tableInfo, insertPointer InfoLine);

	//�ڲ����ã������¼������������ֵ
	Offset* insert(Table &tableInfo, insertPointer InfoLine);

	//����������ȫ���������
	void select(Table tableInfo);

	//���������Ҳ����������
	void select(Table tableInfo, attrPointer selections);

	//��������ȫ���������
	void select(Table tableInfo, condiPointer conditions);

	// �������Ҳ����������
	void select(Table tableInfo, condiPointer conditions, attrPointer selections);

	//������ɾ��
	void deleteTable(Table tableInfo);

	//����ɾ��
	void deleteTable(Table &tableInfo, condiPointer conditions);

	//�ڲ����ã�ɾ��ĳһ����¼
	string deleteRecord(Table &tableInfo, Offset record);

	//�ڲ����ã�ɾ�������ڱ��м�¼����ֵ�ĸ���
	void indexUpdateAfterDelete(Table tableInfo, Offset record, string recordToDelete);

	//�ڲ����ã���������ڱ��м�¼����ֵ�ĸ���
	void  indexUpdateAfterInsert(Table tableInfo, Offset position);

	//�ڲ����ã�ȫ����ͷ���
	void titleOutput(Table tableInfo);

	//�ڲ����ã����ֱ�ͷ���
	void titleOutput(Table tableInfo, attrPointer selections);

	//�ڲ����ã�ȫ�����һ����¼
	void recordOutput(Table tableInfo, string tempRecord);

	//�ڲ����ã��������һ����¼
	void recordOutput(Table tableInfo, vector<string> tempRecord, attrPointer selections);
};

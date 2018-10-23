#pragma once
#include<string>
using namespace std;
#include"stdafx.h"

//ָ�����룺

//create table 1 index 2
//drop table 3,drop index 4 
//select * 5 select ���ԣ���Ϊ������ԣ� 6
//insert 7
//delete 8  ��where���� 9
//execfile 10
//quit 0
//error 99

class Interpret {
	
public:
	friend class API;
	int m_iOp;		//Ҫִ�еĲ�����������
							
	string m_strTablename;		//����
	string m_strIndexname;		//������
	string m_strFilename;		//�ļ���,execfile�õ�
	attribute *m_attr;			//��������,create,select���õ�
	condition *m_condi;			//where�����־�����delete,select���õ�
	insertVal *m_values;		//�����ֵ����insert�õ�

	Interpret();
	~Interpret() {};
	void handle(string& sql);

protected:
	void initAttr(attribute *p);
	void initCond(condition *p);
	void initValue(insertVal *p);

	void initAll();
	string intTo16(string s);
	//��Ϊint��float�͵�ֵ������string��ŵģ�������Ҫһ���������������б�
	int isInt(string str);//�����int����1
	int isFloat(string str);//�����float����1
	int getWord(string& src, string& des);//��src�ַ����еõ�һ���ʣ��������ո񡢻س���TAB��������'��'��'��'��'*'��'��'��'��'�������ȡʧ�ܷ���0
	int getStr(string& src, string& des);//��ȡ�ַ����������ո�ȣ���Ҫ�ǻ�ȡ����֮�������
};

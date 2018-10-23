#pragma once
#include<string>
using namespace std;
#include"stdafx.h"

//指令译码：

//create table 1 index 2
//drop table 3,drop index 4 
//select * 5 select 属性（可为多个属性） 6
//insert 7
//delete 8  带where条件 9
//execfile 10
//quit 0
//error 99

class Interpret {
	
public:
	friend class API;
	int m_iOp;		//要执行的操作或错误代码
							
	string m_strTablename;		//表名
	string m_strIndexname;		//索引名
	string m_strFilename;		//文件名,execfile用到
	attribute *m_attr;			//属性链表,create,select中用到
	condition *m_condi;			//where条件字句链表，delete,select中用到
	insertVal *m_values;		//插入的值链表，insert用到

	Interpret();
	~Interpret() {};
	void handle(string& sql);

protected:
	void initAttr(attribute *p);
	void initCond(condition *p);
	void initValue(insertVal *p);

	void initAll();
	string intTo16(string s);
	//因为int和float型的值都是用string存放的，所以需要一下两个函数将其判别
	int isInt(string str);//如果是int返回1
	int isFloat(string str);//如果是float返回1
	int getWord(string& src, string& des);//从src字符串中得到一个词，不包括空格、回车、TAB，可以是'（'、'）'、'*'、'，'、'；'，如果获取失败返回0
	int getStr(string& src, string& des);//获取字符串，包括空格等，主要是获取‘’之间的内容
};

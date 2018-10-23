#pragma once
#include<string>
#include<sstream>
#include<vector>
#define BLOCK_SIZE 4096
using namespace std;

typedef struct tableinfo Table;
typedef struct indexinfo Index;
typedef struct attrInfo Attr;
typedef struct offsetInfo Offset;
typedef struct attrColumn* attrPointer;
typedef struct whereCondi* condiPointer;
typedef struct insertValue* insertPointer;

typedef enum type blockType;
typedef enum type fileType;
enum type { index, record, uncertain };



typedef struct attrColumn
{
	string attrName;//属性名
	string attrType;//属性类型可以是int,char,float
	int charLen;//如果是char类型，有长度大小
	int isUnique;//1表示为unique
	int isPrimary;//1表示为主键
	attrPointer next;
}attribute;

class Record   //Record类，用来存储一条记录
{
public:
	vector<string> columns;
	int blockNum;
	int offset;

	string getString()
	{
		string totalRecord = "";
		for (int i = 0; i < columns.size(); i++)
		{
			totalRecord += columns[i];
		}
		return totalRecord;
	}
};

typedef struct whereCondi
{
	string attrName;//要选择的属性名，如果是select * ，则为空
	string op;//<,>,>=等比较符
	string value;//与属性比较的值
	int type;//如果为char则等于1，否则等于0，初始化为-1
	int andor;//如果等于1，则条件为and,等于0为or,初始化为-1
	condiPointer next;
}condition;


typedef struct insertValue
{
	int type;//是char则为1，否则为0
			 //int charLen;//插入时，char类型有长度限制
	string value;
	insertPointer next;
}insertVal;


struct tableinfo
{
	string name;
	int blockNum;
	int attrNum;
	int totalLength;
	vector<Attr> attributes;
};

struct indexinfo
{
	int blockNum;
	int type;
	int length;
	string tableName;
	string value;
	string name;
};

struct attrInfo
{
	string name;
	int type; //0:int  1:float  2:char
	int length;
	bool isPrimeryKey;
	bool isUnique;
	bool isIndexed;
	string indexName;
	vector<condition> factor;//仅在条件判断时由record manager使用。其他情况下为空。

};

struct offsetInfo
{
	int blockNum;
	int offset;
};


class Block
{
public:							
	string fileName;							//当前缓冲块对应的文件名
	blockType type;								//标记块的属性：index 或 record
	int blockNum;								//当前缓冲块在文件中的位置
	char* CBlock;								//块内信息
	int charNum;
	bool isDirty;								//标志该块内字符是否被修改
	Block* next;

	Block()
	{
		isDirty = false;
		blockNum = 0;
		fileName = "";
		charNum = 0;
		type = (blockType)uncertain;
		CBlock = new char[BLOCK_SIZE];
		next = NULL;
	}
	void setBlockLocation(string FileName, int offset)
	{
		fileName = FileName;
		blockNum = offset;
	}

	void setBlockValue(const char* values)//用于record块信息初始化
	{
		memcpy(CBlock + 4, values, strlen(values));//将字符串values追加到CBlock后
		isDirty = true;
		charNum = strlen(values);
	}

	//获得record块中记录数
	int getRecordNum()
	{
		string temp = CBlock;
		string record = temp.substr(0, 4);
		int num = atoi(record.c_str());						//将块头四字节的记录数转化成int
		return num;
	}
	//修改块中记录数信息
	void setRecordNum(int value)
	{
		stringstream ss;
		string intInString;
		ss << value;
		ss >> intInString;

		int length = intInString.length();
		for (int i = 0; i < 4 - length; i++)
		{
			intInString = "0" + intInString;
		}

		memcpy(CBlock, intInString.c_str(), 4);
		isDirty = true;
	}

	//块中记录数信息加一
	void setRecordNumAddOne()
	{
		int temp = getRecordNum();
		temp++;
		setRecordNum(temp);
	}

	//块中记录数信息减一
	void setRecordNumSubOne()
	{
		int temp = getRecordNum();
		temp--;
		setRecordNum(temp);
	}

};

class File
{
public:
	vector<int> emptyBlocks;	//记录文件空块信息
	int lastEmptyBlock;			//记录文件末尾后的空块位置
	string fileName;
	fileType type;
	File* next;					//链表指针

	File()
	{
		lastEmptyBlock = 0;
		type = uncertain;
		next = NULL;
		fileName = "";
	}
	File(fileType Type, string name)
	{
		type = Type;
		fileName = name;
		lastEmptyBlock = 0;
		next = NULL;
	}
	//~File() { ; }
};

class primaryKey
{
public:
	string PrimaryValue;	//主键信息
	int primaryOrder;	//主键在属性中的排序
	int primaryPosition; //主键在记录中的字节位置
	int primaryLength;	//主键的长度

	primaryKey(Table tableInfo, insertPointer InfoLine)
	{
		primaryPosition = 0;
		for (primaryOrder = 0; primaryOrder < tableInfo.attributes.size(); primaryOrder++)
		{
			if (tableInfo.attributes[primaryOrder].isPrimeryKey)
			{
				break;
			}
			primaryPosition += tableInfo.attributes[primaryOrder].length;
		}//找到主键的位置

		primaryLength = tableInfo.attributes[primaryOrder].length;//主键的长度

		insertPointer temp = InfoLine;
		for (int i = 0; i < primaryOrder; i++)
		{
			temp = temp->next;
		}
		PrimaryValue = temp->value;//获得插入信息的主键值		
	}
};

//包括了对主键和unique的判断，需要和insert函数组合使用，否则不会返回正确的信息
class unique
{
public:
	string uniqueValue;	//信息
	int uniquePosition; //在记录中的字节位置
	int uniqueLength;	//长度

	unique(Table tableInfo, int attributeOrder, string record)
	{
		uniquePosition = 0;
		uniqueValue = "";
		uniqueLength = 0;

		if (tableInfo.attributes[attributeOrder].isUnique)
		{
			for (int i = 0; i < attributeOrder; i++)
			{
				uniquePosition += tableInfo.attributes[i].length;//找到unique的位置
			}
			uniqueLength = tableInfo.attributes[attributeOrder].length;//unique属性的长度
			uniqueValue = record.substr(uniquePosition, tableInfo.attributes[attributeOrder].length);
			//unique属性值
		}
	}
};
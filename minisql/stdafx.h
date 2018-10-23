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
	string attrName;//������
	string attrType;//�������Ϳ�����int,char,float
	int charLen;//�����char���ͣ��г��ȴ�С
	int isUnique;//1��ʾΪunique
	int isPrimary;//1��ʾΪ����
	attrPointer next;
}attribute;

class Record   //Record�࣬�����洢һ����¼
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
	string attrName;//Ҫѡ����������������select * ����Ϊ��
	string op;//<,>,>=�ȱȽϷ�
	string value;//�����ԱȽϵ�ֵ
	int type;//���Ϊchar�����1���������0����ʼ��Ϊ-1
	int andor;//�������1��������Ϊand,����0Ϊor,��ʼ��Ϊ-1
	condiPointer next;
}condition;


typedef struct insertValue
{
	int type;//��char��Ϊ1������Ϊ0
			 //int charLen;//����ʱ��char�����г�������
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
	vector<condition> factor;//���������ж�ʱ��record managerʹ�á����������Ϊ�ա�

};

struct offsetInfo
{
	int blockNum;
	int offset;
};


class Block
{
public:							
	string fileName;							//��ǰ������Ӧ���ļ���
	blockType type;								//��ǿ�����ԣ�index �� record
	int blockNum;								//��ǰ��������ļ��е�λ��
	char* CBlock;								//������Ϣ
	int charNum;
	bool isDirty;								//��־�ÿ����ַ��Ƿ��޸�
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

	void setBlockValue(const char* values)//����record����Ϣ��ʼ��
	{
		memcpy(CBlock + 4, values, strlen(values));//���ַ���values׷�ӵ�CBlock��
		isDirty = true;
		charNum = strlen(values);
	}

	//���record���м�¼��
	int getRecordNum()
	{
		string temp = CBlock;
		string record = temp.substr(0, 4);
		int num = atoi(record.c_str());						//����ͷ���ֽڵļ�¼��ת����int
		return num;
	}
	//�޸Ŀ��м�¼����Ϣ
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

	//���м�¼����Ϣ��һ
	void setRecordNumAddOne()
	{
		int temp = getRecordNum();
		temp++;
		setRecordNum(temp);
	}

	//���м�¼����Ϣ��һ
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
	vector<int> emptyBlocks;	//��¼�ļ��տ���Ϣ
	int lastEmptyBlock;			//��¼�ļ�ĩβ��Ŀտ�λ��
	string fileName;
	fileType type;
	File* next;					//����ָ��

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
	string PrimaryValue;	//������Ϣ
	int primaryOrder;	//�����������е�����
	int primaryPosition; //�����ڼ�¼�е��ֽ�λ��
	int primaryLength;	//�����ĳ���

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
		}//�ҵ�������λ��

		primaryLength = tableInfo.attributes[primaryOrder].length;//�����ĳ���

		insertPointer temp = InfoLine;
		for (int i = 0; i < primaryOrder; i++)
		{
			temp = temp->next;
		}
		PrimaryValue = temp->value;//��ò�����Ϣ������ֵ		
	}
};

//�����˶�������unique���жϣ���Ҫ��insert�������ʹ�ã����򲻻᷵����ȷ����Ϣ
class unique
{
public:
	string uniqueValue;	//��Ϣ
	int uniquePosition; //�ڼ�¼�е��ֽ�λ��
	int uniqueLength;	//����

	unique(Table tableInfo, int attributeOrder, string record)
	{
		uniquePosition = 0;
		uniqueValue = "";
		uniqueLength = 0;

		if (tableInfo.attributes[attributeOrder].isUnique)
		{
			for (int i = 0; i < attributeOrder; i++)
			{
				uniquePosition += tableInfo.attributes[i].length;//�ҵ�unique��λ��
			}
			uniqueLength = tableInfo.attributes[attributeOrder].length;//unique���Եĳ���
			uniqueValue = record.substr(uniquePosition, tableInfo.attributes[attributeOrder].length);
			//unique����ֵ
		}
	}
};
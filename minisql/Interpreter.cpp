#include<iostream>
#include<string> 
#include <sstream> //要使用stringstream流应包含此头文件 
#include"Interpreter.h"
#include"CatalogManager.h"
#include<stdlib.h>
using namespace std;


extern Interpret globalI;
extern CatalogManager *catalogManager;
Interpret::Interpret()
{
	m_iOp = -1;//UNKNOWN
	m_strTablename = "";
	m_strIndexname = "";
	m_strFilename = "";
	m_attr = NULL;
	m_condi = NULL;
	m_values = NULL;
};
//初始化属性列表
void Interpret::initAttr(attribute *p)
{
	p->attrName = "";
	p->attrType = "";
	p->charLen = 0;
	p->isPrimary = 0;
	p->isUnique = 0;
	p->next = NULL;
}
//初始化where条件列表
void Interpret::initCond(condition *p)
{
	p->attrName = "";
	p->op = "";
	p->value = "";
	p->type = -1;
	p->andor = -1;
	p->next = NULL;
}
//初始化插入的值列表
void Interpret::initValue(insertVal *p)
{
	//p->charLen = 0;
	p->type = -1;
	p->value = "";
	p->next = NULL;
}

void Interpret::initAll()
{
	m_iOp = -1;//UNKNOWN
	m_strTablename = "";
	m_strIndexname = "";
	m_strFilename = "";
	m_attr = NULL;
	m_condi = NULL;
	m_values = NULL;
}

string Interpret::intTo16(string s)//把int字符串传入，转为8位16进制字符串形式 
{
	string res = "";
	int sum = 0,t=0;
	for (int i = 0; i<s.length(); i++)
	{
		sum += (s.at(i) - '0') * pow(10, s.length() - i - 1);
	}
	do
	{
		t = sum % 16;
		sum /= 16;
		if(t<10)
			res = (char)(t + '0') + res;
		else
			res = (char)(t-10 + 'a') + res;
	} while (sum);

	if (res == "0")//是0 
	{
		res = "00000000";
	}
	else if (res.length() < 8)
	{
		while(res.length() <8)
			res = "0" + res;
	}

	//cout <<"int:"<< res << endl;
	return res;
}

int Interpret::isInt(string str)
{
	if (!isdigit(str.at(0)) && !(str.at(0) == '-'))
		return 0;
	for (int i = 1; i < str.length(); i++)
	{
		if (!isdigit(str.at(i)))
			return 0;
	}
	return 1;
}
int Interpret::isFloat(string str)
{
	if (!isdigit(str.at(0)) && !(str.at(0) == '-'))
		return 0;
	int dotnum = 0;//'.'的个数，超过一则返回0
	for (int i = 1; i < str.length(); i++)
	{
		if (!isdigit(str.at(i)) && str.at(i) != '.')
			return 0;
		else if (str.at(i) == '.')
		{
			if (dotnum == 0)
			{
				dotnum++;
				break;
			}
			else
				return 0;
		}
	}
	if (dotnum == 0)
	{
		return 0;
	}
	return 1;
}
int Interpret::getWord(string& src, string& des)
{
	unsigned int srcpos = 0, despos = 0;
	char temp = ' ';

	//cout<<src<<","<<des<<endl;
	des.clear();

	for (; srcpos<src.length(); srcpos++) {
		if (temp == ' ' || temp == '\t' || temp == 10 || temp == 13)
			temp = src.at(srcpos);
		else break;
	}
	if (srcpos == src.length() && (temp == ' ' || temp == '\t' || temp == 10 || temp == 13))
		return 0;

	switch (temp)
	{
	case ',':
	case '(':
	case ')':
	case '*':
	case '=':
	case '\'':
	case ';':
		des += temp;
		despos++;
		despos++;
		src.erase(0, srcpos);
		break;
	case '<':
		des += temp;
		despos++;
		temp = src.at(srcpos++);
		if (temp == '=' || temp == '>')
		{
			des += temp;
			despos++;
			despos++;
			src.erase(0, srcpos);
		}
		else
		{
			despos++;
			src.erase(0, srcpos - 1);
		}
		break;
	case '>':
		des += temp;
		despos++;
		temp = src.at(srcpos++);
		if (temp == '=')
		{
			des += temp;
			despos++;
			//	des+= '\0';
			despos++;
			src.erase(0, srcpos);
		}
		else
		{
			//	des+= '\0';
			despos++;
			src.erase(0, srcpos - 1);
		}
		break;
	default:
		do {
			des += temp;
			despos++;
			if (srcpos < src.length())
				temp = src.at(srcpos++);
			else {
				src.erase(0, srcpos);
				//	des+= '\0';
				despos++;
				return 1;
			}
		} while (temp != ';' &&temp != '*' && temp != ',' && temp != '(' && temp != ')'
			&& temp != ' ' && temp != '\t' && temp != '=' && temp != '>'
			&& temp != '<' && temp != '\'' && temp != 10 && temp != 13);
		src.erase(0, srcpos - 1);
		//des+= '\0';
		despos++;
	}
	return 1;
}
int Interpret::getStr(string& src, string& des)
{
	des.clear();
	char t;
	if (src.at(0) == '\'')//‘’间无字符
	{
		des = "";
		return 1;
	}
	else
	{
		for (int i = 0; i < src.length(); i++)
		{
			if (src.at(i) != '\'')
			{
				t = src.at(i);
				des += t;
			}
			else
			{
				src.erase(0, i);
				return 1;
			}
		}
	}
	return 0;
}
void Interpret::handle(string& sql)
{
	int sqlpos = 0;
	string res = "";
	attribute *lastattr = NULL;
	attribute *tempattr = NULL;
	condition *lastcon = NULL;
	condition *tempcon = NULL;
	insertVal *lastval = NULL;
	insertVal *tempval = NULL;
	initAll();
	//如果没有获取到内容，op=99并返回 
	if (!getWord(sql, res))
	{
		m_iOp = 99;
		cout << "error! Empty command." << endl;
		return;
	}

	if (res == "quit")
	{
		//如果没有分号，返回错误 
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! ';' is missing." << endl;
		}
		else
		{
			if (res != ";")//如果quit后没有分号 
			{
				cout << "error! ';' is missing." << endl;
				m_iOp = 99;
			}
			else
			{
				m_iOp = 0;//退出 
			}
		}
	}

	else if (res == "create")
	{
		//initAll();
		if (getWord(sql, res))
		{
			//create table
			if (res == "table")
			{
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'create' command isn't complete." << endl;
					return;
				}
				m_strTablename = res;//存储表名
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'create' command isn't complete." << endl;
					return;
				}
				if (res != "(")
				{
					m_iOp = 99;
					cout << "error! The 'create' command has some syntax error.Check your '(' ." << endl;
					return;
				}
				m_iOp = 1;
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'create' command isn't complete." << endl;
					return;
				}
				tempattr = new attribute;
				initAttr(tempattr);
				tempattr->attrName = res;
				m_attr = lastattr = tempattr;
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'create' command isn't complete." << endl;
					return;
				}
				if (res != "int"&&res != "char"&&res != "float")//仅限于创建int char float三种类型，否则报错
				{
					m_iOp = 99;
					cout << "error! The attribute type is wrong. You can create int, float, char." << endl;
					return;
				}
				if (res == "int" || res == "float")
				{
					tempattr->attrType = res;
					if (!getWord(sql, res))
					{
						m_iOp = 99;
						cout << "error! The 'create' command isn't complete." << endl;
						return;
					}
					if (res == "unique")
					{
						tempattr->isUnique = 1;
						if (!getWord(sql, res))
						{
							m_iOp = 99;
							cout << "error! The 'create' command isn't complete." << endl;
							return;
						}
					}
				}
				//res为char
				else
				{
					tempattr->attrType = res;
					if (!getWord(sql, res))
					{
						m_iOp = 99;
						cout << "error! The 'create' command isn't complete." << endl;
						return;
					}
					if (res != "(")
					{
						m_iOp = 99;
						cout << "error!  The attribute definition is wrong. Check your char type." << endl;
						return;
					}
					//获取char的长度
					if (!getWord(sql, res))
					{
						m_iOp = 99;
						cout << "error! The 'create' command isn't complete." << endl;
						return;
					}
					if (!isInt(res))//如果输入的char长度不为整数，报错
					{
						m_iOp = 99;
						cout << "error! The char tpye definition is wrong." << endl;
						return;
					}
					stringstream stream;
					stream << res; //把res转化为整型
					stream >> tempattr->charLen;
					if (tempattr->charLen > 1500 || tempattr->charLen < 1)//char的长度为1<=n<=255
					{
						m_iOp = 99;
						cout << "error! The char length out of range." << endl;
						return;
					}
					if (!getWord(sql, res))
					{
						m_iOp = 99;
						cout << "error! The 'create' command isn't complete." << endl;
						return;
					}
					if (res != ")")
					{
						m_iOp = 99;
						cout << "error! The 'create' command has some syntax error.Check your ')' .Maybe ')' is missing." << endl;
						return;
					}
					if (!getWord(sql, res))
					{
						m_iOp = 99;
						cout << "error! The 'create' command isn't complete." << endl;
						return;
					}
					if (res == "unique")
					{
						tempattr->isUnique = 1;
						if (!getWord(sql, res))
						{
							m_iOp = 99;
							cout << "error! The 'create' command isn't complete." << endl;
							return;
						}
					}
				}
				while (res == ",")
				{
					if (!getWord(sql, res))
					{
						m_iOp = 99;
						cout << "error! The 'create' command isn't complete." << endl;
						return;
					}
					//primary
					if (res == "primary")
					{
						if (!getWord(sql, res))
						{
							m_iOp = 99;
							cout << "error! The 'create' command isn't complete." << endl;
							return;
						}
						if (res != "key")
						{
							m_iOp = 99;
							cout << "error! The primary key definition is wrong." << endl;
							return;
						}
						if (!getWord(sql, res))
						{
							m_iOp = 99;
							cout << "error! The 'create' command isn't complete." << endl;
							return;
						}
						if (res != "(")
						{
							m_iOp = 99;
							cout << "error! The primary key definition is wrong." << endl;
							return;
						}
						if (!getWord(sql, res))
						{
							m_iOp = 99;
							cout << "error! The 'create' command isn't complete." << endl;
							return;
						}
						//找到主键，将isPrimary设为1
						tempattr = m_attr;
						int isFindAttr = 0;
						while (tempattr)
						{
							if (res == tempattr->attrName)
							{
								isFindAttr = 1;
								tempattr->isPrimary = 1;
								tempattr->isUnique = 1;//主键包含unique性质
								break;
							}
							tempattr = tempattr->next;
						}
						if (!isFindAttr)//如果没有找到主键属性，报错
						{
							m_iOp = 99;
							cout << "error! The attribute of primary key is wrong." << endl;
							return;
						}
						if (!getWord(sql, res))
						{
							m_iOp = 99;
							cout << "error! The 'create' command isn't complete." << endl;
							return;
						}
						if (res != ")")
						{
							m_iOp = 99;
							cout << "error!  The 'create' command has some syntax error.Check your ')' .Maybe ')' is missing." << endl;
							return;
						}
						if (!getWord(sql, res))
						{
							m_iOp = 99;
							cout << "error! The 'create' command isn't complete." << endl;
							return;
						}
					}

					else
					{
						tempattr = new attribute;
						initAttr(tempattr);
						tempattr->attrName = res;//属性名
						if (!getWord(sql, res))
						{
							m_iOp = 99;
							cout << "error! The 'create' command isn't complete." << endl;
							return;
						}
						if (res != "int"&&res != "char"&&res != "float")//仅限于创建int char float三种类型，否则报错
						{
							m_iOp = 99;
							cout << "error! The attribute type is wrong. You can create int, float, char." << endl;
							return;
						}
						if (res == "int" || res == "float")
						{
							tempattr->attrType = res;
							if (!getWord(sql, res))
							{
								m_iOp = 99;
								cout << "error! The 'create' command isn't complete." << endl;
								return;
							}
							if (res == "unique")
							{
								tempattr->isUnique = 1;
								if (!getWord(sql, res))
								{
									m_iOp = 99;
									cout << "error! The 'create' command isn't complete." << endl;
									return;
								}
							}
						}
						else//res为char
						{
							tempattr->attrType = res;
							if (!getWord(sql, res))
							{
								m_iOp = 99;
								cout << "error! The 'create' command isn't complete." << endl;
								return;
							}
							if (res != "(")
							{
								m_iOp = 99;
								cout << "error!  The attribute definition is wrong." << endl;
								return;
							}
							//获取char的长度
							if (!getWord(sql, res))
							{
								m_iOp = 99;
								cout << "error! The 'create' command isn't complete." << endl;
								return;
							}
							if (!isInt(res))//如果输入的char长度不为整数，报错
							{
								m_iOp = 99;
								cout << "error! The char tpye definition is wrong." << endl;
								return;
							}
							stringstream stream;
							stream << res; //把res转化为整型
							stream >> tempattr->charLen;
							if (tempattr->charLen > 1500 || tempattr->charLen < 1)//char的长度为1<=n<=255
							{
								m_iOp = 99;
								cout << "error! The char length out of range." << endl;
								return;
							}
							if (!getWord(sql, res))
							{
								m_iOp = 99;
								cout << "error! The 'create' command isn't complete." << endl;
								return;
							}
							if (res != ")")
							{
								m_iOp = 99;
								cout << "error! The 'create' command has some syntax error.Check your ')' .Maybe ')' is missing." << endl;
								return;
							}
							if (!getWord(sql, res))
							{
								m_iOp = 99;
								cout << "error! The 'create' command isn't complete." << endl;
								return;
							}
							if (res == "unique")
							{
								tempattr->isUnique = 1;
								if (!getWord(sql, res))
								{
									m_iOp = 99;
									cout << "error! The 'create' command isn't complete." << endl;
									return;
								}
							}
						}
						lastattr->next = tempattr;
						lastattr = tempattr;
					}
				}
				if (res != ")")
				{
					m_iOp = 99;
					cout << "error!The 'create' command has some syntax error.Check your ')' .Maybe ')' is missing." << endl;
					return;
				}
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'create' command isn't complete.You may miss the \';\'" << endl;
					return;
				}
				if (res != ";")
				{
					m_iOp = 99;
					cout << "error! The 'create' command has some syntax error." << endl;
					return;
				}
			}

			//create index
			else if (res == "index")
			{
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'create' command isn't complete." << endl;
					return;
				}
				m_strIndexname = res;
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'create' command isn't complete." << endl;
					return;
				}
				if (res != "on")
				{
					m_iOp = 99;
					cout << "error! The 'create' command has some syntax error. You may miss 'on'. " << endl;
					return;
				}
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'create' command isn't complete." << endl;
					return;
				}
				m_strTablename = res;
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'create' command isn't complete." << endl;
					return;
				}
				if (res != "(")
				{
					m_iOp = 99;
					cout << "error! The 'create' command has some syntax error." << endl;
					return;
				}
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'create' command isn't complete." << endl;
					return;
				}
				m_iOp = 2;
				//属性名
				tempattr = new attribute;
				initAttr(tempattr);
				tempattr->attrName = res;
				m_attr = tempattr;
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'create' command isn't complete." << endl;
					return;
				}
				if (res != ")")
				{
					m_iOp = 99;
					cout << "error! The 'create' command has some syntax error. You may miss ')'. " << endl;
					return;
				}
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'create' command isn't complete.You may miss the \';\'" << endl;
					return;
				}
				if (res != ";")
				{
					m_iOp = 99;
					cout << "error! The 'create' command has some syntax erroe." << endl;
					return;
				}
			}
			//返回错误 
			else
			{
				m_iOp = 99;
				cout << "error! Can't create that type." << endl;
			}
		}
	}

	else if (res == "drop")
	{
		//initAll();
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The 'drop' command isn't complete." << endl;
			return;
		}
		if (res == "table")//如果要删除表，则把表名找出来 
		{
			m_iOp = 3;
			if (!getWord(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'drop' command isn't complete." << endl;
				return;
			}
			m_strTablename = res;
		}
		else if (res == "index")//如果要删除索引 
		{
			m_iOp = 4;
			if (!getWord(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'drop' command isn't complete." << endl;
				return;
			}
			m_strIndexname = res;
		}
		//返回错误 
		else
		{
			m_iOp = 99;
			cout << "error! Can't drop that type." << endl;
		}
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The 'drop' command isn't complete.You may miss the \';\'." << endl;
			return;
		}
		if (res != ";")
		{
			m_iOp = 99;
			cout << "error! The 'drop' command has some syntax error." << endl;
			return;
		}
	}

	else if (res == "select")
	{
		//initAll();
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The 'select' command isn't complete." << endl;
			return;
		}
		if (res == "*")
		{
			m_iOp = 5;
			if (!getWord(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'select' command isn't complete." << endl;
				return;
			}
		}
		else
		{
			m_iOp = 6;
			//存储select的属性名
			tempattr = new attribute;
			initAttr(tempattr);
			tempattr->attrName = res;
			m_attr = lastattr = tempattr;
			if (!getWord(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'select' command isn't complete." << endl;
				return;
			}
			//如果要选择多个属性
			while (res == ",")
			{
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'select' command isn't complete." << endl;
					return;
				}
				tempattr = new attribute;
				initAttr(tempattr);
				tempattr->attrName = res;
				lastattr->next = tempattr;
				lastattr = tempattr;
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'select' command isn't complete." << endl;
					return;
				}
			}
		}

		if (res != "from")
		{
			m_iOp = 99;
			cout << "error! The 'select' command has some syntax error. Check your 'from'. " << endl;
			return;
		}
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The 'select' command isn't complete." << endl;
			return;
		}
		m_strTablename = res;//要select的表名
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The 'select' command isn't complete." << endl;
			return;
		}
		if (res != ";"&&res != "where")
		{
			m_iOp = 99;
			cout << "error! The 'select' command has some syntax error. Check your 'where'. " << endl;
			return;
		}
		else if (res == ";")
		{
			return;//正确返回
		}
		else//有where字句
		{
			if (!getWord(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'select' command isn't complete." << endl;
				return;
			}
			//存储条件
			tempcon = new condition;
			initCond(tempcon);
			tempcon->attrName = res;//属性名
			if (!getWord(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'select' command isn't complete." << endl;
				return;
			}
			//如果是非法操作符
			if (res != "<"&&res != ">"&&res != "<="&&res != "<>"&&res != ">="&&res != "=")
			{
				m_iOp = 99;
				cout << "error! Check the 'where' condition. You may use illegal operations." << endl;
				return;
			}
			else
				tempcon->op = res;
			if (!getWord(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'select' command isn't complete." << endl;
				return;
			}
			if (res == "\'")//如果是char类型
			{
				tempcon->type = 1;
				if (!getStr(sql, res))
				{
					m_iOp = 99;
					cout << "error! Check the char type on the 'where' condition ." << endl;
					return;
				}
				int len = 0;
				Table* tempT = catalogManager->getTable(m_strTablename);
				for (int i = 0; i < tempT->attrNum; i++)
				{
					if (tempcon->attrName == tempT->attributes[i].name)
					{
						len = tempT->attributes[i].length;
						break;
					}
				}
				while (res.length() < len)
				{
					res += " ";
				}
				tempcon->value = res;
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'select' command isn't complete." << endl;
					return;
				}
				if (res != "\'")
				{
					m_iOp = 99;
					cout << "error! Check the char type on the 'where' condition." << endl;
					return;
				}
			}
			else
			{
				if (isInt(res))
				{
					res = intTo16(res);
				}
				else if (res.length() < 8 && isFloat(res))
				{
					while (res.length() < 8)
						res = "0" + res;
				}
				tempcon->value = res;
				tempcon->type = 0;//非char类型
			}
			m_condi = lastcon = tempcon;
			if (!getWord(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'select' command isn't complete.You may miss the\"; \" " << endl;
				return;
			}
			//where语句结束
			if (res == ";")
			{
				return;//正确返回
			}
			else if (res == "and" || res == "or")
			{
				if (res == "and")
				{
					m_condi->andor = 1;
				}
				else m_condi->andor = 0;//记录条件
				while (res == "and" || res == "or")
				{
					if (!getWord(sql, res))
					{
						m_iOp = 99;
						cout << "error! The 'select' command isn't complete.You may miss the\"; \"" << endl;
						return;
					}
					tempcon = new condition;
					initCond(tempcon);
					tempcon->attrName = res;
					if (!getWord(sql, res))
					{
						m_iOp = 99;
						cout << "error! The 'select' command isn't complete." << endl;
						return;
					}
					//如果是非法操作符
					if (res != "<"&&res != ">"&&res != "<="&&res != "<>"&&res != ">="&&res != "=")
					{
						m_iOp = 99;
						cout << "error! Check the 'where' condition.You may use illegal operations." << endl;
						return;
					}
					else
						tempcon->op = res;
					if (!getWord(sql, res))
					{
						m_iOp = 99;
						cout << "error! The 'select' command isn't complete." << endl;
						return;
					}
					if (res == "\'")//如果是char类型
					{
						tempcon->type = 1;
						if (!getStr(sql, res))
						{
							m_iOp = 99;
							cout << "error!  Check the char type on the 'where' condition." << endl;
							return;
						}
						int len = 0;
						Table* tempT = catalogManager->getTable(m_strTablename);
						for (int i = 0; i < tempT->attrNum; i++)
						{
							if (tempcon->attrName == tempT->attributes[i].name)
							{
								len = tempT->attributes[i].length;
								break;
							}
						}
						while (res.length() < len)
						{
							res += " ";
						}
						tempcon->value = res;
						if (!getWord(sql, res))
						{
							m_iOp = 99;
							cout << "error! The 'select' command isn't complete." << endl;
							return;
						}
						if (res != "\'")
						{
							m_iOp = 99;
							cout << "error! Check the char type on the 'where' condition." << endl;
							return;
						}
					}
					else
					{
						if (isInt(res))
						{
							//while (res.length() < 4)
							//	res = "0" + res;
							res = intTo16(res);
						}
						else if (res.length() < 8 && isFloat(res))
						{
							while (res.length() < 8)
								res = "0" + res;
						}
						tempcon->value = res;
						tempcon->type = 0;//非char类型
					}
					lastcon->next = tempcon;
					lastcon = tempcon;
					if (!getWord(sql, res))
					{
						m_iOp = 99;
						cout << "error! The 'select' command isn't complete.You may miss the\"; \"" << endl;
						return;
					}
				}
			}
			if (res == ";")
			{
				return;//正确返回
			}
			else
			{
				m_iOp = 99;
				cout << "error! The 'select' command isn't complete.You may miss the\";\"" << endl;
				return;
			}
		}


	}

	else if (res == "insert")
	{
		int num = 0;//记录插入的属性是第几个
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The 'insert' command isn't complete." << endl;
			return;
		}
		if (res != "into")
		{
			m_iOp = 99;
			cout << "error! The 'insert' command has some syntax error. Check the 'into'. " << endl;
			return;
		}
		m_iOp = 7;
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The 'insert' command isn't complete." << endl;
			return;
		}
		m_strTablename = res;//存储要插入数据的表名
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The 'insert' command isn't complete." << endl;
			return;
		}
		if (res != "values")
		{
			m_iOp = 99;
			cout << "error! The 'insert' command has some syntax error.Check the 'values'. " << endl;
			return;
		}
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The 'insert' command isn't complete." << endl;
			return;
		}
		if (res != "(")
		{
			m_iOp = 99;
			cout << "error! The 'insert' command has some syntax error.Check the '('. " << endl;
			return;
		}
		tempval = new insertVal;//存储插入记录
		initValue(tempval);
		m_values = lastval = tempval;
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The 'insert' command isn't complete." << endl;
			return;
		}
		//如果是char类型
		if (res == "\'")
		{
			num++;
			Table* tempT = catalogManager->getTable(m_strTablename);

			tempval->type = 1;
			int len = 0;//存贮定义的char长度
			if (!getStr(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'insert' command isn't complete." << endl;
				return;
			}
			//cout << "tempT->name:" << tempT->name << endl;
			if (tempT)
			{
				//cout << "tempT->name:" << tempT->name << endl;
				len = (*tempT).attributes[num - 1].length;
				//cout << "len:" << len << endl;
				while (res.length() < len)
				{
					res += " ";
				}
			}

			tempval->value = res;//存储插入值
			if (!getWord(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'insert' command isn't complete." << endl;
				return;
			}
			if (res != "\'")
			{
				m_iOp = 99;
				cout << "error! The 'insert' command has some syntax error.You may miss the \'" << endl;
				return;
			}
		}
		//如果不是char类型
		else
		{
			num++;
			//如果是int, 补成8位16进制
			if (isInt(res)&& catalogManager->getAttrType(m_strTablename, num-1) == 0)
			{
				res = intTo16(res);
			}
			else if (res.length() < 8 && catalogManager->getAttrType(m_strTablename, num - 1) == 1)
			{
				while (res.length() < 8)
					res = "0" + res;
			}
			tempval->value = res;
			tempval->type = 0;
		}
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The 'insert' command isn't complete." << endl;
			return;
		}
		while (res == ",")//插入多个属性值
		{
			num++;
			tempval = new insertVal;
			initValue(tempval);
			if (!getWord(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'insert' command isn't complete." << endl;
				return;
			}
			if (res == "\'")//char类型 
			{
				tempval->type = 1;
				Table* tempT = catalogManager->getTable(m_strTablename);
				int len;//存贮定义的char长度
				if (!getStr(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'insert' command isn't complete." << endl;
					return;
				}

				if (tempT)
				{
					len = (*tempT).attributes[num - 1].length;
					//cout << "len:" << len << endl;
					while (res.length() < len)
					{
						res += " ";
					}
				}

				tempval->value = res;//存储插入值
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'insert' command isn't complete." << endl;
					return;
				}
				if (res != "\'")
				{
					m_iOp = 99;
					cout << "error! The 'insert' command has some syntax error.You may miss the \'" << endl;
					return;
				}
			}
			//如果不是char类型
			else
			{
				if (isInt(res)&&catalogManager->getAttrType(m_strTablename, num-1) == 0)
				{
					res = intTo16(res);
				}
				else if (res.length() < 8 && catalogManager->getAttrType(m_strTablename, num - 1) == 1)
				{
					while (res.length() < 8)
						res = "0" + res;
				}
				tempval->value = res;
				tempval->type = 0;
			}
			lastval->next = tempval;
			lastval = tempval;
			if (!getWord(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'insert' command isn't complete." << endl;
				return;
			}
		}
		if (res != ")")
		{
			m_iOp = 99;
			cout << "error! The 'insert' command has some syntax error.You may miss the ')'. " << endl;
			return;
		}
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The 'insert' command isn't complete.You may miss the \';\'" << endl;
			return;
		}
		if (res != ";")
		{
			m_iOp = 99;
			cout << "error! The 'insert' command isn't complete.You may miss the \';\'" << endl;
			return;
		}
	}

	else if (res == "delete")
	{
		//initAll();
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The 'delete' command isn't complete." << endl;
			return;
		}
		if (res != "from")
		{
			m_iOp = 99;
			cout << "error! The 'delete' command has some syntax error. Check the 'from'. " << endl;
			return;
		}
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The 'delete' command isn't complete." << endl;
			return;
		}
		m_strTablename = res;//存储表名
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The 'delete' command isn't complete." << endl;
			return;
		}
		if (res != ";"&&res != "where")
		{
			m_iOp = 99;
			cout << "error! The 'delete' command has some syntax error. Check the 'where' or ';'. " << endl;
			return;
		}
		//没有where条件
		else if (res == ";")
		{
			m_iOp = 8;
			return;
		}
		//有where条件
		else
		{
			m_iOp = 9;
			if (!getWord(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'delete' command isn't complete." << endl;
				return;
			}
			//存储条件
			tempcon = new condition;
			initCond(tempcon);
			tempcon->attrName = res;//属性名
			if (!getWord(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'delete' command isn't complete." << endl;
				return;
			}
			//如果是非法操作符
			if (res != "<"&&res != ">"&&res != "<="&&res != "<>"&&res != ">="&&res != "=")
			{
				m_iOp = 99;
				cout << "error! Check the 'where' condition. You may use some illegal operations." << endl;
				return;
			}
			else
				tempcon->op = res;
			if (!getWord(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'delete' command isn't complete." << endl;
				return;
			}
			if (res == "\'")//如果是char类型
			{
				tempcon->type = 1;
				if (!getStr(sql, res))
				{
					m_iOp = 99;
					cout << "error! Check the char type on the 'where' condition." << endl;
					return;
				}
				int len = 0;
				Table* tempT = catalogManager->getTable(m_strTablename);
				for (int i = 0; i < tempT->attrNum; i++)
				{
					if (tempcon->attrName == tempT->attributes[i].name)
					{
						len = tempT->attributes[i].length;
						break;
					}
				}
				//cout << "len = " << len << endl;
				while (res.length() < len)
				{
					res += " ";
				}
				tempcon->value = res;
				if (!getWord(sql, res))
				{
					m_iOp = 99;
					cout << "error! The 'delete' command isn't complete." << endl;
					return;
				}
				if (res != "\'")
				{
					m_iOp = 99;
					cout << "error!  Check the char type on the 'where' condition." << endl;
					return;
				}
			}
			else
			{
				if (isInt(res))
				{
					res = intTo16(res);
				}
				else if (res.length() < 8 && isFloat(res))
				{
					while (res.length() < 8)
						res = "0" + res;
				}
				tempcon->value = res;
				tempcon->type = 0;//非char类型
			}
			m_condi = lastcon = tempcon;
			if (!getWord(sql, res))
			{
				m_iOp = 99;
				cout << "error! The 'delete' command isn't complete.You may miss the\"; \" " << endl;
				return;
			}
			//where语句结束
			if (res == ";")
			{
				return;//正确返回
			}
			else if (res == "and" || res == "or")
			{
				if (res == "and")
				{
					m_condi->andor = 1;
				}
				else m_condi->andor = 0;//记录条件
				while (res == "and" || res == "or")
				{
					if (!getWord(sql, res))
					{
						m_iOp = 99;
						cout << "error! The 'delete' command isn't complete.You may miss the\"; \"" << endl;
						return;
					}
					tempcon = new condition;
					initCond(tempcon);
					tempcon->attrName = res;
					if (!getWord(sql, res))
					{
						m_iOp = 99;
						cout << "error! The 'delete' command isn't complete." << endl;
						return;
					}
					//如果是非法操作符
					if (res != "<"&&res != ">"&&res != "<="&&res != "<>"&&res != ">="&&res != "=")
					{
						m_iOp = 99;
						cout << "error! Check the 'where' condition. You may use some illegal operations." << endl;
						return;
					}
					else
						tempcon->op = res;
					if (!getWord(sql, res))
					{
						m_iOp = 99;
						cout << "error! The 'delete' command isn't complete." << endl;
						return;
					}
					if (res == "\'")//如果是char类型
					{
						tempcon->type = 1;
						if (!getStr(sql, res))
						{
							m_iOp = 99;
							cout << "error! Check the char type on the 'where' condition." << endl;
							return;
						}
						int len = 0;
						Table* tempT = catalogManager->getTable(m_strTablename);
						for (int i = 0; i < tempT->attrNum; i++)
						{
							if (tempcon->attrName == tempT->attributes[i].name)
							{
								len = tempT->attributes[i].length;
								break;
							}
						}
						while (res.length() < len)
						{
							res += " ";
						}
						tempcon->value = res;
						if (!getWord(sql, res))
						{
							m_iOp = 99;
							cout << "error! The 'delete' command isn't complete." << endl;
							return;
						}
						if (res != "\'")
						{
							m_iOp = 99;
							cout << "error! Check the char type on 'where' condition." << endl;
							return;
						}
					}
					else
					{
						if (isInt(res))
						{
							res = intTo16(res);
						}
						else if (res.length() < 8 && isFloat(res))
						{
							while (res.length() < 8)
								res = "0" + res;
						}
						tempcon->value = res;
						tempcon->type = 0;//非char类型
					}
					lastcon->next = tempcon;
					lastcon = tempcon;
					if (!getWord(sql, res))
					{
						m_iOp = 99;
						cout << "error! The 'delete' command isn't complete.You may miss the\"; \"" << endl;
						return;
					}
				}
			}
			if (res == ";")
			{
				return;//正确返回
			}
			else
			{
				m_iOp = 99;
				cout << "error! The 'delete' command isn't complete.You may miss the\";\"" << endl;
				return;
			}
		}
	}
	else if (res == "execfile")
	{
		//initAll();
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The command isn't complete." << endl;
			return;
		}
		m_strFilename = res;
		m_iOp = 10;
		if (!getWord(sql, res))
		{
			m_iOp = 99;
			cout << "error! The command isn't complete.You may miss the ;" << endl;
			return;
		}
		if (res != ";")
		{
			m_iOp = 99;
			cout << "error! The command has some syntax error." << endl;
			return;
		}
	}
	else
	{
		m_iOp = 99;
		cout << "error! Check your command." << endl;
	}
}



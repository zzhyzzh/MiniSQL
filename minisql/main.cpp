#include<iostream>
#include<string> 
#include"Interpreter.h"
#include"IndexManager.h"
#include"RecordManager.h"
#include"BufferManager.h"
#include"CatalogManager.h"
#include"API.h"
using namespace std;

Interpret globalI;
CatalogManager *catalogManager;
IndexManager indexManager;
BufferManager bufferManager;
RecordManager recordManager;
API a;

int main()
{
	catalogManager = new CatalogManager;
	string sql = "", tempS = "";
	cout << "        __   __   _           _      _____    ___     _       " << endl;
	cout << "       / /| / /| |_|         |_|    / ___/   / _ \\   | |      " << endl;
	cout << "      / / |/ / |  _   __  _   _    / /___   / / \\ \\  | |      " << endl;
	cout << "     / /| | /| | | | |\\ \\| | | |   \\____ \\ / /  / /  | |      " << endl;
	cout << "    / / | |/ | | | | | \\ \\ | | |   ____/ / \\ \\_/  \\  | |___   " << endl;
	cout << "   /_/  |_|  |_| |_| |_|\\_\\| |_|  /_____/   \\___/\\_\\ |_____|  " << endl;


	cout << "************************************************************" << endl;
	cout << "*                    Welcome to MiniSQL!                   *" << endl;
	cout << "*                        Authors:                          *" << endl;
	cout << "*                        Liu Siyuan                        *" << endl;
	cout << "*                        Zhao Zihan                        *" << endl;
	cout << "*                        Zhou Shuyue                       *" << endl;
	cout << "************************************************************" << endl;

	while (1)
	{
		cout << "MiniSQL-->";
		getline(cin, sql);

		if (sql.length() == 0)//如果直接回车，进行下一条命令
		{
			continue;
		}
		while (sql[sql.length() - 1] != ';')
		{
			getline(cin, tempS);
			sql += "\n" + tempS;
		}
		globalI.handle(sql);
		//如果是退出
		if (globalI.m_iOp == 0)
		{
			a.execSQL(globalI);
			break;
		}
		//如果是执行文件
		else if (globalI.m_iOp == 10)
		{
			a.execSQL(globalI);
		}
		else
		{
			a.execSQL(globalI);
		}
	}
	delete catalogManager;
	system("pause");
	return 0;

}

#include"BPlusTree.h"
#include<iostream>
using namespace std;
extern BufferManager bufferManager;
extern CatalogManager *catalogManager;
BPlusTree::BPlusTree(Index indexInfo)
{

	order  = (BLOCK_SIZE - 12) / (indexInfo.length + 8) + 1;  //����B+���Ķ�
	myIndexInfo = indexInfo;
	leaf_least = ceil((order-1) / 2.0);
	nonleaf_least = ceil((order)/ 2.0 ) - 1;
	//cout << nonleaf_least << endl;
	//cout << order << endl;
	//myIndexInfo = indexInfo;
}

void BPlusTree::insert(string key, int blockOff, int offset) //key ע����չ
{
	string fileName = "file/index/" + myIndexInfo.tableName + "_" + myIndexInfo.name + ".txt";
	Block *root = new Block;
	int blockNum;
	root = bufferManager.readBlock(fileName,0); //��ȡfileName�е�һ��block����root��block��
	//cout << root->CBlock + 4 << endl;
	if (root->charNum == 0) //���ڵ�Ϊ��
	{
		string temp = "!0001";
		temp += "fff";
		temp += toLength(intToStr(blockOff),4);
		temp += toLength(intToStr(offset),4);
		temp += toLength(key,myIndexInfo.length);
		write(root, temp); //��ֵ��CBlock
		return;
	}
	blockNum = searchLeaf(key);
	//cout << blockNum << endl;
	Block *node = new Block;
	node = bufferManager.readBlock(fileName, blockNum); //��ȡҶ�ڵ�
	string info = node->CBlock;
	if (getValueNum(info.substr(1, 4)) < order - 1)
		insertInLeaf(node, key, blockOff, offset);
	else
	{
		Block *L1 = new Block;
		L1 = bufferManager.getBlankBlock(fileName); //��ȡһ���տ�
	//	Block *L2 = new Block;
	//	L2 = bufferManager.getBlankBlock(fileName); //��ȡһ���տ飬L2��ʱ�洢
	//	strcpy(L2->CBlock, info.c_str());

		string pn;

		if (info.substr(5,3) == "fff")
		{
			pn = "";
		}
		else
		{
			pn = info.substr(node->charNum - 4, 4);
		}
		insertInLeaf(node, key, blockOff, offset);

		string tmpT = node->CBlock;
		int halfn = ceil((getValueNum(info.substr(1, 4)) + 1) / 2.0);
		string tmpL;
		tmpL = "!" + toLength(intToStr(halfn),4);
		tmpL += tmpT.substr(5, 3 + halfn*(8+myIndexInfo.length));
		tmpL += toLength(intToStr(L1->blockNum), 4);
		write(node, tmpL);   //����ǰ����ڵ�

		int n = getValueNum(tmpT.substr(1, 4));  //value����

		string tmpL1;
		tmpL1 = "!" + toLength(intToStr(n-halfn), 4) + tmpT.substr(5,3); //���游�ڵ㣬�˴���δ��ʼ��
		tmpL1+= tmpT.substr(8 + halfn*(8 + myIndexInfo.length), (n - halfn)*(8 + myIndexInfo.length));
		tmpL1 += pn;
		write(L1, tmpL1);

		string K1 = tmpL1.substr(16, myIndexInfo.length);

		catalogManager->addIndexBlockNum(myIndexInfo.value); //blockNum����1
		insertInParent(blockNum, L1->blockNum,K1);
		
		
	}
}

void BPlusTree::insertInParent(int blockNum, int blockNum1,string key)
{
	int i;
	string info1, info2;
	string fileName = "file/index/" + myIndexInfo.tableName + "_" + myIndexInfo.name + ".txt";
	Block *root = new Block;
	root = bufferManager.readBlock(fileName, 0); //��ȡ����
	if (root->blockNum == blockNum)  //����֮ǰ�Ľڵ�͸��ڵ�
	{
		Block *R = new Block;
		R = bufferManager.getBlankBlock(fileName);
		catalogManager->addIndexBlockNum(myIndexInfo.value); //blockNum����1
		int r = R->blockNum;
		Block *node1 = new Block;
		Block *node2 = new Block;
		node1 = bufferManager.readBlock(fileName,blockNum);
		node2 = bufferManager.readBlock(fileName,blockNum1);
		string info = "?0001fff" + toLength(intToStr(r),4) + key + toLength(intToStr(blockNum1), 4); //���ڵ������Ҫ���£�ע��
		R->blockNum = node1->blockNum;
		node1->blockNum = r;   //����block

		info1 = node1->CBlock;
		info2 = node2->CBlock;
		info1.replace(5, 3, toLength(intToStr(R->blockNum), 3)); //���¸��ڵ��ַ��������飬����
		info2.replace(5, 3, toLength(intToStr(R->blockNum), 3));

		write(node1, info1);
		write(node2, info2);
		write(R, info);
		fatherUpdate(node1->blockNum);
		return;
	}
	else
	{
		Block *N0 = new Block;
		string nodeInfo;
		N0 = bufferManager.readBlock(fileName, blockNum);

		nodeInfo = N0->CBlock;
		int p = getValueNum(nodeInfo.substr(5,3)); //��ȡ���ڵ�

		Block *PNode = new Block;
		PNode = bufferManager.readBlock(fileName, p);
		string pInfo = PNode->CBlock;
		
		if (getValueNum(pInfo.substr(1, 4)) < order - 1)  //���ڵ���Բ���
		{
			int start;
			string temp = PNode->CBlock;
			int num = getValueNum(temp.substr(1, 4));
			temp.replace(1, 4, toLength(intToStr(num+1), 4));
			for (i = 0; i <= num; i++) // =��������
			{
				start = 8 + (4 + myIndexInfo.length)*i;

				if (getValueNum(temp.substr(start, 4)) == blockNum)
				{
					string insert = key + toLength(intToStr(blockNum1), 4);
					temp.insert(start + 4, insert);
					write(PNode, temp);
					break;  //ע�⣬���ڵ�δ����
				}
			}

			Block *node1 = new Block;
			Block *node2 = new Block;
			node1 = bufferManager.readBlock(fileName,blockNum);
			node2 = bufferManager.readBlock(fileName,blockNum1);
			info1 = node1->CBlock;
			info2 = node2->CBlock;
			info1.replace(5, 3, toLength(intToStr(PNode->blockNum), 3)); //���¸��ڵ��ַ��������飬����
			info2.replace(5, 3, toLength(intToStr(PNode->blockNum), 3));

			write(node1, info1);
			write(node2, info2);

			return;
		}
		else  //���ڵ�����
		{
			//Block *newBlock = new Block;
			//newBlock = bufferManager.getBlankBlock(fileName);  //����µĿ�
			//strcpy(newBlock->CBlock, pInfo.c_str());
			//string tmpT = newBlock->CBlock;
			string tmpT = pInfo;
			int num = getValueNum(tmpT.substr(1, 4));
			int start, end;

			tmpT.replace(1, 4, toLength(intToStr(num+1), 4));
			for (int i = 0; i<=num; i++) {  //�Ⱥ�������
				start = (4 + myIndexInfo.length)*i + 8;

				string insert = key + toLength(intToStr(blockNum1), 4);
				/*�ҵ���ƥ��Ŀ��*/
				if (getValueNum(tmpT.substr(start, 4)) == blockNum)
				{
					
					tmpT.insert(start + 4, insert);
					break;
				}
			}

			PNode->CBlock = NULL;

			Block *P1 = new Block;
			P1 = bufferManager.getBlankBlock(fileName); //��ȡ�տ�
			catalogManager->addIndexBlockNum(myIndexInfo.value); //blockNum����1
			int pnum = P1->blockNum;
			/*copy 1 to n/2 from T to P*/
			int halfn = ceil((order - 1) / 2.0);
			string tempP = "?" + toLength(intToStr(halfn), 4);
			tempP += tmpT.substr(5, 7 + halfn*(4 + myIndexInfo.length)); //���ڵ�δ����
			write(PNode, tempP);
			/*let K11=T.K n/2*/
			fatherUpdate(PNode->blockNum); //���¸��ڵ�

			string K11 = tmpT.substr(12 + halfn*(4 + myIndexInfo.length), myIndexInfo.length);
			
			/*copy n/2 +1 to n from T to P1*/
			string tempP1 = "?" + toLength(intToStr(order-halfn-1), 4) + tmpT.substr(5,3);

			tempP1 += tmpT.substr(8 + (4+myIndexInfo.length)*(halfn + 1), (4 + myIndexInfo.length)*(order - halfn - 1)+4);
			//cout << tempP1 << endl;
			write(P1, tempP1);

			fatherUpdate(P1->blockNum); //���¸��ڵ�

			insertInParent(PNode->blockNum, P1->blockNum, K11);
			
			//bufferManager.deleteBlock(fileName, P1->blockNum);
			return;


		}
	}

}

string BPlusTree::intToStr(int value)  //���⣬int ת string
{
	string temp;
	char ch;
	int i;
	while(value)
	{
		ch = value%10 + '0';
		temp = ch + temp;
		value = value / 10;
	}
	return temp;
}

string BPlusTree::toLength(string str,int length) //���� λ��չ
{
	int i;
	while (str.length() < length)
	{
		str = "0" + str;
	}
	return str;
}

void BPlusTree::write(Block *b, string s) {
	b->CBlock = new char[s.size() + 1];
	strcpy(b->CBlock, s.c_str());
	b->charNum = strlen(b->CBlock);
	b->isDirty = 1;
}



void BPlusTree::writeRootBlock(Block b)
{
	string fileName = "file/index/" + myIndexInfo.tableName + "_" + myIndexInfo.name + ".txt";
	Block root;
	//root = readBlock(fileName, 0);

}
void BPlusTree::insertInLeaf(Block* node, string key, int blockNum, int offset) //��Ҷ�ڵ���룬���д���ȶ
{
	int i;
	int start, end;
	int length = myIndexInfo.length;
	string bnum, off,tmp;
	string info = node->CBlock;
	tmp = info;
	int num = getValueNum(info.substr(1, 4));
	bnum = toLength(intToStr(blockNum), 4);
	off = toLength(intToStr(offset), 4);
	string insert = bnum + off + toLength(key, myIndexInfo.length);
	tmp.replace(1, 4, toLength(intToStr(getValueNum(info.substr(1, 4)) + 1),4)); //value����+1
	for (i = 0; i < num; i++)
	{
		start = 8 + i*(8 + length);
		if (key.compare(tmp.substr(start + 8, length)) < 0)
		{
			tmp.insert(start,insert);
			write(node, tmp);
			return;
		}
	}
	start = 8 + i*(8 + length);
	tmp.insert(start, insert);  //�д���ȶ��ָ����һ���ָ��λ�ò�ȷ��
	write(node, tmp);
	return;
}


int BPlusTree::searchLeaf(string key)  //Ѱ����Ҷ�ڵ�
{
	string fileName = "file/index/" + myIndexInfo.tableName + "_" + myIndexInfo.name + ".txt";
	Block *root = new Block;
	Block *node = new Block;
	root = bufferManager.readBlock(fileName,0); //��ȡfileName�е�һ��block����root��block��
	node = bufferManager.readBlock(fileName,root->blockNum);
	string info = node->CBlock;
	int num = 0;//ÿ���ڵ������ֵ����
	int start,i;
	string nodeValue;
	int length = myIndexInfo.length;
	
	while (node->CBlock[0] != '!')  //�ҵ�Ҷ�ڵ�
	{
		num = getValueNum(info.substr(1, 4));
		int nextBlock = 0;
		info = node->CBlock;
		int end;
		
		for (i = 0; i < num; i++)
		{
			start = 8 + i*(4 + length);
			end = start + length + 3;
			nodeValue = info.substr(start + 4, length);
			if (key.compare(nodeValue) >= 0)             //�˴��Ƚ�ֱ�����ַ����Ƚ�
			{
				if (i + 1 == num)
				{
					nextBlock = getValueNum(info.substr(end + 1, 4));
					break;
				}
				else
					continue;
			}
			else
			{
				nextBlock = getValueNum(info.substr(start, 4));
				break;
			}
		}
		node = bufferManager.readBlock(fileName, nextBlock); //Ѱ����һ�����
		info = node->CBlock;
	}
	info = node->CBlock;
	if (info[0] == '!')
		return node->blockNum;
	else
		return -1;
}


Offset BPlusTree::searchEqual(string key)
{
	string fileName = "file/index/" + myIndexInfo.tableName + "_" + myIndexInfo.name + ".txt";
	int bnum,i,start;
	Offset off;
	off.blockNum = -1;
	off.offset = -1;
	bnum = searchLeaf(key);
	Block *leaf = new Block;
	leaf = bufferManager.readBlock(fileName, bnum);
	string info = leaf->CBlock;
	int num = getValueNum(info.substr(1, 4));
	for (i = 0; i < num; i++)
	{
		start = 8 + i*(8 + myIndexInfo.length);
		if (key == info.substr(start + 8, myIndexInfo.length))
		{
			off.blockNum = getValueNum(info.substr(start, 4));
			off.offset = getValueNum(info.substr(start + 4, 4));
			return off;
		}
	}
	return off;
}


int BPlusTree::getValueNum(string snum) {//���ڵ�valueֵ�ĸ���ת����int
	int num = 0;
	for (int i = 0; i < snum.length(); i++)
		num = 10 * num + snum[i] - '0';
	return num;
}

void BPlusTree::fatherUpdate(int blockNum)
{
	int i;
	string fileName = "file/index/" + myIndexInfo.tableName + "_" + myIndexInfo.name + ".txt";
	Block *node = new Block;
	Block *childBlock = new Block;
	node = bufferManager.readBlock(fileName, blockNum);
	string info = node->CBlock;
	if (info[0] == '!')
		return;
	string cinfo;
	int num = getValueNum(info.substr(1, 4));
	int start,length,bnum;

	length = 4 + myIndexInfo.length;
	for (i = 0; i <=num; i++)
	{
		start = 8 + i*length;
		bnum = getValueNum(info.substr(start, 4));
		childBlock = bufferManager.readBlock(fileName, bnum);
		cinfo = childBlock->CBlock;
		cinfo.replace(5, 3, toLength(intToStr(blockNum),3));
		write(childBlock, cinfo);
	}
	
	
}

void  BPlusTree::deleteOne(string key)
{
	int bnum = searchLeaf(key);
	deleteEntry(bnum, key, bnum);
}

void BPlusTree::deleteEntry(int blockNum, string key, int deletedNum)
{
	string fileName = "file/index/" + myIndexInfo.tableName + "_" + myIndexInfo.name + ".txt";
	Block *node = new Block;
	node = bufferManager.readBlock(fileName, blockNum);
	string info = node->CBlock;
	string tempN;
	tempN = info;
	int j, startF;
	int num = getValueNum(info.substr(1, 4));
	tempN.replace(1, 4, toLength(intToStr(num-1),4)); //ɾ�����value
	int length,i,start;
	if (info[0] == '?')
		length = 4;
	else if (info[0] == '!')
		length = 8;

	for (i = 0; i < num; i++)
	{
		start = 8 + i*(length + myIndexInfo.length);
		if (info.substr(start + length, myIndexInfo.length) == key)
		{
			if (info[0] == '!')
			{
				if (i == 0 && info.substr(5, 3) != "fff") //���ɾ���˵�һ���ڵ㣬Ҫ����·��
				{
					int fatherTmpNum = getValueNum(tempN.substr(5, 3));
					
					Block *fatherTmp = new Block;
					fatherTmp = bufferManager.readBlock(fileName, fatherTmpNum);
					string fatherin = fatherTmp->CBlock;
					for (j = 0; j <= getValueNum(fatherin.substr(1, 4)); j++)
					{
						startF = 8 + j*(4 + myIndexInfo.length);
						if (fatherin.substr(startF + 4, myIndexInfo.length) == tempN.substr(start + 8, myIndexInfo.length))  //�����飬��Ϊ3ʱ
						{
							
							fatherin.replace(startF + 4, myIndexInfo.length, tempN.substr(start + myIndexInfo.length + 16, myIndexInfo.length));
							write(fatherTmp, fatherin);
							break;
						}
					}
				}
				tempN.replace(start, length + myIndexInfo.length, "");  //�����飬����������ɾ��ͷ�ڵ㣬����·��
			}
			else if (info[0] == '?')
			{
				if (getValueNum(info.substr(start, length)) == deletedNum)
				{
					tempN.replace(start,length+myIndexInfo.length,"");
				}
				else if (getValueNum(info.substr(start + length + myIndexInfo.length, length)) == deletedNum)
				{
					tempN.replace(start + length, length + myIndexInfo.length, "");
				}
			}
			break;
			
		}
	}
	//cout << tempN << endl;
	write(node, tempN);
	int father = getValueNum(info.substr(5,3));

	if (node->blockNum == 0 && info.substr(1, 4) == "0001") {
		/*make the child of N the root*/
		int childNum = getValueNum(tempN.substr(8, 4));
		Block *child = new Block;
		child = bufferManager.readBlock(fileName, childNum);
		string childInfo = child->CBlock;
		childInfo.replace(5,3,"fff");
		/*�������*/
		node->blockNum = child->blockNum;
		child->blockNum = 0;
		write(child, childInfo);
		fatherUpdate(child->blockNum);
		//writeRootBlock(dbName, name, Child);
		/*delete N*/
		bufferManager.deleteBlock(fileName, node->blockNum);
		catalogManager->subIndexBlockNum(myIndexInfo.value);
	}
	else
	{
		info = node->CBlock;
		tempN = node->CBlock;

		if (info[0] == '!' && info.substr(5,3)!="fff")
		{
			if (getValueNum(info.substr(1, 4)) < leaf_least) //���Ҷ��value����С���½�,��ϲ�
			{
				string sibInfo;
				int begin,sibling;
				int father = getValueNum(info.substr(5, 3));
				Block *fatherNode = new Block;
				Block *sibNode = new Block;
				fatherNode = bufferManager.readBlock(fileName, father);
				string fatherInfo = fatherNode->CBlock;
				int valueNum = getValueNum(fatherInfo.substr(1, 4));
				for (i = 0; i <= valueNum; i++)
				{
					begin = 8 + i*(4 + myIndexInfo.length);
					if (getValueNum(fatherInfo.substr(begin, 4)) == blockNum)
					{
						sibling = begin;
						break;
					}
				}
				if (sibling == 8)  //����ǵ�һ���ӽڵ�
				{
					
					sibling += 4 + myIndexInfo.length;
					sibling = getValueNum(fatherInfo.substr(sibling, 4));
					sibNode = bufferManager.readBlock(fileName, sibling);
					sibInfo = sibNode->CBlock;
					string firstKey = sibInfo.substr(16, myIndexInfo.length);
					if (getValueNum(sibInfo.substr(1, 4)) + getValueNum(info.substr(1, 4)) <= order - 1) //2���ڵ���Ժϲ�
					{
						int totalNum = getValueNum(sibInfo.substr(1, 4)) + getValueNum(info.substr(1, 4));
						sibInfo.replace(1, 4, toLength(intToStr(totalNum), 4));
						tempN.replace(tempN.length() - 4, 4, "");
						string part1 = tempN.substr(8, tempN.length() - 8);
						sibInfo.insert(8, part1);
						write(sibNode, sibInfo);
						deleteEntry(father, firstKey, blockNum);  //�ݹ�ɾ�����ڵ��Ԫ��
						bufferManager.deleteBlock(fileName,node->blockNum);
						catalogManager->subIndexBlockNum(myIndexInfo.value);
					}
					else //2���ڵ��޷��ϲ�����һ���ֵܽڵ��н�һ���ڵ�
					{
						int i, start;
						int nodeNum = getValueNum(tempN.substr(1, 4));
						int sibValueNum = getValueNum(sibInfo.substr(1, 4));
						string first = sibInfo.substr(8, 8 + myIndexInfo.length);
						sibInfo.replace(1, 4, toLength(intToStr(sibValueNum - 1), 4));
						sibInfo.replace(8, myIndexInfo.length + 8, "");
						write(sibNode, sibInfo);

						tempN.replace(1, 4, toLength(intToStr(nodeNum + 1), 4));
						tempN.insert(tempN.length()-4, first);
						write(node, tempN);

						string km = sibInfo.substr(16, myIndexInfo.length);
						string tmp = fatherInfo;
						int fatherNum = getValueNum(tmp.substr(1, 4));
						for (i = 0; i < fatherNum; i++)
						{
							start = 8 + i*(4 + myIndexInfo.length);
							if (tmp.substr(start + 4, myIndexInfo.length) == firstKey)
							{
								tmp.replace(start + 4, myIndexInfo.length, km);
								write(fatherNode, tmp);
							}
						}
					}
				}
				else
				{
					sibling -= (4 + myIndexInfo.length);
					sibling = getValueNum(fatherInfo.substr(sibling, 4));
					sibNode = bufferManager.readBlock(fileName, sibling);
					
					sibInfo = sibNode->CBlock;
					//cout << sibInfo << endl;
					string firstKey = info.substr(16, myIndexInfo.length);
					//cout << firstKey << endl;
					if (getValueNum(sibInfo.substr(1, 4)) + getValueNum(info.substr(1, 4)) <= order - 1) //2���ڵ���Ժϲ�
					{
						int totalNum = getValueNum(sibInfo.substr(1, 4)) + getValueNum(info.substr(1, 4));
						sibInfo.replace(1, 4, toLength(intToStr(totalNum),4));
						sibInfo.replace(sibInfo.length() - 4, 4, "");
						string tail = info.substr(8, info.length() - 8);
						sibInfo += tail;
						//cout << sibInfo << endl;
						write(sibNode, sibInfo);
						deleteEntry(father, info.substr(16, myIndexInfo.length),node->blockNum);  //�ݹ�ɾ�����ڵ��Ԫ��
						bufferManager.deleteBlock(fileName,node->blockNum);
						catalogManager->subIndexBlockNum(myIndexInfo.value);
					}
					else //2���ڵ��޷��ϲ�����һ���ֵܽڵ��н�һ���ڵ�
					{
						int i,start;
						int nodeNum = getValueNum(tempN.substr(1, 4));
						int sibValueNum = getValueNum(sibInfo.substr(1, 4));
						string last = sibInfo.substr(8+(sibValueNum-1)*(8 + myIndexInfo.length),myIndexInfo.length + 8);
						sibInfo.replace(1, 4, toLength(intToStr(sibValueNum - 1), 4));
						sibInfo.replace(8 + (sibValueNum - 1)*(8 + myIndexInfo.length), myIndexInfo.length + 8, "");
						write(sibNode, sibInfo);

						tempN.replace(1, 4, toLength(intToStr(nodeNum + 1), 4));
						tempN.insert(8, last);
						write(node, tempN);

						string km = last.substr(8, myIndexInfo.length);
						string tmp = fatherInfo;
						int fatherNum = getValueNum(tmp.substr(1, 4));
						for (i = 0; i < fatherNum; i++)
						{
							start = 8 + i*(4 + myIndexInfo.length);
							if (tmp.substr(start + 4, myIndexInfo.length) == firstKey)
							{
								tmp.replace(start + 4, myIndexInfo.length, km);
								write(fatherNode, tmp);
							}
						}
					}

				}
			}
		}
		else if (info[0] == '?')
		{
			if (getValueNum(info.substr(1, 4)) < nonleaf_least)
			{
				//cout << "reach" << endl;
				int father = getValueNum(info.substr(5, 3));
				Block *fatherNode = new Block;
				Block *interSib = new Block;
				fatherNode = bufferManager.readBlock(fileName, father);
				
				string fatherInfo = fatherNode->CBlock;
				string keyFlag;
				num = getValueNum(fatherInfo.substr(1, 4));
				int pre = 1;
				int siblingNum;
				for (i = 0; i <= num; i++) //������=��
				{
					start = 8 + i*(myIndexInfo.length + 4);
					if (getValueNum(fatherInfo.substr(start, 4)) == node->blockNum)
					{
						if (i == 0)
						{
						
							siblingNum = getValueNum(fatherInfo.substr(start + myIndexInfo.length + 4, 4));
							keyFlag = fatherInfo.substr(start + 4, myIndexInfo.length);
							pre = 0;
						}
						else
						{
							siblingNum = getValueNum(fatherInfo.substr(start - myIndexInfo.length - 4, 4));
							keyFlag = fatherInfo.substr(start - myIndexInfo.length, myIndexInfo.length);
						}
						break;
					}
				}
				
				interSib = bufferManager.readBlock(fileName, siblingNum); //��ȡ���ڽڵ�
				string sibInfo = interSib->CBlock;
				if (pre == 1) //�����ȡ����ǰһ���ֵܽڵ�
				{
					if (getValueNum(tempN.substr(1, 4)) + getValueNum(sibInfo.substr(1, 4) ) + 1 <= order - 1) {  //������Ժϲ�,�˴�+1������
						/*append K1 and all in N to N1 */
						int totalNum = getValueNum(sibInfo.substr(1, 4)) + getValueNum(tempN.substr(1, 4)) + 1;
						sibInfo.replace(1, 4, toLength(intToStr(totalNum), 4));
						string tail = tempN.substr(8, node->charNum - 8);

						sibInfo = sibInfo + keyFlag + tail;
						write(interSib, sibInfo);
						fatherUpdate(interSib->blockNum); //���¸��ڵ�
						deleteEntry(father, keyFlag, node->blockNum);
						bufferManager.deleteBlock(fileName, node->blockNum);
						catalogManager->subIndexBlockNum(myIndexInfo.value);
					}
					else   //�޷��ϲ������ֵܽ�ڵ�
					{
						string tail = sibInfo.substr(interSib->charNum - 4, 4);
						string tailKey = sibInfo.substr(interSib->charNum - 4 - myIndexInfo.length, myIndexInfo.length);
						/*remove N1.Km-1,N1.pm from N1*/
						sibInfo.replace(1, 4, toLength(intToStr(getValueNum(sibInfo.substr(1, 4)) - 1),4));
						sibInfo.replace(interSib->charNum - 4 - myIndexInfo.length, myIndexInfo.length + 4, "");
						write(interSib, sibInfo);

						/*insert N1.pm,K1 as the first in N*/
						tempN.replace(1, 4, toLength(intToStr(getValueNum(tempN.substr(1, 4)) + 1), 4));
						tempN.insert(8, tail + keyFlag);
						write(node, tempN);
						/*replace K1 in parent(N) by N1.Km-1*/
						fatherInfo.replace(start - myIndexInfo.length, myIndexInfo.length, tailKey);
						write(fatherNode, fatherInfo);

						int childNum = getValueNum(tail);
						Block *child = new Block;
						child = bufferManager.readBlock(fileName, childNum);
						string chInfo = child->CBlock;
						chInfo.replace(5, 3, toLength(intToStr(node->blockNum), 4));
						write(child, chInfo);
					}
				}
				else if (pre == 0)
				{
					if (getValueNum(tempN.substr(1, 4)) + getValueNum(sibInfo.substr(1, 4)) + 1 <=order - 1) {  //������Ժϲ�
																											   /*append K1 and all in N to N1 */
						
						int totalNum = getValueNum(sibInfo.substr(1, 4)) + getValueNum(tempN.substr(1, 4)) + 1;
						sibInfo.replace(1, 4, toLength(intToStr(totalNum), 4));
						string head = tempN.substr(8, node->charNum - 8);
						sibInfo.insert(8, head + keyFlag);
						write(interSib, sibInfo);
						fatherUpdate(interSib->blockNum); //���¸��ڵ�
						deleteEntry(father, keyFlag, node->blockNum);
						bufferManager.deleteBlock(fileName, node->blockNum);
						catalogManager->subIndexBlockNum(myIndexInfo.value);
					}
					else   //�޷��ϲ������ֵܽ�ڵ�
					{
						//cout << "reach" << endl;
						string head = sibInfo.substr(8, 4);
						string headKey = sibInfo.substr(12, myIndexInfo.length);
						/*remove N1.Km-1,N1.pm from N1*/
						sibInfo.replace(1, 4, toLength(intToStr(getValueNum(sibInfo.substr(1, 4)) - 1), 4));
						sibInfo.replace(8, myIndexInfo.length + 4, "");
						//cout << sibInfo << endl;
						write(interSib, sibInfo);

						/*insert N1.pm,K1 as the first in N*/
						tempN.replace(1, 4, toLength(intToStr(getValueNum(tempN.substr(1, 4)) + 1), 4));
						tempN = tempN + keyFlag + head;
						write(node, tempN);
						/*replace K1 in parent(N) by N1.Km-1*/
						fatherInfo.replace(start + 4, myIndexInfo.length, headKey);
						write(fatherNode, fatherInfo);

						int childNum = getValueNum(head);
						Block *child = new Block;
						child = bufferManager.readBlock(fileName, childNum);
						string chInfo = child->CBlock;
						chInfo.replace(5, 3, toLength(intToStr(node->blockNum), 4));
						write(child, chInfo);
					}
				}
			}
		}
	}
}

void BPlusTree::updateKey(string key, int blockOffset, int offset)
{
	string boff, off;
	string fileName = "file/index/" + myIndexInfo.tableName + "_" + myIndexInfo.name + ".txt";
	int bnum = searchLeaf(key);
	Block *newBlock = bufferManager.readBlock(fileName, bnum);
	string info = newBlock->CBlock;
	int i,start,num;
	num = getValueNum(info.substr(1, 4));
	boff = toLength(intToStr(blockOffset), 4);
	off = toLength(intToStr(offset), 4);
	for (i = 0; i < num; i++)
	{
		start = 8 + i*(8 + myIndexInfo.length);
		if (key == info.substr(start + 8, myIndexInfo.length))
		{
			info.replace(start, 8, boff + off);
			write(newBlock, info);
			break;
		}
	}
}

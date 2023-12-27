#include <fstream>

#include <iostream>

#include <cstring>

#include<string>

#include <ctime>

using namespace std;

const char FileName[] = "os.txt"; //创建的大文件名称

const char NullName[] = "00000000000000";

const int DirLen = 1;

const short SSNum = -1; //super block num

//i节点结构信息

struct INode
{

	char fileSpec; //节点文件类型，0表示NORMAL（正规文件） 1表示DIR（目录文件） 3表示Stock（块设备） 4表示（管道文件）（0-4四种文件类型）

	short iaddr[13]; //索引表0-13，每个2byte，共26byte

	int fileLength; //文件长度4byte

	char linkCount;  //联结计数 1byte
};//共32B

//目录项信息
struct DirChild
{
	char filename[14]; //文件名14byte

	short i_BNum; //i节点号2byte
};//共16B

//目录节点信息
struct DirNode
{
	DirChild childItem[64]; //目录下存放的文件，这里存的是目录项

	short DirCount; //存放的文件数量
};

short SS[51]; //超级栈,指针为SS[0]，超级块

short freeBlockNum = 0; //当前可用盘块所在组记录的盘块号

short freeTotalB = 20450; //当前剩余可用盘块数，20480-30个可分配结构块

short rootDirNum; //当前目录所在的磁盘块号

short currentDirIndex = 0; //当前所在目录所在目录节点（rootDir）中的下标

DirNode rootDir[10]; //定义目录节点

int DirFloorArr[10]; //存放进入的目录节点下标
int DirFloor = 0;  //存放当前DirFloorArr的下标
char DirName[10][14]; //存放进入过的目录名称

INode iNode[640]; //定义i节点

//================================================================

//函数描述：创建20M磁盘


short assAnDirMy()//判断目录中的文件个数，返回目录文件个数小于0的目录号
{
	for (int i = 0; i < 10; i++)
	{
		if (rootDir[i].DirCount == -1)
		{
			rootDir[i].DirCount = 0;
			return i;
		}
	}
	return -1;
}

void Format()//开辟20M文件大小，全部存‘0’
{
	cout << "系统正在初始化......" << endl;

	// 打开文件

	FILE* f = fopen(FileName, "wb+");

	if (f == NULL)
	{
		cout << "程序创建错误，请重新输入" << endl;
		return;
	}
	for (int i = 0; i < 20971520; i++) //20971520=20Mb,暂时全部存为0
		fprintf(f, "%c", '0');

	// 关闭文件
	fclose(f);
}

//================================================================

//函数描述：数组赋值

void ArrarEqual(short arr[51], short begin, short end)//数组赋值eg:begin=1，end=20,0<i<20,arr[50]=1,arr[49]=2,arr[48]=3.....

{

	for (short i = 0; i < end - begin + 1; i++)

		arr[50 - i] = begin + i;
}

//================================================================

//函数描述：重构BWrite，实现一个数组的写入

void BWrite(short arr[51], short diskNum)//将数组从1024*diskNUm位置写入文件中，第diskNum块写入，块从0开始算
{

	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)

	{

		cout << "写文件处错误，请重新输入" << endl;

		return;
	}

	//设置文件指针，将文件指针移动到1024*diskNum处

	if (fseek(f, 1024 * diskNum, 0))

		cout << "文件指针错误" << endl;

	//将数组内容写入文件中
	fwrite(arr, sizeof(short), 51, f);

	fclose(f);
}

void WriteINode()//i节点写入，这里从11个物理块开始写入，将iNode数组中的前640个i节点写入到文件f中，20个i节点块，一个32B*20=640B
{
	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)

	{
		cout << "写文件处错误，请重新输入" << endl;
		return;
	}

	//设置文件指针

	if (fseek(f, 1024 * 11, 0))
		cout << "文件指针错误" << endl;

	fwrite(iNode, sizeof(INode), 640, f);

	fclose(f);
}

void WriteRootDir()//目录文件写入从1开始
{
	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)

	{

		cout << "写文件处错误，请重新输入" << endl;

		return;
	}

	//设置文件指针

	if (fseek(f, 1024 * 1, 0))
		cout << "文件指针错误" << endl;
	for (int i = 0; i < 10; i++) {
		fwrite(rootDir[i].childItem, sizeof(DirChild), 64, f);
	}

	if (fseek(f, sizeof(short)* 53, 0))
		cout << "文件指针错误" << endl;
	for (int i = 0; i < 10; i++) {
		fwrite(&(rootDir[i].DirCount), sizeof(short), 1, f);
	}


	fclose(f);
}
//================================================================

//函数描述：重构BWrite，实现一个数组的写入,数组长度不确定
void BWriteArr(short arr[512], short diskNum)//同上写入
{

	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)

	{
		cout << "写文件处错误，请重新输入" << endl;
		return;
	}

	//设置文件指针

	if (fseek(f, 1024 * diskNum, 0))

		cout << "文件指针错误" << endl;

	fwrite(arr, sizeof(short), 512, f);

	fclose(f);
}

//================================================================

//函数描述：从磁盘中读出数组

void BRead(short arr[51], short diskNum)//同上读出
{

	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)

	{

		cout << "读文件处错误，请重新输入" << endl;

		return;
	}

	//设置文件指针

	if (fseek(f, 1024 * diskNum, 0))

		cout << "文件指针错误" << endl;

	fread(arr, sizeof(short), 51, f);

	fclose(f);
}

//================================================================

//函数描述：从磁盘中读出数组, 放入到iNOde中

void BReadArr(short arr[512], short diskNum)
{

	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)

	{
		cout << "读文件处错误，请重新输入" << endl;

		return;
	}

	//设置文件指针

	if (fseek(f, 1024 * diskNum, 0))

		cout << "文件指针错误" << endl;

	fread(arr, sizeof(short), 512, f);

	fclose(f);
}

//================================================================

//函数描述：写入一个目录项

void BWrite(short diskNum)//将目录项写入文件，先写索引节点号，再写文件名
{

	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)

	{

		cout << "写文件处错误，请重新输入" << endl;

		return;
	}

	//设置文件指针

	if (fseek(f, long(1024 * diskNum), 0))

		cout << "文件指针错误" << endl;

	for (int i = 0; i < 64; i++)

	{

		fprintf(f, "%hd", rootDir->childItem[i].i_BNum);

		fputs(rootDir->childItem[i].filename, f);
	}

	fclose(f);
}

//================================================================

//函数描述：分配一个空闲的普通快，返回分配的块号

short AssAnEmpty()
{

	short temp;

	if (SS[0] > 1)
	{

		SS[0]--;//N--

		temp = SS[SS[0] + 1];//定位索引位置

		//                               SS[SS[0]+1]=-1;

		freeTotalB--; //总剩余数-1

		return temp;
	}
	else
	{

		if (SS[1] == 0)
		{

			cout << "盘片用尽" << endl;

			return -1;
		}

		//temp = freeBlockNum;

		freeBlockNum = SS[1]; //保存当前读入的块号

		temp = freeBlockNum;

		BRead(SS, SS[1]);

		/*if (temp == 0) {
		SS[0]--;
		temp = SS[SS[0] + 1];
		}*/

		freeTotalB--;

		return temp;
	}
}

short AssAnNode()//判断联结计数
{

	for (short i = 0; i < 640; i++)
	{
		if (iNode[i].fileLength == -1)
		{
			return i;
		}
	}
	printf("无节点可分配\n");
	return -1;
}

//================================================================

//函数描述：分配一个空闲的目录块

short AssAnDir(DirNode& dir)
{

	if (dir.DirCount == 64)
	{

		cout << "该目录已满" << endl;

		return -1;
	}

	else
	{
		for (short i = 0; i < 64; i++)
		{
			if (dir.childItem[i].i_BNum == -1)
			{
				dir.DirCount++;
				dir.childItem[i].i_BNum = AssAnNode();
				return i;
			}
		}
	}
}

//================================================================

//函数描述：创建一个文件节点，并分配INOde和磁盘空间

void InitCreate(char fielSpec, short diskNum)
{

	//         int blockNum=AssertAnEmpty();

	if (fielSpec == '1')
	{
		WriteINode();
		WriteRootDir();



	}
}

//================================================================

//函数描述：初始化

void Init(char fielSpec, short diskNum)
{

	InitCreate(fielSpec, diskNum);

	BRead(SS, 0);
}

//================================================================

//函数描述：初始化索引区

void Init()
{
	for (int i = 0; i < 10; i++)
	{
		rootDir[i].DirCount = -1;
		for (int j = 0; j < 64; j++)
		{
			rootDir[i].childItem[j].i_BNum = -1;
			strncpy(rootDir[i].childItem[j].filename, NullName, 14);
		}
	}

	for (int i = 0; i < 640; i++)
	{

		iNode[i].fileSpec = 2;

		iNode[i].linkCount = -1;

		iNode[i].fileLength = -1;

		for (short i = 0; i < 13; i++)
			iNode[i].iaddr[i] = -1;
	}
	rootDir[0].DirCount = 0;

	freeBlockNum = 0; //当前可用盘快所在组记录的盘快号

	freeTotalB = 20450;
	BRead(SS, 0);

	//根据文件长度非配文件磁盘节点

	//直接寻址
}

//================================================================

//函数描述：成组链接初始化



void Linkdisk()
{

	//临时空闲栈

	SS[0] = 50;

	ArrarEqual(SS, 31, 80);

	BWrite(SS, 0);

	for (short i = 1; i < 408; i++)
	{

		SS[0] = 50;

		ArrarEqual(SS, i * 50 + 31, i * 50 + 80);

		BWrite(SS, i * 50 + 30);
	}

	ArrarEqual(SS, 408 * 50 + 31, 408 * 50 + 79);

	SS[0] = 49;

	SS[1] = 0; //49

	BWrite(SS, 408 * 50 + 30);

	cout << "磁盘os.txt完成创建,大小20MB" << endl;
}
//================================================================

//函数描述：判断一个文件是一个普通文件


bool IsFile(short diskNum)
{

	if (iNode[diskNum].fileSpec == '0' || iNode[diskNum].fileSpec == '3' || iNode[diskNum].fileSpec == '4')

		return true;

	else

		return false;
}

//================================================================

//函数描述：判断一个文件是一个目录文件



bool IsDir(short diskNum)
{

	if (iNode[diskNum].fileSpec == '1')
		return true;

	else

		return false;
}
//================================================================

//函数描述：判断一个文件是否存在



short IsFileExist(DirNode dir, char fileName[14])
{

	for (int i = 0; i < 64; i++)
	{

		if (strcmp(fileName, dir.childItem[i].filename) == 0 && IsFile(dir.childItem[i].i_BNum))

			return dir.childItem[i].i_BNum;
	}

	return -1;
}

short IsDirExist(DirNode dir, char fileName[14])
{

	for (int i = 0; i < 64; i++)
	{

		if (strcmp(fileName, dir.childItem[i].filename) == 0 && IsDir(dir.childItem[i].i_BNum))

			return dir.childItem[i].i_BNum;
	}

	return -1;
}

//================================================================

//函数描述：创建一个iNode,并分配磁盘空间



void CreateINode(short iNodeNum, char fileSpec, short linkCount, short length)
{

	iNode[iNodeNum].fileSpec = fileSpec;

	iNode[iNodeNum].linkCount = linkCount;

	iNode[iNodeNum].fileLength = length;

	//为目录磁盘，分配目录节点

	if (fileSpec == '1')
	{

		for (int i = 0; i < 4; i++)
		{
			iNode[iNodeNum].iaddr[i] = AssAnEmpty();
		}
		return;
	}

	//根据文件长度分配文件磁盘节点

	//直接寻址

	short i;

	i = 10;

	short left = length;

	while (left && i)
	{

		iNode[iNodeNum].iaddr[10 - i] = AssAnEmpty();

		left--;

		i--;
	}

	if (left > 0)
	{ //一级索引

		i = 512;

		short dir[512];

		iNode[iNodeNum].iaddr[10] = AssAnEmpty();

		while (left && i)
		{

			dir[512 - i] = AssAnEmpty();

			i--;

			left--;
		}

		BWriteArr(dir, iNode[iNodeNum].iaddr[10]);

		if (left > 0)
		{ //二级索引

			short k = 512;

			short j = 512;

			short dirChild[512];

			iNode[iNodeNum].iaddr[11] = AssAnEmpty();

			while (left && k)
			{ //二级索引1次寻址

				dir[512 - k] = AssAnEmpty();
				j = 512;
				while (left && j)
				{ //二级索引二次寻址
					dirChild[512 - j] = AssAnEmpty();
					left--;

					j--;
				}

				BWriteArr(dirChild, dir[512 - k]);

				BWriteArr(dir, iNode[iNodeNum].iaddr[11]); //写二级索引一次寻址中盘快记录的一次寻的盘快号

				k--;
			}
		}
	}
}

//================================================================

//函数描述：清空iNode信息,并分配磁盘空间



void CleanINode(INode& iNode)
{

	iNode.fileSpec = '2';

	iNode.linkCount = -1;

	iNode.fileLength = -1;

	//根据文件长度非配文件磁盘节点

	//直接寻址

	for (short i = 0; i < 13; i++)

		iNode.iaddr[i] = -1;
}

//================================================================

//函数描述：创建一个iNode,并分配磁盘空间



void InsertDir(char fileName[14], short blockNum)
{

	strcpy(rootDir->childItem[blockNum].filename, fileName);

	rootDir->childItem[rootDir->DirCount].i_BNum = blockNum;

	rootDir->DirCount++;

	return;
}

//================================================================

//函数描述：存在文件，并分配iNOde节点和磁盘空间



void Create(DirNode& dir, char fileName[14], short length, char fileSpec)
{

	if (length > freeTotalB)
	{

		cout << "当前文件超出长度" << endl;

		return;
	}
	short index = AssAnDir(dir);
	short iNodeNum = dir.childItem[index].i_BNum;
	if (index == -1)
	{
		printf("目录区已满，无法继续创建文件或目录\n");
		return;
	}
	CreateINode(iNodeNum, fileSpec, 0, length);

	strcpy(dir.childItem[index].filename, fileName);

	if (fileSpec == '1')
	{
		iNode[iNodeNum].linkCount = assAnDirMy();
	}
	//InsertDir(rootDir, fileName, iNodeNum);

	//BWrite(rootDirNum);//此处
}

//================================================================

//函数描述：创建一个文件，



void Mf(DirNode& dir, char fileName[14], short length)
{

	int iNodeNum = IsFileExist(dir, fileName);

	if (iNodeNum != -1)
	{ //有重名名，进一步判断

		cout << "当前文件已经存在，请重新输入文件名" << endl;
	}
	else
	{ //存在文件，为索引文件，或者无重名现象,创建文件，并分配iNOde节点和磁盘空间
		char  temp_choice = '1';
		short temp_temp = 0;
		printf("选择文件类型：\n");
		printf("1.正规文件     2.块设备     3.管道文件\n");
		cin >> temp_temp;
		fflush(stdin);
		if (temp_temp == 1)
			temp_choice = '0';
		if (temp_temp == 2)
			temp_choice = '3';
		if (temp_temp == 3)
			temp_choice = '4';
		Create(dir, fileName, length, temp_choice);
	}
}

//================================================================

//函数描述：在当前目录创建一个子目录

void Md(DirNode& dir, char fileName[14], short length)
{

	int iNodeNum = IsDirExist(dir, fileName);

	if (iNodeNum != -1)
	{ //有重名名，进一步判断

		cout << "当前目录已经存在，请重新输入目录名" << endl;
	}
	else
	{
		Create(dir, fileName, length, '1');
	}

} //================================================================
//================================================================
//函数描述：打开一个文件，显示其所用物理块

void ShowBlockInfo(INode iNode)
{

	short dir[512];

	short i;

	i = 10;

	short left = iNode.fileLength;
	printf("\n文件所用物理块：\n");
	while (left && i)
	{

		cout << (iNode.fileLength - left) << ":" << iNode.iaddr[10 - i] << "  ";

		left--;

		i--;
	}

	if (left > 0)
	{

		i = 512;

		short dir1[512];

		BReadArr(dir1, iNode.iaddr[10]);

		while (left && i)
		{

			cout << (iNode.fileLength - left) << ":" << dir1[512 - i] << "  ";

			i--;

			left--;
		}
	}

	if (left > 0)
	{ //二级索引

		short k = 512;

		short j = 512;

		short dirChild[512];

		BReadArr(dir, iNode.iaddr[11]);

		while (left && k)
		{ //二级索引1次寻址

			BReadArr(dirChild, dir[512 - k]);

			j = 512;

			while (left && j)
			{ //二级索引二次寻址

				cout << (iNode.fileLength - left) << ":" << dirChild[512 - j] << "  ";

				left--;

				j--;
			}

			k--;
		}
	}
}

//================================================================

//函数描述：打开一个文件，


void ShowFileInfo(INode iNode, char fileName[14])
{

	cout << "文件名        " << fileName;

	cout << "      文件类型  ";

	switch (iNode.fileSpec)
	{

	case '0':

		cout << "< 正规文件 > ";

		break;

	case '1':

		cout << "< 目录 > ";
		break;
	case '3':

		cout << "< 块设备 > ";
		break;
	case '4':

		cout << "< 管道文件 > ";
		break;
	default:
		cout << "错误";
	}

	cout << "         " << iNode.fileLength << "KB" << endl;
}

//================================================================

//函数描述：打开一个文件，



void OpenFile(DirNode& dir, char fileName[14])
{

	int blockNum = IsFileExist(dir, fileName);

	if (blockNum == -1)
	{ //不存在该文件，退出

		cout << "该文件按不存在" << endl;

		return;
	}

	else
	{

		ShowFileInfo(iNode[blockNum], fileName);

		ShowBlockInfo(iNode[blockNum]);

		cout << endl;
	}
}


void OpenDir(DirNode& dir, char fileName[14])
{

	int blockNum = IsDirExist(dir, fileName);

	if (blockNum == -1)
	{ //不存在该文件，退出

		cout << "该目录按不存在" << endl;

		return;
	}

	else
	{

		ShowFileInfo(iNode[blockNum], fileName);

		ShowBlockInfo(iNode[blockNum]);

		cout << endl;
	}
}
//================================================================

//函数描述：回收一块空余磁盘片

void CallBackOne(short diskNum)
{

	freeTotalB++;

	if (SS[0] < 49)
	{

		SS[0]++;

		SS[SS[0]] = diskNum;
	}
	else if (SS[0] == 49 && SS[1] != 0)
	{
		SS[0]++;

		SS[SS[0]] = diskNum;
	}
	else if (SS[0] == 49 && SS[1] == 0)
	{
		BWrite(SS, diskNum); //将空白的一组回写到上一组记录空闲盘快号的磁盘

		freeBlockNum = diskNum; //将当前空白的一组第一个盘快作为下一个盘组的记录盘

		//修改超级栈

		SS[1] = diskNum;

		SS[0] = 1;
	}
	else
	{ //SS[0]==50

		BWrite(SS, diskNum); //将空白的一组回写到上一组记录空闲盘快号的磁盘

		freeBlockNum = diskNum; //将当前空白的一组第一个盘快作为下一个盘组的记录盘

		//修改超级栈

		SS[1] = diskNum;

		SS[0] = 1;
	}
}

//================================================================

//函数描述：回收文件占用的磁盘

void CallBackDisk(INode& iNode)
{

	short i;

	i = 10;

	short left = iNode.fileLength;

	while (left && i)
	{ //直接索引回收

		CallBackOne(iNode.iaddr[10 - i]);

		left--;

		i--;
	}

	if (left > 0)
	{ //一级索引回收

		i = 512;

		short dir1[512];

		BReadArr(dir1, iNode.iaddr[10]);

		while (left && i)
		{

			CallBackOne(dir1[512 - i]);

			i--;

			left--;
		}

		CallBackOne(iNode.iaddr[10]);
	}

	if (left > 0)
	{ //二级索引

		short k = 512;

		short j = 512;

		short dir[512];

		short dirChild[512];

		BReadArr(dir, iNode.iaddr[11]); //二级索引1次寻址

		while (left && k)
		{ //二级索引1次寻址

			BReadArr(dirChild, dir[512 - k]);

			j = 512;
			while (left && j)
			{ //二级索引二次回收

				CallBackOne(dirChild[512 - j]);

				left--;

				j--;
			}

			CallBackOne(dir[512 - k]); //二级索引一次寻址

			k--;
		}

		CallBackOne(iNode.iaddr[11]);
	}
}

//================================================================

//函数描述：回收文件的iNOde节点

void CallBackINode(short diskNum)
{

	CallBackOne(diskNum);
}

//================================================================

//函数描述：删除索引中一项

void DelDirItem(DirNode& dir, short index)
{

	strcpy(dir.childItem[index].filename, NullName);

	dir.childItem[index].i_BNum = -1;
}

//================================================================

//函数描述：删除一个文件

void Del(DirNode& dir, char fileName[14])
{

	short blockNum = IsFileExist(dir, fileName);
	int index = -1;
	if (blockNum == -1)
	{ //不存在该文件，退出

		cout << "文件不存在，删除失败" << endl;
	}
	else
	{
		for (int i = 0; i < 64; i++)
		{

			if (strcmp(fileName, dir.childItem[i].filename) == 0 && IsFile(dir.childItem[i].i_BNum)) {
				index = i;
				break;
			}

		}
		CallBackDisk(iNode[blockNum]);

		DelDirItem(dir, index);

		CleanINode(iNode[blockNum]);
	}
}

//================================================================

//函数描述：删除一个目录


void recursionDel(DirNode& dir)
{
	if (dir.DirCount != 0)
	{
		for (int i = 0; i < 64; i++)
		{
			if (dir.childItem[i].i_BNum != -1) {
				if (iNode[dir.childItem[i].i_BNum].fileSpec == '0' || iNode[dir.childItem[i].i_BNum].fileSpec == '3' || iNode[dir.childItem[i].i_BNum].fileSpec == '4')
				{ //如果是文件

					CallBackDisk(iNode[dir.childItem[i].i_BNum]);

					DelDirItem(dir, i);

					CleanINode(iNode[dir.childItem[i].i_BNum]);
				}
				else
				{
					if (rootDir[iNode[dir.childItem[i].i_BNum].linkCount].DirCount != 0)
					{
						recursionDel(rootDir[iNode[dir.childItem[i].i_BNum].linkCount]);
						CallBackDisk(iNode[dir.childItem[i].i_BNum]);

						DelDirItem(dir, i);

						CleanINode(iNode[dir.childItem[i].i_BNum]);
						dir.DirCount = -1;
					}
					else
					{
						CallBackDisk(iNode[dir.childItem[i].i_BNum]);

						DelDirItem(dir, i);

						CleanINode(iNode[dir.childItem[i].i_BNum]);
						dir.DirCount = -1;
					}
				}
			}
		}
	}

}

void Rd(DirNode& dir, char fileName[14])
{

	short blockNum = IsDirExist(dir, fileName);  //返回子节点所在的INODE下标
	int index = -1;
	if (blockNum == -1)
	{ //不存在该文件，退出

		cout << "目录不存在，删除失败" << endl;
	}
	else
	{
		for (int i = 0; i < 64; i++)
		{

			if (strcmp(fileName, dir.childItem[i].filename) == 0 && IsDir(dir.childItem[i].i_BNum)) {
				index = i;
				break;
			}

		}
		recursionDel(rootDir[iNode[blockNum].linkCount]);
		CallBackDisk(iNode[blockNum]);
		DelDirItem(dir, index);
		CleanINode(iNode[blockNum]);
	}
}

//================================================================

//函数描述：显示目录项的内容


void Dir()
{

	for (int i = 0; i < 64; i++)
	{

		if (rootDir[currentDirIndex].childItem[i].i_BNum != -1)
		{

			//BRead(iNode[i], rootDir->childItem[i].i_BNum);

			ShowFileInfo(iNode[rootDir[currentDirIndex].childItem[i].i_BNum], rootDir[currentDirIndex].childItem[i].filename);
		}
	}
}

//================================================================

//函数描述：销毁资源



void exit()
{

	delete[] iNode;

	delete rootDir;
}

//================================================================

//函数描述：打印版权信息


void help() //打印命令及解释

{

	printf("                                              命令帮助\n");

	printf("    1:     init---------------------------------------------------------------------初始化系统\n");
	printf("\n");
	printf("    2:     mf-----------------------------------------------------------------------新建文件\n");
	printf("\n");
	printf("    3:     mkdir--------------------------------------------------------------------建立子目录\n");
	printf("\n");
	printf("    4:     cat----------------------------------------------------------------------查看文件信息\n");
	printf("\n");
	printf("    5:     rmf----------------------------------------------------------------------删除文件\n");
	printf("\n");
	printf("    6:     rmdir--------------------------------------------------------------------删除目录\n");
	printf("\n");
	printf("    7:     dir----------------------------------------------------------------------显示目录\n");
	printf("\n");
	printf("    8:     cd-----------------------------------------------------------------------进入指定目录\n");
	printf("\n");
	printf("    9:     cd..---------------------------------------------------------------------返回上一级目录\n");
	printf("\n");
	printf("    10:    help--------------------------------------------------------------------显示帮助命令\n");
	printf("\n");
	printf("    11:     cls--------------------------------------------------------------------清屏\n");
	printf("\n");
	
	cout << "==============================================================================\n"
		<< endl;

	cout << "\n请输入命令,回车确认" << endl;
}

void help_1() //打印命令及解释

{

	printf("                                              命令帮助\n");
	printf("    1:     cat----------------------------------------------------------------------查看文件信息\n");
	printf("\n");
	printf("    2:     dir----------------------------------------------------------------------显示目录\n");
	printf("\n");
	printf("    3:     cd-----------------------------------------------------------------------进入指定目录\n");
	printf("\n");
	printf("    4:     cd..---------------------------------------------------------------------返回上一级目录\n");
	printf("\n");
	printf("    5:    help--------------------------------------------------------------------显示帮助命令\n");
	printf("\n");
	printf("    6:     cls--------------------------------------------------------------------清屏\n");
	printf("\n");

	cout << "==============================================================================\n"
		<< endl;

	cout << "\n请输入命令,回车确认" << endl;
}

bool WritePara()
{
	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)
	{
		cout << "系统还未初始化" << endl;
		return false;
	}
	if (fseek(f, sizeof(short)* 51, 0))
		cout << "文件指针错误" << endl;

	fwrite(&freeBlockNum, sizeof(freeBlockNum), 1, f);
	fwrite(&freeTotalB, sizeof(freeTotalB), 1, f);

	fclose(f);
	return true;
}

bool readSystem()
{
	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)

	{
		cout << "系统还未初始化\n"
			<< endl;

		return false;
	}

	if (fseek(f, 1024 * 1, 0))

		cout << "文件指针错误" << endl;
	//设置文件指针
	for (int i = 0; i < 10; i++) {
		fread(rootDir[i].childItem, sizeof(DirChild), 64, f);
	}

	if (fseek(f, sizeof(short)* 53, 0))
		cout << "文件指针错误" << endl;

	for (int i = 0; i < 10; i++) {
		fread(&(rootDir[i].DirCount), sizeof(short), 1, f);
	}


	if (fseek(f, 1024 * 11, 0))

		cout << "文件指针错误" << endl;

	fread(iNode, sizeof(INode), 640, f);

	BRead(SS, 0);

	if (fseek(f, sizeof(short)* 51, 0))
		cout << "文件指针错误" << endl;

	fread(&freeBlockNum, sizeof(freeBlockNum), 1, f);

	fread(&freeTotalB, sizeof(freeTotalB), 1, f);

	fclose(f);
	cout << "系统文件载入完成\n"
		<< endl;
	return true;
}

void savaFile()
{
	WriteINode();
	WriteRootDir();
	WritePara();
}

void initSystem()
{
	// 打开日志文件，并使用 trunc 模式
	std::ofstream logFile("operation_log.txt", std::ios_base::trunc);

	// 检查文件是否成功打开
	if (!logFile.is_open()) {
		std::cerr << "无法打开文件" << std::endl;
		return;
	}

	// 关闭文件
	logFile.close();
	Format(); //初始化
	Linkdisk();
	Init();
	DirFloor = 0;
	currentDirIndex = 0;
	DirFloorArr[0] = 0;
}

void disPlayNowDir()
{
	for (int i = 0; i <= currentDirIndex; i++)
	{
		printf("%s", DirName[i]);
		printf("/");
	}
	printf(":");
}

void enterDir(DirNode dir, char fileName[14])
{
	for (int i = 0; i < 64; i++)
	{

		if (strcmp(fileName, dir.childItem[i].filename) == 0 && IsDir(dir.childItem[i].i_BNum))
		{
			currentDirIndex = iNode[dir.childItem[i].i_BNum].linkCount;
			DirFloor++;
			DirFloorArr[DirFloor] = currentDirIndex;
			strcpy(DirName[DirFloor], dir.childItem[i].filename);
			return;
		}
	}

	printf("此目录不存在\n");
}

void exitDir()
{
	if (DirFloor == 0)
	{
		return;
	}

	DirFloor--;
	currentDirIndex = DirFloorArr[DirFloor];

}

// 日志函数
void logAction(const std::string& action) {
	// 获取当前时间
	time_t now = time(nullptr);
	char* dt = ctime(&now);

	std::ofstream logFile("operation_log.txt", std::ios_base::app); // 打开日志文件，以追加模式写入
	logFile << action << " 操作时间：" << dt << std::endl; // 将操作和操作时间写入日志文件
}

int main()
{

	bool run = true;

	int length = 1024;

	char fileName[14];

	char dirName[14];

	int command;

	//AuthorMessage();

	if (!readSystem())
	{
		initSystem();
	}
	strcpy(DirName[0], "root");
	while (true) {
		printf("***********************************欢迎使用UNIX文件管理系统***********************************\n");
		printf("\n====================================请选择您要登录使用的用户====================================\n");
		printf("\n             1.超级管理员                                           2.普通用户\n");
		int role;
		scanf("%d", &role);
		fflush(stdin);
		if (role == 1) {
			help();
			while (run)
			{

				disPlayNowDir();

				while (true)
				{

					scanf("%d", &command);

					fflush(stdin);

					if (command > 0 && command <= 13)

						break;

					cout << "\n命令错误,请重新输入" << endl;
					getchar();
					command = -1;
				}

				switch (command)

				{

				case 2://建立文件

					cout << "\n请输入文件名" << endl; //新建文件

					cin >> fileName;

					fflush(stdin);

					cout << "\n请输入文件长度，单位KB" << endl;

					cin >> length;

					fflush(stdin);

					if (length < 0 || length > freeTotalB)
					{

						cout << "文件长度不合法\n"
							<< endl;

						break;
					}

					Mf(rootDir[currentDirIndex], fileName, length);
					savaFile();
					
					logAction("创建文件: 操作者=超级管理员 ，文件名 = " + std::string(fileName) + ", 长度 = " + std::to_string(length)+"K"); // 记录操作
					break;

				case 3://新建目录

					cout << "\n请输入目录名" << endl;

					cin >> dirName;

					Md(rootDir[currentDirIndex], dirName, 4);
					savaFile();
					logAction("创建目录: 操作者=超级管理员 ，目录名 = " + std::string(dirName) + ", 长度 = 4K"); // 记录操作
					break;

				case 4://打开文件

					cout << "\n请输入打开文件名" << endl; //打开文件

					cin >> fileName;

					OpenFile(rootDir[currentDirIndex], fileName);
					logAction("打开文件: 操作者=超级管理员 ，文件名 = " + std::string(fileName)); // 记录操作
					break;

				case 5://删除文件

					cout << "\n请输入删除文件名" << endl; //删除文件

					cin >> fileName;

					Del(rootDir[currentDirIndex], fileName);
					savaFile();
					logAction("删除文件: 操作者=超级管理员 ，文件名 = " + std::string(fileName)); // 记录操作
					break;

				case 6: //删除目录

					cout << "\n请输入删除的目录名" << endl;

					cin >> dirName;

					Rd(rootDir[currentDirIndex], dirName);
					savaFile();
					logAction("删除目录: 操作者=超级管理员 ，目录名 = " + std::string(dirName)); // 记录操作
					break;

				case 7://显示目录信息

					Dir(); //显示当前目录下的信息
					
					break;

				case 8://进入子目录

					printf("输入要进入的目录：");
					cin >> fileName;
					enterDir(rootDir[currentDirIndex], fileName);
					logAction("进入目录: 操作者=超级管理员 ，目录名 = " + std::string(fileName)); // 记录操作
					break;

				case 9://返回上一目录
					exitDir();//back
					break;


				case 1://初始化
					initSystem();
					break;

				case 10://帮助指令显示
					help();
					break;

					/*case 11:
						printf("还剩余%d个盘片\n", freeTotalB);
						break;*/

				case 11:
					system("cls"); //清屏
					break;

					/*case 13:
						cout << "\n请输入打开目录" << endl;

						cin >> fileName;
						OpenDir(rootDir[currentDirIndex], fileName);
						break;*/
				default:

					break;
				}
			}

		}
		if (role == 2) {
			help_1();
			while (run)
			{

				disPlayNowDir();

				while (true)
				{

					scanf("%d", &command);

					fflush(stdin);

					if (command > 0 && command <= 13)

						break;

					cout << "\n命令错误,请重新输入" << endl;
					getchar();
					command = -1;
				}

				switch (command)

				{
				case 1://打开文件

					cout << "\n请输入打开文件名" << endl; 

					cin >> fileName;

					OpenFile(rootDir[currentDirIndex], fileName);
					logAction("打开文件: 操作者=普通用户 ，文件名 = " + std::string(fileName)); // 记录操作
					break;
				case 2://显示目录信息

					Dir(); //显示当前目录下的信息

					break;

				case 3://进入子目录

					printf("输入要进入的目录：");
					cin >> fileName;
					enterDir(rootDir[currentDirIndex], fileName);
					logAction("进入目录: 操作者=普通用户 ，文件名 = " + std::string(fileName)); // 记录操作
					break;

				case 4://返回上一目录
					exitDir();//back
					break;


				

				case 5://帮助指令显示
					help_1();
					break;

					/*case 11:
						printf("还剩余%d个盘片\n", freeTotalB);
						break;*/

				case 6:
					system("cls"); //清屏
					break;

					/*case 13:
						cout << "\n请输入打开目录" << endl;

						cin >> fileName;
						OpenDir(rootDir[currentDirIndex], fileName);
						break;*/
				default:

					break;
				}
			}
		}
	}
	return 0;
}
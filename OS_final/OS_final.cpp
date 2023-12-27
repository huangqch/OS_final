#include <fstream>

#include <iostream>

#include <cstring>

#include<string>

#include <ctime>

using namespace std;

const char FileName[] = "os.txt"; //�����Ĵ��ļ�����

const char NullName[] = "00000000000000";

const int DirLen = 1;

const short SSNum = -1; //super block num

//i�ڵ�ṹ��Ϣ

struct INode
{

	char fileSpec; //�ڵ��ļ����ͣ�0��ʾNORMAL�������ļ��� 1��ʾDIR��Ŀ¼�ļ��� 3��ʾStock�����豸�� 4��ʾ���ܵ��ļ�����0-4�����ļ����ͣ�

	short iaddr[13]; //������0-13��ÿ��2byte����26byte

	int fileLength; //�ļ�����4byte

	char linkCount;  //������� 1byte
};//��32B

//Ŀ¼����Ϣ
struct DirChild
{
	char filename[14]; //�ļ���14byte

	short i_BNum; //i�ڵ��2byte
};//��16B

//Ŀ¼�ڵ���Ϣ
struct DirNode
{
	DirChild childItem[64]; //Ŀ¼�´�ŵ��ļ�����������Ŀ¼��

	short DirCount; //��ŵ��ļ�����
};

short SS[51]; //����ջ,ָ��ΪSS[0]��������

short freeBlockNum = 0; //��ǰ�����̿��������¼���̿��

short freeTotalB = 20450; //��ǰʣ������̿�����20480-30���ɷ���ṹ��

short rootDirNum; //��ǰĿ¼���ڵĴ��̿��

short currentDirIndex = 0; //��ǰ����Ŀ¼����Ŀ¼�ڵ㣨rootDir���е��±�

DirNode rootDir[10]; //����Ŀ¼�ڵ�

int DirFloorArr[10]; //��Ž����Ŀ¼�ڵ��±�
int DirFloor = 0;  //��ŵ�ǰDirFloorArr���±�
char DirName[10][14]; //��Ž������Ŀ¼����

INode iNode[640]; //����i�ڵ�

//================================================================

//��������������20M����


short assAnDirMy()//�ж�Ŀ¼�е��ļ�����������Ŀ¼�ļ�����С��0��Ŀ¼��
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

void Format()//����20M�ļ���С��ȫ���桮0��
{
	cout << "ϵͳ���ڳ�ʼ��......" << endl;

	// ���ļ�

	FILE* f = fopen(FileName, "wb+");

	if (f == NULL)
	{
		cout << "���򴴽���������������" << endl;
		return;
	}
	for (int i = 0; i < 20971520; i++) //20971520=20Mb,��ʱȫ����Ϊ0
		fprintf(f, "%c", '0');

	// �ر��ļ�
	fclose(f);
}

//================================================================

//�������������鸳ֵ

void ArrarEqual(short arr[51], short begin, short end)//���鸳ֵeg:begin=1��end=20,0<i<20,arr[50]=1,arr[49]=2,arr[48]=3.....

{

	for (short i = 0; i < end - begin + 1; i++)

		arr[50 - i] = begin + i;
}

//================================================================

//�����������ع�BWrite��ʵ��һ�������д��

void BWrite(short arr[51], short diskNum)//�������1024*diskNUmλ��д���ļ��У���diskNum��д�룬���0��ʼ��
{

	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)

	{

		cout << "д�ļ�����������������" << endl;

		return;
	}

	//�����ļ�ָ�룬���ļ�ָ���ƶ���1024*diskNum��

	if (fseek(f, 1024 * diskNum, 0))

		cout << "�ļ�ָ�����" << endl;

	//����������д���ļ���
	fwrite(arr, sizeof(short), 51, f);

	fclose(f);
}

void WriteINode()//i�ڵ�д�룬�����11������鿪ʼд�룬��iNode�����е�ǰ640��i�ڵ�д�뵽�ļ�f�У�20��i�ڵ�飬һ��32B*20=640B
{
	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)

	{
		cout << "д�ļ�����������������" << endl;
		return;
	}

	//�����ļ�ָ��

	if (fseek(f, 1024 * 11, 0))
		cout << "�ļ�ָ�����" << endl;

	fwrite(iNode, sizeof(INode), 640, f);

	fclose(f);
}

void WriteRootDir()//Ŀ¼�ļ�д���1��ʼ
{
	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)

	{

		cout << "д�ļ�����������������" << endl;

		return;
	}

	//�����ļ�ָ��

	if (fseek(f, 1024 * 1, 0))
		cout << "�ļ�ָ�����" << endl;
	for (int i = 0; i < 10; i++) {
		fwrite(rootDir[i].childItem, sizeof(DirChild), 64, f);
	}

	if (fseek(f, sizeof(short)* 53, 0))
		cout << "�ļ�ָ�����" << endl;
	for (int i = 0; i < 10; i++) {
		fwrite(&(rootDir[i].DirCount), sizeof(short), 1, f);
	}


	fclose(f);
}
//================================================================

//�����������ع�BWrite��ʵ��һ�������д��,���鳤�Ȳ�ȷ��
void BWriteArr(short arr[512], short diskNum)//ͬ��д��
{

	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)

	{
		cout << "д�ļ�����������������" << endl;
		return;
	}

	//�����ļ�ָ��

	if (fseek(f, 1024 * diskNum, 0))

		cout << "�ļ�ָ�����" << endl;

	fwrite(arr, sizeof(short), 512, f);

	fclose(f);
}

//================================================================

//�����������Ӵ����ж�������

void BRead(short arr[51], short diskNum)//ͬ�϶���
{

	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)

	{

		cout << "���ļ�����������������" << endl;

		return;
	}

	//�����ļ�ָ��

	if (fseek(f, 1024 * diskNum, 0))

		cout << "�ļ�ָ�����" << endl;

	fread(arr, sizeof(short), 51, f);

	fclose(f);
}

//================================================================

//�����������Ӵ����ж�������, ���뵽iNOde��

void BReadArr(short arr[512], short diskNum)
{

	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)

	{
		cout << "���ļ�����������������" << endl;

		return;
	}

	//�����ļ�ָ��

	if (fseek(f, 1024 * diskNum, 0))

		cout << "�ļ�ָ�����" << endl;

	fread(arr, sizeof(short), 512, f);

	fclose(f);
}

//================================================================

//����������д��һ��Ŀ¼��

void BWrite(short diskNum)//��Ŀ¼��д���ļ�����д�����ڵ�ţ���д�ļ���
{

	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)

	{

		cout << "д�ļ�����������������" << endl;

		return;
	}

	//�����ļ�ָ��

	if (fseek(f, long(1024 * diskNum), 0))

		cout << "�ļ�ָ�����" << endl;

	for (int i = 0; i < 64; i++)

	{

		fprintf(f, "%hd", rootDir->childItem[i].i_BNum);

		fputs(rootDir->childItem[i].filename, f);
	}

	fclose(f);
}

//================================================================

//��������������һ�����е���ͨ�죬���ط���Ŀ��

short AssAnEmpty()
{

	short temp;

	if (SS[0] > 1)
	{

		SS[0]--;//N--

		temp = SS[SS[0] + 1];//��λ����λ��

		//                               SS[SS[0]+1]=-1;

		freeTotalB--; //��ʣ����-1

		return temp;
	}
	else
	{

		if (SS[1] == 0)
		{

			cout << "��Ƭ�þ�" << endl;

			return -1;
		}

		//temp = freeBlockNum;

		freeBlockNum = SS[1]; //���浱ǰ����Ŀ��

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

short AssAnNode()//�ж��������
{

	for (short i = 0; i < 640; i++)
	{
		if (iNode[i].fileLength == -1)
		{
			return i;
		}
	}
	printf("�޽ڵ�ɷ���\n");
	return -1;
}

//================================================================

//��������������һ�����е�Ŀ¼��

short AssAnDir(DirNode& dir)
{

	if (dir.DirCount == 64)
	{

		cout << "��Ŀ¼����" << endl;

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

//��������������һ���ļ��ڵ㣬������INOde�ʹ��̿ռ�

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

//������������ʼ��

void Init(char fielSpec, short diskNum)
{

	InitCreate(fielSpec, diskNum);

	BRead(SS, 0);
}

//================================================================

//������������ʼ��������

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

	freeBlockNum = 0; //��ǰ�����̿��������¼���̿��

	freeTotalB = 20450;
	BRead(SS, 0);

	//�����ļ����ȷ����ļ����̽ڵ�

	//ֱ��Ѱַ
}

//================================================================

//�����������������ӳ�ʼ��



void Linkdisk()
{

	//��ʱ����ջ

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

	cout << "����os.txt��ɴ���,��С20MB" << endl;
}
//================================================================

//�����������ж�һ���ļ���һ����ͨ�ļ�


bool IsFile(short diskNum)
{

	if (iNode[diskNum].fileSpec == '0' || iNode[diskNum].fileSpec == '3' || iNode[diskNum].fileSpec == '4')

		return true;

	else

		return false;
}

//================================================================

//�����������ж�һ���ļ���һ��Ŀ¼�ļ�



bool IsDir(short diskNum)
{

	if (iNode[diskNum].fileSpec == '1')
		return true;

	else

		return false;
}
//================================================================

//�����������ж�һ���ļ��Ƿ����



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

//��������������һ��iNode,��������̿ռ�



void CreateINode(short iNodeNum, char fileSpec, short linkCount, short length)
{

	iNode[iNodeNum].fileSpec = fileSpec;

	iNode[iNodeNum].linkCount = linkCount;

	iNode[iNodeNum].fileLength = length;

	//ΪĿ¼���̣�����Ŀ¼�ڵ�

	if (fileSpec == '1')
	{

		for (int i = 0; i < 4; i++)
		{
			iNode[iNodeNum].iaddr[i] = AssAnEmpty();
		}
		return;
	}

	//�����ļ����ȷ����ļ����̽ڵ�

	//ֱ��Ѱַ

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
	{ //һ������

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
		{ //��������

			short k = 512;

			short j = 512;

			short dirChild[512];

			iNode[iNodeNum].iaddr[11] = AssAnEmpty();

			while (left && k)
			{ //��������1��Ѱַ

				dir[512 - k] = AssAnEmpty();
				j = 512;
				while (left && j)
				{ //������������Ѱַ
					dirChild[512 - j] = AssAnEmpty();
					left--;

					j--;
				}

				BWriteArr(dirChild, dir[512 - k]);

				BWriteArr(dir, iNode[iNodeNum].iaddr[11]); //д��������һ��Ѱַ���̿��¼��һ��Ѱ���̿��

				k--;
			}
		}
	}
}

//================================================================

//�������������iNode��Ϣ,��������̿ռ�



void CleanINode(INode& iNode)
{

	iNode.fileSpec = '2';

	iNode.linkCount = -1;

	iNode.fileLength = -1;

	//�����ļ����ȷ����ļ����̽ڵ�

	//ֱ��Ѱַ

	for (short i = 0; i < 13; i++)

		iNode.iaddr[i] = -1;
}

//================================================================

//��������������һ��iNode,��������̿ռ�



void InsertDir(char fileName[14], short blockNum)
{

	strcpy(rootDir->childItem[blockNum].filename, fileName);

	rootDir->childItem[rootDir->DirCount].i_BNum = blockNum;

	rootDir->DirCount++;

	return;
}

//================================================================

//���������������ļ���������iNOde�ڵ�ʹ��̿ռ�



void Create(DirNode& dir, char fileName[14], short length, char fileSpec)
{

	if (length > freeTotalB)
	{

		cout << "��ǰ�ļ���������" << endl;

		return;
	}
	short index = AssAnDir(dir);
	short iNodeNum = dir.childItem[index].i_BNum;
	if (index == -1)
	{
		printf("Ŀ¼���������޷����������ļ���Ŀ¼\n");
		return;
	}
	CreateINode(iNodeNum, fileSpec, 0, length);

	strcpy(dir.childItem[index].filename, fileName);

	if (fileSpec == '1')
	{
		iNode[iNodeNum].linkCount = assAnDirMy();
	}
	//InsertDir(rootDir, fileName, iNodeNum);

	//BWrite(rootDirNum);//�˴�
}

//================================================================

//��������������һ���ļ���



void Mf(DirNode& dir, char fileName[14], short length)
{

	int iNodeNum = IsFileExist(dir, fileName);

	if (iNodeNum != -1)
	{ //������������һ���ж�

		cout << "��ǰ�ļ��Ѿ����ڣ������������ļ���" << endl;
	}
	else
	{ //�����ļ���Ϊ�����ļ�����������������,�����ļ���������iNOde�ڵ�ʹ��̿ռ�
		char  temp_choice = '1';
		short temp_temp = 0;
		printf("ѡ���ļ����ͣ�\n");
		printf("1.�����ļ�     2.���豸     3.�ܵ��ļ�\n");
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

//�����������ڵ�ǰĿ¼����һ����Ŀ¼

void Md(DirNode& dir, char fileName[14], short length)
{

	int iNodeNum = IsDirExist(dir, fileName);

	if (iNodeNum != -1)
	{ //������������һ���ж�

		cout << "��ǰĿ¼�Ѿ����ڣ�����������Ŀ¼��" << endl;
	}
	else
	{
		Create(dir, fileName, length, '1');
	}

} //================================================================
//================================================================
//������������һ���ļ�����ʾ�����������

void ShowBlockInfo(INode iNode)
{

	short dir[512];

	short i;

	i = 10;

	short left = iNode.fileLength;
	printf("\n�ļ���������飺\n");
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
	{ //��������

		short k = 512;

		short j = 512;

		short dirChild[512];

		BReadArr(dir, iNode.iaddr[11]);

		while (left && k)
		{ //��������1��Ѱַ

			BReadArr(dirChild, dir[512 - k]);

			j = 512;

			while (left && j)
			{ //������������Ѱַ

				cout << (iNode.fileLength - left) << ":" << dirChild[512 - j] << "  ";

				left--;

				j--;
			}

			k--;
		}
	}
}

//================================================================

//������������һ���ļ���


void ShowFileInfo(INode iNode, char fileName[14])
{

	cout << "�ļ���        " << fileName;

	cout << "      �ļ�����  ";

	switch (iNode.fileSpec)
	{

	case '0':

		cout << "< �����ļ� > ";

		break;

	case '1':

		cout << "< Ŀ¼ > ";
		break;
	case '3':

		cout << "< ���豸 > ";
		break;
	case '4':

		cout << "< �ܵ��ļ� > ";
		break;
	default:
		cout << "����";
	}

	cout << "         " << iNode.fileLength << "KB" << endl;
}

//================================================================

//������������һ���ļ���



void OpenFile(DirNode& dir, char fileName[14])
{

	int blockNum = IsFileExist(dir, fileName);

	if (blockNum == -1)
	{ //�����ڸ��ļ����˳�

		cout << "���ļ���������" << endl;

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
	{ //�����ڸ��ļ����˳�

		cout << "��Ŀ¼��������" << endl;

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

//��������������һ��������Ƭ

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
		BWrite(SS, diskNum); //���հ׵�һ���д����һ���¼�����̿�ŵĴ���

		freeBlockNum = diskNum; //����ǰ�հ׵�һ���һ���̿���Ϊ��һ������ļ�¼��

		//�޸ĳ���ջ

		SS[1] = diskNum;

		SS[0] = 1;
	}
	else
	{ //SS[0]==50

		BWrite(SS, diskNum); //���հ׵�һ���д����һ���¼�����̿�ŵĴ���

		freeBlockNum = diskNum; //����ǰ�հ׵�һ���һ���̿���Ϊ��һ������ļ�¼��

		//�޸ĳ���ջ

		SS[1] = diskNum;

		SS[0] = 1;
	}
}

//================================================================

//���������������ļ�ռ�õĴ���

void CallBackDisk(INode& iNode)
{

	short i;

	i = 10;

	short left = iNode.fileLength;

	while (left && i)
	{ //ֱ����������

		CallBackOne(iNode.iaddr[10 - i]);

		left--;

		i--;
	}

	if (left > 0)
	{ //һ����������

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
	{ //��������

		short k = 512;

		short j = 512;

		short dir[512];

		short dirChild[512];

		BReadArr(dir, iNode.iaddr[11]); //��������1��Ѱַ

		while (left && k)
		{ //��������1��Ѱַ

			BReadArr(dirChild, dir[512 - k]);

			j = 512;
			while (left && j)
			{ //�����������λ���

				CallBackOne(dirChild[512 - j]);

				left--;

				j--;
			}

			CallBackOne(dir[512 - k]); //��������һ��Ѱַ

			k--;
		}

		CallBackOne(iNode.iaddr[11]);
	}
}

//================================================================

//���������������ļ���iNOde�ڵ�

void CallBackINode(short diskNum)
{

	CallBackOne(diskNum);
}

//================================================================

//����������ɾ��������һ��

void DelDirItem(DirNode& dir, short index)
{

	strcpy(dir.childItem[index].filename, NullName);

	dir.childItem[index].i_BNum = -1;
}

//================================================================

//����������ɾ��һ���ļ�

void Del(DirNode& dir, char fileName[14])
{

	short blockNum = IsFileExist(dir, fileName);
	int index = -1;
	if (blockNum == -1)
	{ //�����ڸ��ļ����˳�

		cout << "�ļ������ڣ�ɾ��ʧ��" << endl;
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

//����������ɾ��һ��Ŀ¼


void recursionDel(DirNode& dir)
{
	if (dir.DirCount != 0)
	{
		for (int i = 0; i < 64; i++)
		{
			if (dir.childItem[i].i_BNum != -1) {
				if (iNode[dir.childItem[i].i_BNum].fileSpec == '0' || iNode[dir.childItem[i].i_BNum].fileSpec == '3' || iNode[dir.childItem[i].i_BNum].fileSpec == '4')
				{ //������ļ�

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

	short blockNum = IsDirExist(dir, fileName);  //�����ӽڵ����ڵ�INODE�±�
	int index = -1;
	if (blockNum == -1)
	{ //�����ڸ��ļ����˳�

		cout << "Ŀ¼�����ڣ�ɾ��ʧ��" << endl;
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

//������������ʾĿ¼�������


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

//����������������Դ



void exit()
{

	delete[] iNode;

	delete rootDir;
}

//================================================================

//������������ӡ��Ȩ��Ϣ


void help() //��ӡ�������

{

	printf("                                              �������\n");

	printf("    1:     init---------------------------------------------------------------------��ʼ��ϵͳ\n");
	printf("\n");
	printf("    2:     mf-----------------------------------------------------------------------�½��ļ�\n");
	printf("\n");
	printf("    3:     mkdir--------------------------------------------------------------------������Ŀ¼\n");
	printf("\n");
	printf("    4:     cat----------------------------------------------------------------------�鿴�ļ���Ϣ\n");
	printf("\n");
	printf("    5:     rmf----------------------------------------------------------------------ɾ���ļ�\n");
	printf("\n");
	printf("    6:     rmdir--------------------------------------------------------------------ɾ��Ŀ¼\n");
	printf("\n");
	printf("    7:     dir----------------------------------------------------------------------��ʾĿ¼\n");
	printf("\n");
	printf("    8:     cd-----------------------------------------------------------------------����ָ��Ŀ¼\n");
	printf("\n");
	printf("    9:     cd..---------------------------------------------------------------------������һ��Ŀ¼\n");
	printf("\n");
	printf("    10:    help--------------------------------------------------------------------��ʾ��������\n");
	printf("\n");
	printf("    11:     cls--------------------------------------------------------------------����\n");
	printf("\n");
	
	cout << "==============================================================================\n"
		<< endl;

	cout << "\n����������,�س�ȷ��" << endl;
}

void help_1() //��ӡ�������

{

	printf("                                              �������\n");
	printf("    1:     cat----------------------------------------------------------------------�鿴�ļ���Ϣ\n");
	printf("\n");
	printf("    2:     dir----------------------------------------------------------------------��ʾĿ¼\n");
	printf("\n");
	printf("    3:     cd-----------------------------------------------------------------------����ָ��Ŀ¼\n");
	printf("\n");
	printf("    4:     cd..---------------------------------------------------------------------������һ��Ŀ¼\n");
	printf("\n");
	printf("    5:    help--------------------------------------------------------------------��ʾ��������\n");
	printf("\n");
	printf("    6:     cls--------------------------------------------------------------------����\n");
	printf("\n");

	cout << "==============================================================================\n"
		<< endl;

	cout << "\n����������,�س�ȷ��" << endl;
}

bool WritePara()
{
	FILE* f = fopen(FileName, "rb+");

	if (f == NULL)
	{
		cout << "ϵͳ��δ��ʼ��" << endl;
		return false;
	}
	if (fseek(f, sizeof(short)* 51, 0))
		cout << "�ļ�ָ�����" << endl;

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
		cout << "ϵͳ��δ��ʼ��\n"
			<< endl;

		return false;
	}

	if (fseek(f, 1024 * 1, 0))

		cout << "�ļ�ָ�����" << endl;
	//�����ļ�ָ��
	for (int i = 0; i < 10; i++) {
		fread(rootDir[i].childItem, sizeof(DirChild), 64, f);
	}

	if (fseek(f, sizeof(short)* 53, 0))
		cout << "�ļ�ָ�����" << endl;

	for (int i = 0; i < 10; i++) {
		fread(&(rootDir[i].DirCount), sizeof(short), 1, f);
	}


	if (fseek(f, 1024 * 11, 0))

		cout << "�ļ�ָ�����" << endl;

	fread(iNode, sizeof(INode), 640, f);

	BRead(SS, 0);

	if (fseek(f, sizeof(short)* 51, 0))
		cout << "�ļ�ָ�����" << endl;

	fread(&freeBlockNum, sizeof(freeBlockNum), 1, f);

	fread(&freeTotalB, sizeof(freeTotalB), 1, f);

	fclose(f);
	cout << "ϵͳ�ļ��������\n"
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
	// ����־�ļ�����ʹ�� trunc ģʽ
	std::ofstream logFile("operation_log.txt", std::ios_base::trunc);

	// ����ļ��Ƿ�ɹ���
	if (!logFile.is_open()) {
		std::cerr << "�޷����ļ�" << std::endl;
		return;
	}

	// �ر��ļ�
	logFile.close();
	Format(); //��ʼ��
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

	printf("��Ŀ¼������\n");
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

// ��־����
void logAction(const std::string& action) {
	// ��ȡ��ǰʱ��
	time_t now = time(nullptr);
	char* dt = ctime(&now);

	std::ofstream logFile("operation_log.txt", std::ios_base::app); // ����־�ļ�����׷��ģʽд��
	logFile << action << " ����ʱ�䣺" << dt << std::endl; // �������Ͳ���ʱ��д����־�ļ�
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
		printf("***********************************��ӭʹ��UNIX�ļ�����ϵͳ***********************************\n");
		printf("\n====================================��ѡ����Ҫ��¼ʹ�õ��û�====================================\n");
		printf("\n             1.��������Ա                                           2.��ͨ�û�\n");
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

					cout << "\n�������,����������" << endl;
					getchar();
					command = -1;
				}

				switch (command)

				{

				case 2://�����ļ�

					cout << "\n�������ļ���" << endl; //�½��ļ�

					cin >> fileName;

					fflush(stdin);

					cout << "\n�������ļ����ȣ���λKB" << endl;

					cin >> length;

					fflush(stdin);

					if (length < 0 || length > freeTotalB)
					{

						cout << "�ļ����Ȳ��Ϸ�\n"
							<< endl;

						break;
					}

					Mf(rootDir[currentDirIndex], fileName, length);
					savaFile();
					
					logAction("�����ļ�: ������=��������Ա ���ļ��� = " + std::string(fileName) + ", ���� = " + std::to_string(length)+"K"); // ��¼����
					break;

				case 3://�½�Ŀ¼

					cout << "\n������Ŀ¼��" << endl;

					cin >> dirName;

					Md(rootDir[currentDirIndex], dirName, 4);
					savaFile();
					logAction("����Ŀ¼: ������=��������Ա ��Ŀ¼�� = " + std::string(dirName) + ", ���� = 4K"); // ��¼����
					break;

				case 4://���ļ�

					cout << "\n��������ļ���" << endl; //���ļ�

					cin >> fileName;

					OpenFile(rootDir[currentDirIndex], fileName);
					logAction("���ļ�: ������=��������Ա ���ļ��� = " + std::string(fileName)); // ��¼����
					break;

				case 5://ɾ���ļ�

					cout << "\n������ɾ���ļ���" << endl; //ɾ���ļ�

					cin >> fileName;

					Del(rootDir[currentDirIndex], fileName);
					savaFile();
					logAction("ɾ���ļ�: ������=��������Ա ���ļ��� = " + std::string(fileName)); // ��¼����
					break;

				case 6: //ɾ��Ŀ¼

					cout << "\n������ɾ����Ŀ¼��" << endl;

					cin >> dirName;

					Rd(rootDir[currentDirIndex], dirName);
					savaFile();
					logAction("ɾ��Ŀ¼: ������=��������Ա ��Ŀ¼�� = " + std::string(dirName)); // ��¼����
					break;

				case 7://��ʾĿ¼��Ϣ

					Dir(); //��ʾ��ǰĿ¼�µ���Ϣ
					
					break;

				case 8://������Ŀ¼

					printf("����Ҫ�����Ŀ¼��");
					cin >> fileName;
					enterDir(rootDir[currentDirIndex], fileName);
					logAction("����Ŀ¼: ������=��������Ա ��Ŀ¼�� = " + std::string(fileName)); // ��¼����
					break;

				case 9://������һĿ¼
					exitDir();//back
					break;


				case 1://��ʼ��
					initSystem();
					break;

				case 10://����ָ����ʾ
					help();
					break;

					/*case 11:
						printf("��ʣ��%d����Ƭ\n", freeTotalB);
						break;*/

				case 11:
					system("cls"); //����
					break;

					/*case 13:
						cout << "\n�������Ŀ¼" << endl;

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

					cout << "\n�������,����������" << endl;
					getchar();
					command = -1;
				}

				switch (command)

				{
				case 1://���ļ�

					cout << "\n��������ļ���" << endl; 

					cin >> fileName;

					OpenFile(rootDir[currentDirIndex], fileName);
					logAction("���ļ�: ������=��ͨ�û� ���ļ��� = " + std::string(fileName)); // ��¼����
					break;
				case 2://��ʾĿ¼��Ϣ

					Dir(); //��ʾ��ǰĿ¼�µ���Ϣ

					break;

				case 3://������Ŀ¼

					printf("����Ҫ�����Ŀ¼��");
					cin >> fileName;
					enterDir(rootDir[currentDirIndex], fileName);
					logAction("����Ŀ¼: ������=��ͨ�û� ���ļ��� = " + std::string(fileName)); // ��¼����
					break;

				case 4://������һĿ¼
					exitDir();//back
					break;


				

				case 5://����ָ����ʾ
					help_1();
					break;

					/*case 11:
						printf("��ʣ��%d����Ƭ\n", freeTotalB);
						break;*/

				case 6:
					system("cls"); //����
					break;

					/*case 13:
						cout << "\n�������Ŀ¼" << endl;

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
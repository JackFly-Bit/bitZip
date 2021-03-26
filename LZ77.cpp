#include "LZ77.h"

#include <iostream>
using namespace std;
#include <assert.h>

LZ77::LZ77()
: _pWin(new uch[2 * WSIZE])
, _ht(WSIZE)
{ }

LZ77::~LZ77()
{
	delete[] _pWin;
	_pWin = nullptr;
}

void LZ77::CompressLZ77(const string& filePath)
{
	FILE* fIn = fopen(filePath.c_str(), "rb"); 
	if (nullptr == fIn)
	{
		cout << "待压缩文件路径出错" << endl;
		return;
	}

	//文件太小时则不进行压缩---文件小于3个字节则不压缩
	fseek(fIn, 0, SEEK_END);
	ulg fileSize = ftell(fIn);
	fseek(fIn, 0, SEEK_SET);
	if (fileSize <= MIN_MATCH)
	{
		cout << "文件太小了，小于3个字节不压缩" << endl;
		fclose(fIn);
		return;
	}

	//读取一个窗口的数据
	ulg lookahead = fread(_pWin, 1, 2 * WSIZE, fIn);

	//因为哈希地址--三个字符一组来进行计算的
	//而Insert里面传入的ch时三个字符中的第三个
	//因此在正式压缩前，需要先使用前2个字节预算hashAddr
	ush hashAddr = 0;
	ush matchHead = 0;
	for (ush i = 0; i < MIN_MATCH - 1; i++)
	{
		_ht.Insert(hashAddr, _pWin[i], i, matchHead);
	}

	FILE* fOut = fopen("111.txt", "wb");

	//该文件是用来写解压缩时所用到的标记信息的
	FILE* fFlag = fopen("temp.txt", "wb");

	ush start = 0;
	ush curMatchLength = 0;
	ush curMatchDist = 0;

	uch ch = 0;
	uch bitCount = 0;
	while (lookahead)
	{
		//将三个字符一组，往哈希桶中进行插入
		_ht.Insert(hashAddr, _pWin[start + 2], start, matchHead);

		//在插入之后，matchHead会将查找缓冲区中最近的一个匹配的位置带出来
		if (0 == matchHead)
		{
			//该三个字节之前没有出现过
			//将当前start位置的字节原封不动的写入到压缩文件单中
			fputc(_pWin[start], fOut);
			start++;
			lookahead--;

			WriteFlag(fFlag, false, ch, bitCount);
		}
		else
		{
			//找最长匹配了
			curMatchLength = LongestMatch(matchHead, start, curMatchDist);//<长度,距离>

			//将该<长度，距离>对写入到压缩文件中
			fputc(curMatchLength - 3, fOut);
			fwrite(&curMatchDist, 2, 1, fOut);

			WriteFlag(fFlag, true, ch, bitCount);
			lookahead -= curMatchLength;

			//需要将匹配内容三个字节一组往哈希表格中插入
			curMatchLength -= 1; //因为start位置开始的字节已经插入了
			while (curMatchLength)
			{
				++start;
				_ht.Insert(hashAddr, _pWin[start + 2], start, matchHead);
				curMatchLength--;
			}
			++start;
		}
		fflush(fOut);
	}
	if (bitCount > 0 && bitCount < 8)
	{
		ch <<= (8 - bitCount);
		fputc(ch, fFlag);
	}

	fclose(fIn);
	fclose(fFlag);

	//将两个文件合并成一个文件
	MergeFile(fOut, fileSize);
	fclose(fOut);
}

ush LZ77::LongestMatch(ush matchHead, ush start, ush& curMatchDist)
{
	ush maxLen = 0;
	uch maxMatchCount = 255;
	do
	{
		uch* pScan = _pWin + matchHead;
		uch* pEnd = pScan + MAX_MATCH;
		uch* pStart = _pWin + start;

		ush curLength = 0;
		ush curDist = 0;

		//找一个匹配
		while (pScan < pEnd && *pScan == *pStart)
		{
			pScan++;
			pStart++;
			curLength++;
		}
		curDist = start - matchHead;

		//确保最长匹配
		if (curLength > maxLen)
		{
			maxLen = curLength;
			curMatchDist = curDist;
		}
	} while ((matchHead = _ht.GetNext(matchHead)) && maxMatchCount--);

	return maxLen;
}

void LZ77::WriteFlag(FILE* temp, bool isDist, uch& ch, uch& bitCount)
{
	ch <<= 1;
	if (isDist)
		ch |= 1;
	bitCount++;
	if (8 == bitCount)
	{
		fputc(ch, temp);
		ch = 0;
		bitCount = 0;
	}
}

void LZ77::MergeFile(FILE* fOut, ulg fileSize)
{
	FILE* fFlag = fopen("temp.txt", "rb");
	assert(fFlag);

	uch readBuff[1024];
	size_t flagSize = 0;
	while (true)
	{
		size_t rdSize = fread(readBuff, 1, 1024, fFlag);
		if (0 == rdSize)
			break;
		flagSize += rdSize;
		fwrite(readBuff, 1, rdSize, fOut);
	}

	fwrite(&flagSize, sizeof(flagSize), 1, fOut);
	fwrite(&fileSize, sizeof(fileSize), 1, fOut);
	fclose(fFlag);
	remove("temp.txt");
}

void LZ77::UNCompressLZ77(const string& filePath)
{
	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (nullptr == fIn)
	{
		cout << "待压缩的文件路径有问题" << endl;
		return;
	}
	//获取源文件的大小
	fseek(fIn, 0 - sizeof(ulg), SEEK_END);
	ulg fileSize = 0;
	fread(&fileSize, sizeof(fileSize), 1, fIn);

	//获取标记的大小
	size_t flagSize = 0;
	fseek(fIn, 0 - sizeof(fileSize)-sizeof(flagSize), SEEK_END);
	fread(&flagSize, sizeof(flagSize), 1, fIn);

	fseek(fIn, 0, SEEK_SET);

	FILE* fFlag = fopen(filePath.c_str(), "rb");
	fseek(fFlag, 0 - sizeof(fileSize)-sizeof(flagSize) - flagSize, SEEK_END);

	//解压缩
	FILE* fOut = fopen("222.txt", "wb");

	FILE* fRead = fopen("222.txt", "rb");
	uch ch = 0;
	uch bitCount = 0;
	size_t i = 0;
	uch chData = 0;
	ulg compressCount = 0;
	while (i < flagSize)
	{
		if (0 == bitCount)
		{
			ch = fgetc(fFlag);
			bitCount = 8;
			i++;
		}
		//检测该比特位是0还是1
		if (ch & 0x80)
		{
			//长度距离对
			ush matchLength = fgetc(fIn) + 3;
			ush matchDist = 0;
			fread(&matchDist, sizeof(matchDist), 1, fIn);

			//切记:解压缩出来的数据可能还在缓冲区中
			fflush(fOut);

			fseek(fRead,  0 - matchDist, SEEK_END);
			compressCount += matchLength;

			while (matchLength)
			{
				chData = fgetc(fRead);
				fputc(chData, fOut);
				fflush(fOut);
				matchLength--;
			}
			if (compressCount == fileSize)
				break;
		}
		else
		{
			//原字符
			chData = fgetc(fIn);
			fputc(chData, fOut);
			compressCount++;
		}
		bitCount--;
		ch <<= 1;

		if (compressCount == fileSize)
			break;
	}
	fclose(fIn);
	fclose(fFlag);
	fclose(fOut);
	fclose(fRead);
}
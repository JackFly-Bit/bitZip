#include "LZ77.h"

#include <iostream>
using namespace std;

LZ77::LZ77()
: _pWin(new uch[2 * WSIZE])
, _ht(WSIZE)
{ }

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
	ush start = 0;
	ush curMatchLength = 0;
	ush curMatchDist = 0;
	while ()
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
		}
		else
		{
			//找最长匹配了
			curMatchLength = LongestMatch(matchHead, start, curMatchDist);//<长度,距离>

			//将该<长度，距离>对写入到压缩文件中
			fputc(curMatchLength - 3, fOut);
			fwrite(&curMatchDist, 2, 1, fOut);
		}
	}

	fclose(fIn);
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

		return maxLen;
	} while (matchHead = _ht.GetNext(matchHead) && maxMatchCount--);
}

void LZ77::UNCompressLZ77(const string& filePath)
{

}
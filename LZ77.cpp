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
		cout << "��ѹ���ļ�·������" << endl;
		return;
	}

	//�ļ�̫Сʱ�򲻽���ѹ��---�ļ�С��3���ֽ���ѹ��
	fseek(fIn, 0, SEEK_END);
	ulg fileSize = ftell(fIn);
	fseek(fIn, 0, SEEK_SET);
	if (fileSize <= MIN_MATCH)
	{
		cout << "�ļ�̫С�ˣ�С��3���ֽڲ�ѹ��" << endl;
		fclose(fIn);
		return;
	}

	//��ȡһ�����ڵ�����
	ulg lookahead = fread(_pWin, 1, 2 * WSIZE, fIn);

	//��Ϊ��ϣ��ַ--�����ַ�һ�������м����
	//��Insert���洫���chʱ�����ַ��еĵ�����
	//�������ʽѹ��ǰ����Ҫ��ʹ��ǰ2���ֽ�Ԥ��hashAddr
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
		//�������ַ�һ�飬����ϣͰ�н��в���
		_ht.Insert(hashAddr, _pWin[start + 2], start, matchHead);

		//�ڲ���֮��matchHead�Ὣ���һ������������һ��ƥ���λ�ô�����
		if (0 == matchHead)
		{
			//�������ֽ�֮ǰû�г��ֹ�
			//����ǰstartλ�õ��ֽ�ԭ�ⲻ����д�뵽ѹ���ļ�����
			fputc(_pWin[start], fOut);
			start++;
		}
		else
		{
			//���ƥ����
			curMatchLength = LongestMatch(matchHead, start, curMatchDist);//<����,����>

			//����<���ȣ�����>��д�뵽ѹ���ļ���
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

		//��һ��ƥ��
		while (pScan < pEnd && *pScan == *pStart)
		{
			pScan++;
			pStart++;
			curLength++;
		}
		curDist = start - matchHead;

		//ȷ���ƥ��
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
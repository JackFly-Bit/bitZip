#include "BitZip.h"


#include <iostream>
using namespace std;
#include <assert.h>

// ������ṹ��
struct IntervalSolution
{
	ush code;               // ������
	uch extraBit;           // ��չ��
	ush interval[2];        // �������а������ٸ�����
};

/*************************************************************/
// ����������
static IntervalSolution distInterval[] = {
	{ 0, 0, { 1, 1 } },
	{ 1, 0, { 2, 2 } },
	{ 2, 0, { 3, 3 } },
	{ 3, 0, { 4, 4 } },
	{ 4, 1, { 5, 6 } },
	{ 5, 1, { 7, 8 } },
	{ 6, 2, { 9, 12 } },
	{ 7, 2, { 13, 16 } },
	{ 8, 3, { 17, 24 } },
	{ 9, 3, { 25, 32 } },
	{ 10, 4, { 33, 48 } },
	{ 11, 4, { 49, 64 } },
	{ 12, 5, { 65, 96 } },
	{ 13, 5, { 97, 128 } },
	{ 14, 6, { 129, 192 } },
	{ 15, 6, { 193, 256 } },
	{ 16, 7, { 257, 384 } },
	{ 17, 7, { 385, 512 } },
	{ 18, 8, { 513, 768 } },
	{ 19, 8, { 769, 1024 } },
	{ 20, 9, { 1025, 1536 } },
	{ 21, 9, { 1537, 2048 } },
	{ 22, 10, { 2049, 3072 } },
	{ 23, 10, { 3073, 4096 } },
	{ 24, 11, { 4097, 6144 } },
	{ 25, 11, { 6145, 8192 } },
	{ 26, 12, { 8193, 12288 } },
	{ 27, 12, { 12289, 16384 } },
	{ 28, 13, { 16385, 24576 } },
	{ 29, 13, { 24577, 32768 } }
};

// ����������
static IntervalSolution lengthInterval[] = {
	{ 257, 0, { 3, 3 } },
	{ 258, 0, { 4, 4 } },
	{ 259, 0, { 5, 5 } },
	{ 260, 0, { 6, 6 } },
	{ 261, 0, { 7, 7 } },
	{ 262, 0, { 8, 8 } },
	{ 263, 0, { 9, 9 } },
	{ 264, 0, { 10, 10 } },
	{ 265, 1, { 11, 12 } },
	{ 266, 1, { 13, 14 } },
	{ 267, 1, { 15, 16 } },
	{ 268, 1, { 17, 18 } },
	{ 269, 2, { 19, 22 } },
	{ 270, 2, { 23, 26 } },
	{ 271, 2, { 27, 30 } },
	{ 272, 2, { 31, 34 } },
	{ 273, 3, { 35, 42 } },
	{ 274, 3, { 43, 50 } },
	{ 275, 3, { 51, 58 } },
	{ 276, 3, { 59, 66 } },
	{ 277, 4, { 67, 82 } },
	{ 278, 4, { 83, 98 } },
	{ 279, 4, { 99, 114 } },
	{ 280, 4, { 115, 130 } },
	{ 281, 5, { 131, 162 } },
	{ 282, 5, { 163, 194 } },
	{ 283, 5, { 195, 226 } },
	{ 284, 5, { 227, 257 } },
	{ 285, 0, { 258, 258 } } };
/******************************************************************/

BitZip::BitZip()
: _pWin(new uch[2 * WSIZE])
, _ht(WSIZE)
, _isLast(false)
{
	_byteLenData.reserve(BUFF_SIZE);
	_distData.reserve(BUFF_SIZE);
	_flagData.reserve(BUFF_SIZE / 8);

	_byteLenInfo.resize(256 + 1 + 29);
	for (ush i = 0; i < _byteLenInfo.size(); ++i)
	{
		_byteLenInfo[i]._ch = i;
		_byteLenInfo[i]._appearCount = 0;
		_byteLenInfo[i]._len = 0;
		_byteLenInfo[i]._chCode = 0;
	}

	_distInfo.resize(30);
	for (ush i = 0; i < _distInfo.size(); ++i)
	{
		_distInfo[i]._ch = i;
		_distInfo[i]._appearCount = 0;
		_distInfo[i]._len = 0;
		_distInfo[i]._chCode = 0;
	}
}

BitZip::~BitZip()
{
	delete[] _pWin;
	_pWin = nullptr;
}

void BitZip::Deflate(const string& filePath)
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


	//��ѹ��������ļ���Դ�ļ�������ͬ
	string fileName;
	fileName = filePath.substr(0, filePath.rfind('.'));
	fileName += "bzp";
	FILE* fOut = fopen(fileName.c_str(), "wb");

	ush start = 0;
	uch ch = 0;
	uch bitCount = 0;

	while (lookahead)
	{
		ush curMatchLength = 0;
		ush curMatchDist = 0;

		//�������ַ�һ�飬����ϣͰ�н��в���
		_ht.Insert(hashAddr, _pWin[start + 2], start, matchHead);

		//�ڲ���֮��matchHead�Ὣ���һ������������һ��ƥ���λ�ô�����
		if (matchHead)
		{
			//���ƥ����
			curMatchLength = LongestMatch(matchHead, start, curMatchDist);//<����,����>
		}

		if (curMatchLength < MIN_MATCH)
		{
			//�������ֽ�֮ǰû�г��ֹ�
			//����ǰstartλ�õ��ֽ�ԭ�ⲻ����д�뵽ѹ���ļ�����
			//����LZ77�Ľ������������huffman����������ѹ��
			lookahead--;
			SaveLZ77Result(0, _pWin[start], ch, bitCount, lookahead);
			start++;
		}
		else
		{
			lookahead -= curMatchLength;

			//����<���ȣ�����>��д�뵽ѹ���ļ���
			//����LZ77�еĳ��Ⱦ����--����huffman����������ѹ��
			SaveLZ77Result(curMatchDist, curMatchLength-3, ch, bitCount, lookahead);

			//��Ҫ��ƥ�����������ֽ�һ������ϣ����в���
			curMatchLength -= 1; //��Ϊstartλ�ÿ�ʼ���ֽ��Ѿ�������
			while (curMatchLength)
			{
				++start;
				_ht.Insert(hashAddr, _pWin[start + 2], start, matchHead);
				curMatchLength--;
			}

			++start;
		}

		//lookahead�������л�������ʣ��Ĵ�ѹ���ֽڵĸ���
		if (lookahead <= MIN_LOOKAHEAD)
			FillDate(fIn, lookahead, start);
	}

	if (_byteLenData.size() > 0 && _byteLenData.size() < BUFF_SIZE)
	{
		//���һ�鲻��BUFF_SIZE,����û��ѹ��
		if (bitCount > 0 && bitCount < 8)
		{
			ch <<= (8 - bitCount);
			_flagData.push_back(ch);
		}

		_isLast = true;
		CompressBlock();
	}

	fclose(fIn);
	fclose(fOut);
}

void BitZip::FillDate(FILE* fIn, ulg& lookahead, ush& start)
{
	if (start > MAX_DIST)
	{
		//���Ҵ����е����ݰ��Ƶ���
		memcpy(_pWin, _pWin + WSIZE, WSIZE);
		start -= WSIZE;

		//ע�⣺���¹�ϣ��
		_ht.UpdateHashTable();

		//��Ҫ���Ҵ����в�������
		if (!feof(fIn))
			lookahead += fread(_pWin + WSIZE, 1, WSIZE, fIn);
	}
}

ush BitZip::LongestMatch(ush matchHead, ush start, ush& curMatchDist)
{
	ush maxLen = 0;
	uch maxMatchCount = 255;
	ush limit = start > MAX_DIST ? start - MAX_DIST : 0;
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
	} while (((matchHead = _ht.GetNext(matchHead)) > limit) && maxMatchCount--);

	return maxLen;
}

//LZ77ÿһ���Ľ����������ԭ�ַ� Ҳ�����ǳ��Ⱦ����
void BitZip::SaveLZ77Result(ush dist, uch length, uch& flag, uch& bitCount, const ulg lookahead)
{
	//���distΪ0����ʾlength��ԭ�ַ�
	//���dist������0�� <length, dist>�պù�����һ�����Ⱦ����
	flag <<= 1;
	_byteLenData.push_back(length);

	if (dist)
	{
		flag |= 1;
		//length��ʾ����
		_distData.push_back(dist);
	}

	++bitCount;
	if (8 == bitCount)
	{
		_flagData.push_back(flag);
		flag = 0;
		bitCount = 0;
	}

	//ע�⣺������ZIP�����ǵ�LZ77�������ļ�ѹ�����ֻ�У�Ȼ�󽫽�����ݽ���huffman��
	//		��Ϊbuffer��ǳ��󣬵����ַ������ر�࣬huffman����Ҷ�ӽڵ��ǳ���ƽ�������䳤��Ӱ��ѹ����
	//�ֿ�������ѹ��
	if(_byteLenData.size() == BUFF_SIZE)
	{
		if (lookahead == 0)
		{
			_isLast = true;
		}

		CompressBlock();
	}
}

void BitZip::CompressBlock()
{
	//1.ͳ��ÿ��Ԫ�س��ֵĴ���
	StatAppearCount();
	//2.����huffman��
	HuffmanTree<ElemInfo>byteLenTree;
	byteLenTree.CreatHuffmanTree(_byteLenInfo, ElemInfo());

	HuffmanTree<ElemInfo>distTree;
	distTree.CreatHuffmanTree(_distInfo, ElemInfo());

	//3.���ݷ�ʽhuffman�������ɱ���
	GenerateCodeLen(byteLenTree.GetRoot(), _byteLenInfo);
	GenerateCode(_byteLenInfo);

	GenerateCodeLen(distTree.GetRoot(), _distInfo);
	GenerateCode(_distInfo);

	//4.ѹ��

}

void BitZip::StatAppearCount()
{
	size_t index = 0;
	size_t distIdx = 0;
	uch flag = 0;
	ush bitCount = 0;
	for (ush i = 0; i < _byteLenData.size(); ++i)
	{
		if (0 == bitCount)
		{
			flag = _flagData[index++];
			bitCount = 8;
		}

		if (flag & 0x80)
		{
			//<���� ����>��
			//����--->�ҳ���Ӧ��������
			_byteLenInfo[GetIntervalCodeIndex(_byteLenData[i])]._appearCount++;
			_distInfo[GetIntervalCodeIndex(_distData[distIdx++])]._appearCount++;
		}
		else
		{
			_byteLenInfo[_byteLenData[i]]._appearCount++;
		}

		flag <<= 1;
		bitCount--;
	}
}

ush BitZip::GetIntervalCodeIndex(uch len)
{
	len += 3;
	uch size = sizeof(lengthInterval) / sizeof(lengthInterval[0]);
	for (uch i = 0; i < size; ++i)
	{
		if (len >= lengthInterval[i].interval[0] && len <= lengthInterval[i].interval[1])
		{
			return 257 + i;
		}
	}

	assert(false);
	return 0;
}

ush BitZip::GetIntervalCodeIndex(ush dist)
{
	uch size = sizeof(distInterval) / sizeof(distInterval[0]);
	for (uch i = 0; i < size; ++i)
	{
		if (dist >= distInterval[i].interval[0] && dist <= distInterval[i].interval[1])
		{
			return i;
		}
	}

	assert(false);
	return 0;
}

void BitZip::GenerateCodeLen(HuffmanTreeNode<ElemInfo>* root, vector<ElemInfo>& elemInfo)
{
	ush len = 0;
	GenerateCodeLen(root, elemInfo, len);
}

void BitZip::GenerateCodeLen(HuffmanTreeNode<ElemInfo>* root, vector<ElemInfo>& elemInfo, ush len)
{
	if (nullptr == root)
		return;

	if (nullptr == root->_left && nullptr == root->_right)
	{
		elemInfo[root->_weight._ch]._len = len;
		return;
	}

	++len;
	GenerateCodeLen(root->_left, elemInfo, len);
	GenerateCodeLen(root->_right, elemInfo, len);
}

void BitZip::GenerateCode(vector<ElemInfo>& codeInfo)
{
	//1.���ձ���λλ��Ϊ��һ�ؼ��֣�Ԫ�ش�СΪ�ڶ��ؼ���������
	vector<ElemInfo>temp(codeInfo);
	sort(temp.begin(), temp.end());

	//�ҵ���һ������λ��������0��Ԫ��
	ush index = 0;
	while (index < temp.size())
	{
		if (temp[index]._len)
			break;

		index++;
	}

	//2.���շ�ʽhuffman���Ĺ������������
	temp[index++]._chCode = 0;
	codeInfo[temp[index++]._ch]._chCode = 0;
	size_t elemCount = 1;
	while (index < temp.size())
	{
		if (temp[index]._len == temp[index - 1]._len)
		{
			temp[index]._chCode = temp[index - 1]._chCode + 1;
			codeInfo[temp[index]._ch]._chCode = temp[index - 1]._chCode + 1;
			elemCount++;
		}
		else
		{
			temp[index]._chCode = (temp[index - elemCount]._chCode + elemCount) << (temp[index]._len - temp[index - 1]._len);
			codeInfo[temp[index]._ch]._chCode = (temp[index - elemCount]._chCode + elemCount) << (temp[index]._len - temp[index - 1]._len);
			elemCount = 1;
		}

		++index;
	}
}

void BitZip::UNCompressLZ77(const string& filePath)
{
	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (nullptr == fIn)
	{
		cout << "��ѹ�����ļ�·��������" << endl;
		return;
	}
	//��ȡԴ�ļ��Ĵ�С
	fseek(fIn, 0 - sizeof(ulg), SEEK_END);
	ulg fileSize = 0;
	fread(&fileSize, sizeof(fileSize), 1, fIn);

	//��ȡ��ǵĴ�С
	size_t flagSize = 0;
	fseek(fIn, 0 - sizeof(fileSize)-sizeof(flagSize), SEEK_END);
	fread(&flagSize, sizeof(flagSize), 1, fIn);

	fseek(fIn, 0, SEEK_SET);

	FILE* fFlag = fopen(filePath.c_str(), "rb");
	fseek(fFlag, 0 - sizeof(fileSize)-sizeof(flagSize)-flagSize, SEEK_END);

	//��ѹ��
	FILE* fOut = fopen("222.txt", "wb");

	FILE* fRead = fopen("222.txt", "rb");
	uch ch = 0;
	uch bitCount = 0;
	size_t i = 0;
	uch chData = 0;
	ulg compressCount = 0;
	while (compressCount < fileSize)
	{
		if (0 == bitCount)
		{
			ch = fgetc(fFlag);
			bitCount = 8;
			i++;
		}
		//���ñ���λ��0����1
		if (ch & 0x80)
		{
			//���Ⱦ����
			ush matchLength = fgetc(fIn) + 3;
			ush matchDist = 0;
			fread(&matchDist, sizeof(matchDist), 1, fIn);

			//�м�:��ѹ�����������ݿ��ܻ��ڻ�������
			fflush(fOut);

			fseek(fRead, 0 - matchDist, SEEK_END);
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
			//ԭ�ַ�
			chData = fgetc(fIn);
			fputc(chData, fOut);
			compressCount++;
		}
		bitCount--;
		ch <<= 1;
	}

	fclose(fIn);
	fclose(fFlag);
	fclose(fOut);
	fclose(fRead);
}
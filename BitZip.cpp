#include "BitZip.h"


#include <iostream>
using namespace std;
#include <assert.h>
#include <map>

// 区间码结构体
struct IntervalSolution
{
	ush code;               // 区间编号
	uch extraBit;           // 扩展码
	ush interval[2];        // 改区间中包含多少个数字
};

/*************************************************************/
// 距离区间码
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

// 长度区间码
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


	//让压缩结果的文件与源文件名子相同
	string fileName;
	fileName = filePath.substr(0, filePath.rfind('.'));
	fileName += ".bzp";
	fOut = fopen(fileName.c_str(), "wb");

	ush start = 0;
	uch ch = 0;
	uch bitCount = 0;

	//LZ77
	while (lookahead)
	{
		ush curMatchLength = 0;
		ush curMatchDist = 0;

		//将三个字符一组，往哈希桶中进行插入
		_ht.Insert(hashAddr, _pWin[start + 2], start, matchHead);

		//在插入之后，matchHead会将查找缓冲区中最近的一个匹配的位置带出来
		if (matchHead)
		{
			//找最长匹配了
			curMatchLength = LongestMatch(matchHead, start, curMatchDist);//<长度,距离>
		}

		if (curMatchLength < MIN_MATCH)
		{
			//该三个字节之前没有出现过
			//将当前start位置的字节原封不动的写入到压缩文件单中
			//保存LZ77的结果，将来交给huffman树继续进行压缩
			lookahead--;
			SaveLZ77Result(0, _pWin[start], ch, bitCount, lookahead);
			start++;
		}
		else
		{
			lookahead -= curMatchLength;

			//将该<长度，距离>对写入到压缩文件中
			//保存LZ77中的长度距离对--交给huffman树继续进行压缩
			SaveLZ77Result(curMatchDist, curMatchLength-3, ch, bitCount, lookahead);

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

		//lookahead就是先行缓冲区中剩余的带压缩字节的个数
		if (lookahead <= MIN_LOOKAHEAD)
			FillDate(fIn, lookahead, start);
	}

	if (_byteLenData.size() > 0 && _byteLenData.size() < BUFF_SIZE)
	{
		//最后一块不足BUFF_SIZE,导致没有压缩
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
		//将右窗口中的数据搬移到左窗
		memcpy(_pWin, _pWin + WSIZE, WSIZE);
		start -= WSIZE;

		//注意：更新哈希表
		_ht.UpdateHashTable();

		//需要向右窗口中补充数据
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
	} while (((matchHead = _ht.GetNext(matchHead)) > limit) && maxMatchCount--);

	return maxLen;
}

//LZ77每一步的结果：可能是原字符 也可能是长度距离对
void BitZip::SaveLZ77Result(ush dist, uch length, uch& flag, uch& bitCount, const ulg lookahead)
{
	//如果dist为0，表示length是原字符
	//如果dist不等于0， <length, dist>刚好构成了一个长度距离对
	flag <<= 1;
	_byteLenData.push_back(length);

	if (dist)
	{
		flag |= 1;
		//length表示长度
		_distData.push_back(dist);
	}

	++bitCount;
	if (8 == bitCount)
	{
		_flagData.push_back(flag);
		flag = 0;
		bitCount = 0;
	}

	//注意：真正的ZIP并不是等LZ77将整个文件压缩完成只有，然后将结果数据交给huffman树
	//		因为buffer会非常大，导致字符种类特别多，huffman树的叶子节点会非常大，平均编码会变长，影响压缩率
	//分块来进行压缩
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
	//1.统计每个元素出现的次数
	StatAppearCount();
	//2.构建huffman树
	HuffmanTree<ElemInfo>byteLenTree;
	byteLenTree.CreatHuffmanTree(_byteLenInfo, ElemInfo());

	HuffmanTree<ElemInfo>distTree;
	distTree.CreatHuffmanTree(_distInfo, ElemInfo());

	//3.根据范式huffman树来生成编码
	GenerateCodeLen(byteLenTree.GetRoot(), _byteLenInfo);
	GenerateCode(_byteLenInfo);

	GenerateCodeLen(distTree.GetRoot(), _distInfo);
	GenerateCode(_distInfo);

	//4.保存解压缩需要用到的位长信息
	WriteCodeLen();

	//5.压缩
	//取原字符或长度、距离 找对应的编码来进行压缩
	uch flag = 0;
	uch bitCount = 0;
	ush flagIdx = 0;
	ush distIdx = 0;
	uch bitInfo = 0;
	uch bitInfoCount = 0;
	for (ush i = 0; i < _byteLenData.size(); i++)
	{
		if (0 == bitCount)
		{
			flag = _flagData[flagIdx++];
			bitCount = 8;
		}

		if (flag & 0x80)
		{
			// _byteLenDate[i]:长度
			// _distData[]:距离
			CompressLengthDist(_byteLenData[i], _distData[distIdx++], bitInfo, bitInfoCount);
		}
		else
		{
			//_byteLenData[i]:原字符
			CompressChar(_byteLenData[i], bitInfo, bitInfoCount);
		}

		flag <<= 1;
		bitCount--;
	}

	//上述for循环中压缩的是：LZ77中的有效数据
	//最后位置压缩一个256作为块的标记
	CompressChar(256, bitInfo, bitInfoCount);

	//注意：bitInfo中不一定刚好是8个比特位全部都有效
	if (bitInfoCount > 0 && bitInfoCount < 8)
	{
		bitInfo <<= (8 - bitInfoCount);
		fputc(bitInfo, fOut);
	}

	//清空：LZ77的结果buff
	_byteLenData.clear();
	_distData.clear();
	_flagData.clear();
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
			//<长度 距离>对
			//长度--->找出对应的区间码
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

	// 给块结束标记
	// 字节[0, 255]
	// 长度分组区间编码[257, 285]
	// 256---> 块结束标记
	_byteLenInfo[256]._appearCount = 1;
}

ush BitZip::GetIntervalCodeIndex(uch len)
{
	len += 3;
	uch size = sizeof(lengthInterval) / sizeof(lengthInterval[0]);
	for (uch i = 0; i < size; ++i)
	{
		if (len >= lengthInterval[i].interval[0] && len <= lengthInterval[i].interval[1])
		{
			return i + 257;
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
	//1.按照编码位位长为第一关键字，元素大小为第二关键字排升序
	vector<ElemInfo>temp(codeInfo);
	sort(temp.begin(), temp.end());

	//找到第一个编码位长不等于0的元素
	ush index = 0;
	while (index < temp.size())
	{
		if (temp[index]._len)
			break;

		index++;
	}

	//2.按照范式huffman树的规则来计算编码
	//temp[index++]._chCode = 0;
	codeInfo[temp[index++]._ch]._chCode = 0;
	size_t elemCount = 1;
	while (index < temp.size())
	{
		if (temp[index]._len == temp[index - 1]._len)
		{
			//temp[index]._chCode = temp[index - 1]._chCode + 1;
			codeInfo[temp[index]._ch]._chCode = codeInfo[temp[index - 1]._ch]._chCode + 1;
			elemCount++;
		}
		else
		{
			//temp[index]._chCode = (temp[index - elemCount]._chCode + elemCount) << (temp[index]._len - temp[index - 1]._len);
			codeInfo[temp[index]._ch]._chCode = (codeInfo[temp[index - elemCount]._ch]._chCode + elemCount) << (codeInfo[temp[index]._ch]._len - codeInfo[temp[index - 1]._ch]._len);
			elemCount = 1;
		}

		++index;
	}
}

//保存解压缩时需要用到的编码位长信息
void BitZip::WriteCodeLen()
{
	if (_isLast)
		fputc(1, fOut);
	else
		fputc(0, fOut);

	for (auto& e : _byteLenInfo)
	{
		fputc(e._len, fOut);
	}

	for (auto& e : _distInfo)
	{
		fputc(e._len, fOut);
	}
}

void BitZip::CompressChar(ush ch, uch& bitInfo, uch& bitInfoCount)
{
	//必须要找到ch的编码---—_byteLenInfo
	ulg chCode = _byteLenInfo[ch]._chCode; //比如:m--->"110" chCode:6
	ush codeLen = _byteLenInfo[ch]._len;

	Compress(chCode, codeLen, bitInfo, bitInfoCount);
}

void BitZip::Compress(ulg chCode, ush codeLen, uch& bitInfo, uch& bitInfoCount)
{
	//将ch的编码chCode往压缩文件中去写
	chCode <<= (64 - codeLen);

	for (ush i = 0; i < codeLen; ++i)
	{
		bitInfo <<= 1;
		if (chCode & 0x8000000000000000)
			bitInfo |= 1;

		bitInfoCount++;
		if (8 == bitInfoCount)
		{
			fputc(bitInfo, fOut);
			bitInfo = 0;
			bitInfoCount = 0;
		}

		chCode <<= 1;
	}
}

void BitZip::CompressLengthDist(uch length, ush dist, uch& bitInfo, uch& bitInfoCount)
{
	//1.先压缩长度
	//注意:长度对应的分组的编码+扩展码
	//假设:length=20
	//20属于269的分组 因此需要到_byteLenInfo中找257对应的编码
	size_t index = GetIntervalCodeIndex(length);
	ulg chCode = _byteLenInfo[index]._chCode;
	ush codeLen = _byteLenInfo[index]._len;
	Compress(chCode, codeLen, bitInfo, bitInfoCount);

	//压缩length所对应的额外的扩展码
	chCode = length+3 - lengthInterval[index-257].interval[0];
	codeLen = lengthInterval[index-257].extraBit;
	Compress(chCode, codeLen, bitInfo, bitInfoCount);
	//2.压缩距离
	//假设：距离是20 找20所对应的分组
	//{8,3,{17, 24}}, // 17:000 18:001 19:010 20:011 21:100 22:101 23:110 24:111
	index = GetIntervalCodeIndex(dist);
	chCode = _distInfo[distInterval[index].code]._chCode;
	codeLen = _distInfo[distInterval[index].code]._len;
	Compress(chCode, codeLen, bitInfo, bitInfoCount);

	//压缩dist所对应的额外的扩展码
	chCode = dist - distInterval[index].interval[0];
	codeLen = distInterval[index].extraBit;
	Compress(chCode, codeLen, bitInfo, bitInfoCount);

}

void BitZip::unDeflate(const string& filePath)
{
	if (filePath.substr(filePath.rfind(".") + 1) != "bzp")
	{
		cout << "压缩文件格式不对，无法解压缩" << endl;
		return;
	}

	FILE* fIn = fopen(filePath.c_str(), "rb");

	string fileName(filePath.substr(0, filePath.rfind(".")));
	fileName += "_d.txt";
	fOut = fopen(fileName.c_str(), "wb");

	uch ch = 0;
	uch bitCount = 0;
	while (true)
	{
		_isLast = fgetc(fIn);
		

		//for (ush i = 0; i < 286; i++)
		//{
		//	_distInfo[i]._ch = i;
		//	_distInfo[i]._len = fputc();
		//}

		// 1.获取编码位长
		GetDecodeLen(fIn);

		// 2.生成解码表
		vector<DecodeTable> byteLenDecTab;
		GenerateDecodeTab(_byteLenInfo, byteLenDecTab);

		vector<DecodeTable> distDecTab;
		GenerateDecodeTab(_distInfo, distDecTab);

		// 3.解码
		while (true)
		{
			ush data = UNCompressSymbol(fIn, _byteLenInfo, byteLenDecTab, ch, bitCount);
			if (data < 256)
			{
				//解压缩了一个原字符
				fputc(data, fOut);
				fflush(fOut);
			}
			else if (data == 256)
			{
				//该块解压缩结束
				break;
			}
			else
			{
				//data表示一个长度

				//解压缩距离
				ush dist = UNCompressSymbol(fIn, _distInfo, distDecTab, ch, bitCount);

				//以LZ77的方式还原结果了

			}
		}

		if (_isLast)
			break;
	}

	fclose(fIn);
	fclose(fOut);
}

void BitZip::GetDecodeLen(FILE* fIn)
{
	_byteLenInfo.clear();
	for (ush i = 0; i < 286; ++i)
	{
		uch len = fgetc(fIn);
		if (0 != len)
		{
			ElemInfo e;
			e._ch = i;
			e._len = len;
			_byteLenInfo.push_back(e);
		}
	}

	_distInfo.clear();
	for (ush i = 0; i < 30; ++i)
	{
		uch len = fgetc(fIn);
		if (0 != len)
		{
			ElemInfo e;
			e._ch = i;
			e._len = len;
			_distInfo.push_back(e);
		}
	}
}

void BitZip::GenerateDecodeTab(vector<ElemInfo>& codeInfo, vector<DecodeTable>& decTab)
{
	//1.统计相同位长字符个数
	map<uch, ush> m;
	for (auto& e : codeInfo)
		m[e._len]++;

	//2.对读取到的位长信息进行排序
	sort(codeInfo.begin(), codeInfo.end());

	size_t index = 0;
	for (auto e : m)
	{
		DecodeTable dec;
		dec._decodeLen = e.first;
		dec._lenCount = e.second;
		if (0 == index)
		{
			dec._code = 0;
			dec._charIndex = 0;
		}
		else
		{
			DecodeTable& prev = decTab.back();
			dec._charIndex = prev._charIndex + prev._lenCount;
			dec._code = (prev._code + prev._lenCount) << (dec._decodeLen - prev._decodeLen);
		}

		decTab.push_back(dec);
		index++;
	}
}

ush BitZip::UNCompressSymbol(FILE* fIn, vector<ElemInfo>& codeInfo, vector<DecodeTable>& decTable, uch& ch, uch& bitCount)
{
	ush i = 0;
	ush codeLen = decTable[0]._decodeLen;
	ush code = 0;
	while (codeLen--)
	{
		GetNextBit(fIn, code, ch, bitCount);
	}

	ush num = 0;
	while ((num = code - decTable[i]._code) >= decTable[i]._lenCount)
	{
		i++;
		//中间可能会有一些长度没有出现，比如：
		//len:7 ---> code:126
		//len:9 ---> code:508
		//次数需要一次性读取两位，但是i只加一次
		ush lenGap = decTable[i]._decodeLen - decTable[i - 1]._decodeLen;
		while (lenGap--)
		{
			GetNextBit(fIn, code, ch, bitCount);
		}
	}

	//找出扩展的比特位
	num += decTable[i]._charIndex;
	return codeInfo[num]._ch;
}

//每次获取一个比特位，放到code中
void BitZip::GetNextBit(FILE* fIn, ush& code, uch& ch, uch& bitCount)
{
	if (0 == bitCount)
	{
		ch = fgetc(fIn);
		bitCount = 8;
	}

	code <<= 1;
	if (ch & 0x80)
		code |= 1;

	ch <<= 1;
}
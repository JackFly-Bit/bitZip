#include "Common.h"
#include "HuffmanTree.hpp"
#include "HashTable.h"


struct ElemInfo
{
	ush _ch;//字符
	ulg _appearCount;//表示字符出现次数
	ush _len;
	ulg _chCode;

	ElemInfo(ulg appCount = 0)
		: _appearCount(appCount)
	{ }

	ElemInfo operator+(const ElemInfo& c)
	{
		return ElemInfo(_appearCount + c._appearCount);
	}

	bool operator>(const ElemInfo& c)const
	{
		return _appearCount > c._appearCount;
	}

	bool operator==(const ElemInfo& c)const
	{
		return _appearCount == c._appearCount;
	}

	bool operator!=(const ElemInfo& c)const
	{
		return _appearCount == c._appearCount;
	}

	bool operator<(const ElemInfo& c)const
	{
		if ((_len < c._len) || 
			(_len == c._len && _ch < c._ch))
		{
			return true;
		}

		return false;
	}
};

class BitZip
{
public:
	BitZip();
	~BitZip();

	void Deflate(const string& filePath);
	void UNCompressLZ77(const string& filePath);

private:
	void SaveLZ77Result(ush dist, uch length, uch& flag, uch& bitCount, const ulg lookahead);
	ush LongestMatch(ush matchHead, ush start, ush& curMatchDist);
	void FillDate(FILE* fIn, ulg& lookahead, ush& start);

	/////////////////////////////////////
	void CompressBlock();
	void StatAppearCount();
	ush GetIntervalCodeIndex(uch len);
	ush GetIntervalCodeIndex(ush dist);
	void GenerateCodeLen(HuffmanTreeNode<ElemInfo>* root, vector<ElemInfo>& elemInfo);
	void GenerateCodeLen(HuffmanTreeNode<ElemInfo>* root, vector<ElemInfo>& elemInfo, ush len);
	void GenerateCode(vector<ElemInfo>& codeInfo);
private:
	uch* _pWin;
	HashTable _ht;

	/////////////////////////////////////
	//用来保存LZ77的结果
	vector<uch> _byteLenData;
	vector<ush> _distData;
	vector<uch> _flagData;

	bool _isLast; // 是否为最后一个块

	///////////////////////////////////////
	//用来构建huffman树以及编码
	vector<ElemInfo> _byteLenInfo;
	vector<ElemInfo> _distInfo;


	const static ush MIN_LOOKAHEAD = MIN_MATCH + MAX_MATCH + 1;
	const static ush MAX_DIST = WSIZE - MIN_LOOKAHEAD;
	const static ush BUFF_SIZE = 0x8000;
};
#pragma once

typedef unsigned long long ulg;

#include <string>
#include <vector>

#include "HuffmanTree.hpp"

typedef unsigned char uch;
struct CharInfo
{
	uch _ch;//字符
	ulg _appearCount;//表示字符出现次数
	std::string _strCode;//字符对应的编码

	CharInfo(ulg appCount = 0)
		: _appearCount(appCount)
	{ }

	CharInfo operator+(const CharInfo& c)
	{
		return CharInfo(_appearCount + c._appearCount);
	}

	bool operator>(const CharInfo& c)const
	{
		return _appearCount > c._appearCount;
	}
	bool operator==(const CharInfo& c)const
	{
		return _appearCount == c._appearCount;
	}
	bool operator!=(const CharInfo& c)const
	{
		return _appearCount == c._appearCount;
	}
};

class HTCompress
{
public:
	HTCompress();
	void CompressFile(const std::string& filePath);
	void UNCompressFile(const std::string& filePath);

	void WriteHeadInfo(FILE* fOut, const std::string& filePath);
private:
	void GeneteCode(HuffmanTreeNode<CharInfo>* root);

	void GetLine(FILE* fIn, std::string& s);
private:
	//数据在文件中都是以字节方式保存的
	std::vector<CharInfo> _charInfo;
};
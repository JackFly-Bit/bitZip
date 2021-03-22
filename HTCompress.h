#pragma once

typedef unsigned long long ulg;

#include <string>
#include <vector>

#include "HuffmanTree.hpp"

struct CharInfo
{
	char _ch;
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
};

class HTCompress
{
public:
	HTCompress();
	void CompressFile(const std::string& filePath);
	void UNCompressFile(const std::string& filePath);

private:
	void GeneteCode(HuffmanTreeNode<CharInfo>* root);
private:
	//数据在文件中都是以字节方式保存的
	std::vector<CharInfo> _charInfo;
};
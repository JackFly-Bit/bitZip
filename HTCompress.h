#pragma once

typedef unsigned long long ulg;

#include <string>
#include <vector>

#include "HuffmanTree.hpp"

struct CharInfo
{
	char _ch;
	ulg _appearCount;//��ʾ�ַ����ִ���
	std::string _strCode;//�ַ���Ӧ�ı���

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
	//�������ļ��ж������ֽڷ�ʽ�����
	std::vector<CharInfo> _charInfo;
};
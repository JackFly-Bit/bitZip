#include "HTCompress.h"

#include <iostream>
using namespace std;

HTCompress::HTCompress()
{
	_charInfo.resize(256);
	for (size_t i = 0; i < 256; ++i)
	{
		_charInfo[i]._ch = i;
		_charInfo[i]._appearCount = 0;
	}
}
void HTCompress::CompressFile(const std::string& filePath)
{
	FILE* fIn = fopen(filePath.c_str(), "r");
	if (nullptr == fIn)
	{
		cout << "待压缩文件路径出错" << endl;
		return;
	}

	//1.统计文件中每个字符出现的次数
	char readBuff[1024];
	while (true)
	{
		size_t rdSize = fread(readBuff, 1, 1024, fIn);
		if (0 == rdSize)
			break;
		for (size_t i = 0; i < rdSize; ++i)
		{
			_charInfo[readBuff[i]]._appearCount++;
		}
	}

	//2.以charInfo作为权值去创建huffman树
	HuffmanTree<CharInfo> ht;
	ht.CreatHuffmanTree(_charInfo);

	//3.再获取每个字符对应的huffman编码
	GeneteCode(ht.GetRoot());

	fclose(fIn);
}

//先走到叶子节点的位置，然后顺着叶子节点向上进行编码的获取
void HTCompress::GeneteCode(HuffmanTreeNode<CharInfo>* root)
{
	if (nullptr == root)
		return;

	GeneteCode(root->_left);
	GeneteCode(root->_right);

	if (nullptr == root->_left && nullptr == root->_right)
	{
		//root是一个叶子节点

		HuffmanTreeNode<CharInfo>* cur = root;
		HuffmanTreeNode<CharInfo>* parent = cur->_parent;

		string& strCode = _charInfo[cur->_weight._ch]._strCode;
		while (parent)
		{
			if (cur == parent->_left)
				strCode += '0';
			else
				strCode += '1';

			cur = parent;
			parent = cur->_parent;
		}

		reverse(strCode.begin(), strCode.end());
	}
}
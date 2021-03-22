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
		cout << "��ѹ���ļ�·������" << endl;
		return;
	}

	//1.ͳ���ļ���ÿ���ַ����ֵĴ���
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

	//2.��charInfo��ΪȨֵȥ����huffman��
	HuffmanTree<CharInfo> ht;
	ht.CreatHuffmanTree(_charInfo);

	//3.�ٻ�ȡÿ���ַ���Ӧ��huffman����
	GeneteCode(ht.GetRoot());

	fclose(fIn);
}

//���ߵ�Ҷ�ӽڵ��λ�ã�Ȼ��˳��Ҷ�ӽڵ����Ͻ��б���Ļ�ȡ
void HTCompress::GeneteCode(HuffmanTreeNode<CharInfo>* root)
{
	if (nullptr == root)
		return;

	GeneteCode(root->_left);
	GeneteCode(root->_right);

	if (nullptr == root->_left && nullptr == root->_right)
	{
		//root��һ��Ҷ�ӽڵ�

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
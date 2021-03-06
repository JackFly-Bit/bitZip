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
	//注意：为什么要以二进制格式打开文件
	//因为：压缩的时候，压缩文件可以是文本文件，也可以是二进制格式的文件
	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (nullptr == fIn)
	{
		cout << "待压缩文件路径出错" << endl;
		return;
	}
	//1.统计文件中每个字符出现的次数
	uch readBuff[1024];
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
	//2.以ChInfo作为权值去创建huffman树
	HuffmanTree<CharInfo> ht;
	ht.CreatHuffmanTree(_charInfo, CharInfo(0));
	//3.再获取每个字符对应的huffman编码
	GeneteCode(ht.GetRoot());
	//将文件指针恢复到起始位置
	rewind(fIn);
	//用来写压缩结果的文件
	FILE* fOut = fopen("compressResult.txt", "wb");

	WriteHeadInfo(fOut, filePath);
	//4.用获取到的编码对源文件中的每个字符重新进改写
	uch chData = 0;
	uch bitCount = 0;
	while (true)
	{
		size_t rdSize = fread(readBuff, 1, 1024, fIn);
		if (0 == rdSize)
			break;
		for (size_t i = 0; i < rdSize; ++i)
		{
			//找该字节对应的编码
			string& strCode = _charInfo[readBuff[i]]._strCode;
			//将该编码中每个二进制的比特位放置到一个字节中
			//A--->100 B--->101 C--->11 D--->0
			for (size_t j = 0; j < strCode.size(); ++j)
			{
				chData <<= 1;
				if ('1' == strCode[j])
					chData |= 1;
				bitCount++;
				if (8 == bitCount)
				{
					//将该字节写入到压缩文件中
					fputc(chData, fOut);
					bitCount = 0;
					chData = 0;
				}
			}
		}
	}
	//如果chData中右效比特位不够8个时，是没有写入到压缩文件中的
	/*
	ADDDD
	最后一个字节一定要注意：假设bitCount = 7 chData:0100 0000
	该字节的低7个比特位是有效的压缩比特流数据

	写该字节时必须要将低7个有效的比特位移动到高位，否则解压缩的时候会有影响
	*/
	if (bitCount > 0 && bitCount < 8)
	{
		chData <<= (8 - bitCount);
		fputc(chData, fOut);
	}
	fclose(fIn);
	fclose(fOut);
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

void HTCompress::WriteHeadInfo(FILE* fOut, const string& filePath)
{
	//1.源文件后缀
	string filePostFix = filePath.substr(filePath.find('.'));
	filePostFix += "\n";
	//2.字节，出现次数总行数
	size_t szCount = 0;
	//3.字节，出现次数的信息--->每条占一行
	string chAppear;
	for (size_t i = 0; i < 256; ++i)
	{
		if (0 != _charInfo[i]._appearCount)
		{
			chAppear += _charInfo[i]._ch;
			chAppear += ",";
			chAppear += to_string(_charInfo[i]._appearCount);
			chAppear += "\n";
			szCount++;
		}
	}
	fwrite(filePostFix.c_str(), 1, filePostFix.size(), fOut);

	string strCount;
	strCount = to_string(szCount);
	strCount += "\n";
	fwrite(strCount.c_str(), 1, strCount.size(), fOut);

	fwrite(chAppear.c_str(), 1, chAppear.size(), fOut);
}

void HTCompress::UNCompressFile(const std::string& filePath)
{
	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (nullptr == fIn)
	{
		cout << "压缩文件路径有问题的" << endl;
		return;
	}
	//源文件的名字应该也要保存在压缩结果中---
	string filename("2");
	string s;
	GetLine(fIn, s);
	filename += s;
	//获取字节次数总的行数
	s = "";
	GetLine(fIn, s);
	size_t szCount = atoi(s.c_str());

	//读取字节次数的每行的信息
	for (size_t i = 0; i < szCount; ++i)
	{
		s = "";
		//注意：对于换行需要单独来进行处理
		GetLine(fIn, s);
		if ("" == s)
		{
			s += "\n";
			GetLine(fIn, s);
		}

		_charInfo[(uch)s[0]]._appearCount = atoi(s.c_str() + 2);
	}

	//还原huffman树
	HuffmanTree<CharInfo>ht;
	ht.CreatHuffmanTree(_charInfo, CharInfo(0));

	//解压缩
	FILE* fOut = fopen(filename.c_str(), "wb");

	uch readBuff[1024];
	HuffmanTreeNode<CharInfo>* cur = ht.GetRoot();
	uch bitCount = 8;
	uch chBit;
	ulg fileSize = cur->_weight._appearCount;

	while (true)
	{
		size_t rdSize = fread(readBuff, 1, 1024, fIn);
		if (0 == rdSize)
			break;
		//将rdBuff中每个字节逐比特位来进行解析
		for (size_t i = 0; i < rdSize; ++i)
		{
			//将readBuff[i]中保存字节的8个比特位从高往低逐个来进行检测
			bitCount = 8;
			chBit = readBuff[i];
			while (bitCount)
			{
				if (chBit & 0x80)
				{
					cur = cur->_right;
				}
				else
				{
					cur = cur->_left;
				}

				if (nullptr == cur->_left && nullptr == cur->_right)
				{
					//解压缩出来了一个字符
					fputc(cur->_weight._ch, fOut);
					cur = ht.GetRoot();

					//处理最后一个字节可能不够8个比特位的情况
					fileSize--;
					if (0 == fileSize)
						break;
				}
				bitCount--;
				chBit <<= 1;
			}
		}
	}
	fclose(fIn);
	fclose(fOut);
}

void HTCompress::GetLine(FILE* fIn, std::string& s)
{
	uch ch;
	while (!feof(fIn))
	{
		ch = fgetc(fIn);
		if ('\n' == ch)
			break;
		s += ch;
	}
}
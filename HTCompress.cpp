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
	//ע�⣺ΪʲôҪ�Զ����Ƹ�ʽ���ļ�
	//��Ϊ��ѹ����ʱ��ѹ���ļ��������ı��ļ���Ҳ�����Ƕ����Ƹ�ʽ���ļ�
	FILE* fIn = fopen(filePath.c_str(), "rb");
	if (nullptr == fIn)
	{
		cout << "��ѹ���ļ�·������" << endl;
		return;
	}
	//1.ͳ���ļ���ÿ���ַ����ֵĴ���
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
	//2.��ChInfo��ΪȨֵȥ����huffman��
	HuffmanTree<CharInfo> ht;
	ht.CreatHuffmanTree(_charInfo, CharInfo(0));
	//3.�ٻ�ȡÿ���ַ���Ӧ��huffman����
	GeneteCode(ht.GetRoot());
	//���ļ�ָ��ָ�����ʼλ��
	rewind(fIn);
	//����дѹ��������ļ�
	FILE* fOut = fopen("compressResult.txt", "wb");

	WriteHeadInfo(fOut, filePath);
	//4.�û�ȡ���ı����Դ�ļ��е�ÿ���ַ����½���д
	uch chData = 0;
	uch bitCount = 0;
	while (true)
	{
		size_t rdSize = fread(readBuff, 1, 1024, fIn);
		if (0 == rdSize)
			break;
		for (size_t i = 0; i < rdSize; ++i)
		{
			//�Ҹ��ֽڶ�Ӧ�ı���
			string& strCode = _charInfo[readBuff[i]]._strCode;
			//���ñ�����ÿ�������Ƶı���λ���õ�һ���ֽ���
			//A--->100 B--->101 C--->11 D--->0
			for (size_t j = 0; j < strCode.size(); ++j)
			{
				chData <<= 1;
				if ('1' == strCode[j])
					chData |= 1;
				bitCount++;
				if (8 == bitCount)
				{
					//�����ֽ�д�뵽ѹ���ļ���
					fputc(chData, fOut);
					bitCount = 0;
					chData = 0;
				}
			}
		}
	}
	//���chData����Ч����λ����8��ʱ����û��д�뵽ѹ���ļ��е�
	/*
	ADDDD
	���һ���ֽ�һ��Ҫע�⣺����bitCount = 7 chData:0100 0000
	���ֽڵĵ�7������λ����Ч��ѹ������������

	д���ֽ�ʱ����Ҫ����7����Ч�ı���λ�ƶ�����λ�������ѹ����ʱ�����Ӱ��
	*/
	if (bitCount > 0 && bitCount < 8)
	{
		chData <<= (8 - bitCount);
		fputc(chData, fOut);
	}
	fclose(fIn);
	fclose(fOut);
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

void HTCompress::WriteHeadInfo(FILE* fOut, const string& filePath)
{
	//1.Դ�ļ���׺
	string filePostFix = filePath.substr(filePath.find('.'));
	filePostFix += "\n";
	//2.�ֽڣ����ִ���������
	size_t szCount = 0;
	//3.�ֽڣ����ִ�������Ϣ--->ÿ��ռһ��
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
		cout << "ѹ���ļ�·���������" << endl;
		return;
	}
	//Դ�ļ�������Ӧ��ҲҪ������ѹ�������---
	string filename("2");
	string s;
	GetLine(fIn, s);
	filename += s;
	//��ȡ�ֽڴ����ܵ�����
	s = "";
	GetLine(fIn, s);
	size_t szCount = atoi(s.c_str());

	//��ȡ�ֽڴ�����ÿ�е���Ϣ
	for (size_t i = 0; i < szCount; ++i)
	{
		s = "";
		//ע�⣺���ڻ�����Ҫ���������д���
		GetLine(fIn, s);
		if ("" == s)
		{
			s += "\n";
			GetLine(fIn, s);
		}

		_charInfo[(uch)s[0]]._appearCount = atoi(s.c_str() + 2);
	}

	//��ԭhuffman��
	HuffmanTree<CharInfo>ht;
	ht.CreatHuffmanTree(_charInfo, CharInfo(0));

	//��ѹ��
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
		//��rdBuff��ÿ���ֽ������λ�����н���
		for (size_t i = 0; i < rdSize; ++i)
		{
			//��readBuff[i]�б����ֽڵ�8������λ�Ӹ�������������м��
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
					//��ѹ��������һ���ַ�
					fputc(cur->_weight._ch, fOut);
					cur = ht.GetRoot();

					//�������һ���ֽڿ��ܲ���8������λ�����
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
#pragma once
#include "Common.h"

class HashTable
{
public:
	HashTable(ush size);
	~HashTable();

	void Insert(ush& hashAddr, uch ch, ush pos, ush& matchHead);
	ush GetNext(ush& matchHead);

private:
	void HashFunc(ush& hashAddr, ush ch);
	ush H_SHIFT();

private:
	ush* _prev;
	ush* _head;

	//��ϣͰ�ĸ���Ϊ2^15
	const static ush HASH_BITS = 15;
	//��ϣ��Ĵ�С
	const static ush HASH_SIZE = (1 << HASH_BITS);
	//��ϣ���룺��Ҫ�����ǽ��Ҵ������������ʱ���������¹�ϣ��������
	const static ush HASH_MASK = HASH_SIZE - 1;
};
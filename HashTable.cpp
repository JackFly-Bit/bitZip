#include "HashTable.h"

HashTable::HashTable(ush size)
: _prev(new ush[2*size])
, _head(_prev + size)
{}

HashTable::~HashTable()
{
	delete[] _prev;
	_prev = nullptr;
	_head = nullptr;
}

//hashAddr:上一个字符串计算出的哈希地址
//ch:当前字符
//本次的哈希地址是前一次哈希地址基础上，再结合当前字符ch计算出来的
//HASH_MASK为WSIZE-1,&上掩码主要是为了防止哈希地址越界
void HashTable::HashFunc(ush& hashAddr, ush ch)
{
	hashAddr = (((hashAddr) << H_SHIFT()) ^ (ch)) & HASH_MASK;
}

ush HashTable::H_SHIFT()
{
	return (HASH_BITS + MIN_MATCH - 1) / MIN_MATCH;
}

//hashAddr:前三个字节的哈希地址，当前三个字节的哈希地址要根据前三个字节进行计算
//ch:表示当前三个字节中的第三个
//pos:表示 三个字节中首字节再窗口中的下标
//matchHead:表示在哈希表中找到的匹配链的头
void HashTable::Insert(ush& hashAddr, uch ch, ush pos, ush& matchHead)
{
	HashFunc(hashAddr, ch);

	//插入并带出匹配链的头
	matchHead = _head[hashAddr];
	_prev[pos&HASH_MASK] = _head[hashAddr];
	_head[hashAddr] = pos;
}

ush HashTable::GetNext(ush& matchHead)
{
	return _prev[matchHead & HASH_MASK];
}
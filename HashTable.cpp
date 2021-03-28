#include "HashTable.h"

HashTable::HashTable(ush size)
: _prev(new ush[2*size])
, _head(_prev + size)
{
	memset(_prev, 0, 2 * size*sizeof(ush));
}

HashTable::~HashTable()
{
	delete[] _prev;
	_prev = nullptr;
	_head = nullptr;
}

//hashAddr:��һ���ַ���������Ĺ�ϣ��ַ
//ch:��ǰ�ַ�
//���εĹ�ϣ��ַ��ǰһ�ι�ϣ��ַ�����ϣ��ٽ�ϵ�ǰ�ַ�ch���������
//HASH_MASKΪWSIZE-1,&��������Ҫ��Ϊ�˷�ֹ��ϣ��ַԽ��
void HashTable::HashFunc(ush& hashAddr, ush ch)
{
	hashAddr = (((hashAddr) << H_SHIFT()) ^ (ch)) & HASH_MASK;
}

ush HashTable::H_SHIFT()
{
	return (HASH_BITS + MIN_MATCH - 1) / MIN_MATCH;
}

//hashAddr:ǰ�����ֽڵĹ�ϣ��ַ����ǰ�����ֽڵĹ�ϣ��ַҪ����ǰ�����ֽڽ��м���
//ch:��ʾ��ǰ�����ֽ��еĵ�����
//pos:��ʾ �����ֽ������ֽ��ٴ����е��±�
//matchHead:��ʾ�ڹ�ϣ�����ҵ���ƥ������ͷ
void HashTable::Insert(ush& hashAddr, uch ch, ush pos, ush& matchHead)
{
	HashFunc(hashAddr, ch);

	//���벢����ƥ������ͷ
	matchHead = _head[hashAddr];
	_prev[pos&HASH_MASK] = _head[hashAddr];
	_head[hashAddr] = pos;
}

ush HashTable::GetNext(ush& matchHead)
{
	return _prev[matchHead & HASH_MASK];
}

//�����л�������ʣ��Ĵ�ѹ�����ֽ���С��MIN_LOOKAHEADʱ��
//��Ҫ���Ҵ����е����ݰ��Ƶ��󴰣��󴰿ڵ����ݶ�����
//��ϣ������������±����Ҫ����
void HashTable::UpdateHashTable()
{
	for (ush i = 0; i < HASH_SIZE; ++i)
	{
		if (_head[i] <= WSIZE)
			_head[i] = 0;
		else
			_head[i] -= WSIZE;
	}

	for (ush i = 0; i < HASH_SIZE; ++i)
	{
		if (_prev[i] <= WSIZE)
			_prev[i] = 0;
		else
			_prev[i] -= WSIZE;
	}
}
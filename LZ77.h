#include "Common.h"

#include "HashTable.h"
class LZ77
{
public:
	LZ77();
	~LZ77();

	void CompressLZ77(const string& filePath);
	void UNCompressLZ77(const string& filePath);
private:
	ush LongestMatch(ush matchHead, ush start, ush& curMatchDist);
private:
	uch* _pWin;

	HashTable _ht;
};
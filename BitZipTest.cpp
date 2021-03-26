#include "HTCompress.h"

#include <iostream>
using namespace std;

//void TestFile()
//{
//	FILE* pf = fopen("111", "wb");
//
//	fputc('1', pf);
//	fputc('2', pf);
//	fputc('3', pf);
//	fputc(0xff, pf);
//	fputc('4', pf);
//	fputc('5', pf);
//	fputc('6', pf);
//
//	fclose(pf);
//
//	pf = fopen("111", "r");
//	
//	char ch;
//	while ((ch = fgetc(pf)) != EOF)
//	{
//		cout << ch << endl;
//	}
//}

#if 0
int main()
{
	//TestFile();
	char s[] = "ÄãºÃ";
	HTCompress htc;
	htc.CompressFile("1.png");
	htc.UNCompressFile("compressResult.txt");
	return 0;
}
#endif

#include "LZ77.h"
int main()
{
	LZ77 lz;
	lz.CompressLZ77("1.txt");
	lz.UNCompressLZ77("111.txt");
	return 0;
}
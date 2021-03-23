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
int main()
{
	//TestFile();
	char s[] = "ÄãºÃ";
	HTCompress htc;
	htc.CompressFile("1.png");
	htc.UNCompressFile("compressResult.txt");
	return 0;
}
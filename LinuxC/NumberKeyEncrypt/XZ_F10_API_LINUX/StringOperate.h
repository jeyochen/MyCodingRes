#ifndef _SUNSON_STRING_OPERATION__
#define _SUNSON_STRING_OPERATION__

//char型转换成unsigned char型
int Char_unChar(const char *strSource,unsigned char *strDes);

//unsigned char字符类型  转换成 char 类型
int unChar_Char(const unsigned char *strSource,char *strDes);

//去掉字符串的空格
void TrimString(char *str);

//十六进制字符串转换成ASCII格式数据
int HexStr_ASCIIStr(const unsigned char *strSource, unsigned char *strDes);

//ASII格式数据转换成十六进制字符串 （一个字节拆分成两个字节—拆分）
void ASCIIStr_HexStr(const unsigned char *strSource,unsigned char *strDes,int len);

//将一个字节ASII数据转换成十六进制数据
void ASCII_Hex(const unsigned char chSrc, unsigned char *strDes);

//把十六进制的字符串转换成对应的整型数据
int HexStr_Int(unsigned char *HexData);
#endif


#ifndef _SUNSON_STRING_OPERATION__
#define _SUNSON_STRING_OPERATION__

//char��ת����unsigned char��
int Char_unChar(const char *strSource,unsigned char *strDes);

//unsigned char�ַ�����  ת���� char ����
int unChar_Char(const unsigned char *strSource,char *strDes);

//ȥ���ַ����Ŀո�
void TrimString(char *str);

//ʮ�������ַ���ת����ASCII��ʽ����
int HexStr_ASCIIStr(const unsigned char *strSource, unsigned char *strDes);

//ASII��ʽ����ת����ʮ�������ַ��� ��һ���ֽڲ�ֳ������ֽڡ���֣�
void ASCIIStr_HexStr(const unsigned char *strSource,unsigned char *strDes,int len);

//��һ���ֽ�ASII����ת����ʮ����������
void ASCII_Hex(const unsigned char chSrc, unsigned char *strDes);

//��ʮ�����Ƶ��ַ���ת���ɶ�Ӧ����������
int HexStr_Int(unsigned char *HexData);
#endif


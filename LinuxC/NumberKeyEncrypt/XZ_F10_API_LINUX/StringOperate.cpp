//�ַ������
#include "StringOperate.h"
#include <string>
#include <string.h>

/*********************************************************************
�ַ������غ���
*********************************************************************/
/*********************************************************************
* ������ƣ�Char_unChar
* ����������unsigned char����ת����char���� 
* �������char *strSource  
* �������unsigned char *strDes��
* �� �� ֵ������0���ɹ���С�ڵ���0ʧ�� ������ֵ����strSource���ȣ�
* ����˵������
* ������ݣ�"12345" to 0x31 0x32 0x33 0x34 0x35
* ԭ������Str_Array_Signal
**********************************************************************/
int Char_unChar(const char *strSource,unsigned char *strDes)
{
	int i;
	int length;
	length = strlen(strSource);
	for(i=0; i<length; i++)
	{
		strDes[i] = strSource[i];
	}
	return i;
}

/*********************************************************************
* ������ƣ�unChar_Char
* ����������Char����ת����unsigned char���� ����ֵΪҪת���ַ�ĳ���
* �������unsigned char *strSource  
* �������char *strDes
* �� �� ֵ������0���ɹ���С�ڵ���0ʧ�� ������ֵ����strSource���ȣ�
* ����˵������
* ������ݣ�0x31 0x32 0x33 0x34 0x35 to "12345"
* ԭ������Array_String  
**********************************************************************/
int unChar_Char(const unsigned char *strSource,char *strDes)
{
	int i;
	int length = 0;
	length = strlen((char *)strSource);
	for(i=0; i<length; i++)
	{
		strDes[i] = strSource[i];
	}
	return i;
}

/*********************************************************************
* ������ƣ�TrimString
* ����������ȥ���ַ��еĿո�
* �������char *strSourceDes
* �������char *strSourceDes
* �� �� ֵ����
* ����˵������
* ������ݣ�" 12 2ab dfe ee " to "122abdfeee"
* ԭ������chomp
**********************************************************************/
void TrimString(char *str)
{
	int i = 0;
	int j = 0;
	while((*(str + i) = *(str + j++)) != '\0')
	{
		if((*(str + i) != ' ') && (*(str + i) != '\r') && (*(str + i) != '\n'))
		{
			i++;
		}
	}
}

/*********************************************************************
* ������ƣ�HexStr_ASCIIStr
* ����������ʮ������ַ�ת����ASCII��ʽ��� �������ֽںϳ�һ���ֽڡ�ѹ����
* �������unsigned char *strSource 
* �������unsigned char *strDes
* �� �� ֵ��<=0,ʧ�ܣ�>0�ɹ�������ֵΪת�����ַ�ĳ��ȣ�
* ����˵������
* ������ݣ�"3132333435363738" to 0x310x320x330x340x350x360x370x38 �� "12345678"
* ԭ������HexToStr   STR_ARRY
**********************************************************************/
int HexStr_ASCIIStr(const unsigned char *strSource, unsigned char *strDes)    
{
	int i;
	unsigned char H_BYTE; //��λ
	unsigned char L_BYTE; //��λ
	int length;
	int k;
	int j;
	//char str[500]="";
	length=strlen((char *)strSource); //��ȡ�����ַ�ĳ���
	char *str = new char[length+1];
	memset(str,0,length+1);
	k=0;
	for(i=0;i<length;i++) //ȥ���ַ��еĿո�
	{
		if((strSource[i]!=' ') && (strSource[i]!= 0x0D) && (strSource[i] != '\n'))
		{
			str[k]=strSource[i];
			k++;
		}
	}


	length = strlen(str);
	length = length/2;
	k=0;	
	j=0;
	for(i=0;i<length;i++)
	{
		H_BYTE=str[j];
		j++;
		L_BYTE=str[j];
		j++;
		
		if((H_BYTE>=0x30) && (H_BYTE<=0x39))
		{
			H_BYTE=(H_BYTE<<4) & 0xf0;;
		}
		else if((H_BYTE>='A') && (H_BYTE <='F'))
		{
			H_BYTE=((H_BYTE-0x37)<<4) & 0xf0 ;
		}
		else if((H_BYTE>='a') && (H_BYTE<='f'))
		{
			H_BYTE=((H_BYTE-0x57)<<4) & 0xf0 ;
		}
		else 
			H_BYTE=0;
		
		if((L_BYTE>=0x30) && (L_BYTE<=0x39))
		{
			L_BYTE=L_BYTE & 0x0F;;
		}
		else if((L_BYTE>='A') && (L_BYTE <='F'))
		{
			L_BYTE=(L_BYTE-0x37) & 0x0F ;
		}
		else if((L_BYTE>='a') && (L_BYTE<='f'))
		{
			L_BYTE=(L_BYTE-0x57) & 0x0F ;
		}
		else 
			L_BYTE=0;
		strDes[k]=H_BYTE+L_BYTE;
		k++;
	}
	delete str;
	return k;
}

/*********************************************************************
* ������ƣ�ASCIIStr_HexStr
* ����������ASII��ʽ���ת����ʮ������ַ� ��һ���ֽڲ�ֳ������ֽڡ���֣�
* �������const unsigned char *strSource , int len
* �������unsigned char *strDes
* �� �� ֵ����
* ����˵������
* ������ݣ�0x310x320x330x340x350x360x370x38 �� "12345678" to "3132333435363738"
* ԭ������hex_asc  ARRY_STR
**********************************************************************/
void ASCIIStr_HexStr(const unsigned char *strSource,unsigned char *strDes,int len)
{
	int i;
	for(i=0; i<len; i++)
	{
		if((strDes[2*i]=(strSource[i]>>4))<10)
			strDes[2*i]+='0';
		else
			strDes[2*i]+=('A'-0x0a);
		if((strDes[2*i+1]=(strSource[i]&0x0f))<10)
			strDes[2*i+1]+='0';
		else
			strDes[2*i+1]+=('A'-0x0a);
	}
}

/*********************************************************************
* ������ƣ�ASCII_Hex
* ������������һ���ֽ�ASII���ת����ʮ�������� 
* �������const unsigned char *strSource , int len
* �������unsigned char *strDes
* �� �� ֵ����
* ����˵������
* ������ݣ�0x31��1 to "31"
* ԭ������hex_asc_char   ARRY_STR  
**********************************************************************/
void ASCII_Hex(const unsigned char chSrc, unsigned char *strDes)
{
	if((strDes[0]=(chSrc>>4))<10)     //��λ
		strDes[0]+='0';
	else
		strDes[0]+=('A'-0x0a);
	if((strDes[1]=(chSrc&0x0f))<10)   //��λ
		strDes[1]+='0';
	else
		strDes[1]+=('A'-0x0a);
}

/*********************************************************************
* ������ƣ�HexStr_Int
* ������������ʮ����Ƶ��ַ�ת���ɶ�Ӧ���������
* �������unsigned char *HexData
* ���������
* �� �� ֵ��ת������������
* ����˵������
* ������ݣ�'0F' to 0x0F
* ԭ������Hex_Str_Int
**********************************************************************/
int HexStr_Int(unsigned char *HexData)
{
	if (HexData[0] > 0x40 )
	{
		HexData[0] -= 0x57;
	}
	
	if (HexData[1] > 0x40)
	{
		HexData[1] -= 0x57;
	}
	return  (HexData[0]&0x0F)<<4 | (HexData[1] &0x0F);
}

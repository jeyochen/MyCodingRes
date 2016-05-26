#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef unsigned char uint8;

uint8 CRC[2];//定义数组
void CRC16(uint8 *ptr, uint8 len)
{
  unsigned long wcrc=0XFFFF;//预置16位crc寄存器，初值全部为1
  unsigned char temp;//定义中间变量
  int i=0,j=0;//定义计数
  for(i=0;i<len;i++)//循环计算每个数据
  {
    temp=*ptr&0X00FF;//将八位数据与crc寄存器亦或
    ptr++;//指针地址增加，指向下个数据
    wcrc^=temp;//将数据存入crc寄存器
    for(j=0;j<8;j++)//循环计算数据的
    {
      if(wcrc&0X0001)//判断右移出的是不是1，如果是1则与多项式进行异或。
      {
        wcrc>>=1;//先将数据右移一位
        wcrc^=0XA001;//与上面的多项式进行异或
      }
      else//如果不是1，则直接移出
      {
        wcrc>>=1;//直接移出
      }
    }
  }
  temp=wcrc;//crc的值
  CRC[0]=wcrc;//crc的低八位
  CRC[1]=wcrc>>8;//crc的高八位
  fprintf(stdout, "CRC[0]=%02X, CRC[1]=%02X\n", CRC[0], CRC[1]);
}

int TrimSpace(char* pDest, const char *src, int nLen)
{
	int j = 0;
	int i;
	for (i = 0; i < nLen; i++)
	{
		if(src[i] == 0x20)
		{
			continue;
		}
		pDest[j++] = src[i];
	}
	pDest[j] = 0;
	return j;
}

void StrToHex(uint8 *pbDest, const char *pbSrc, int nLen)
{
    char h1,h2;
    uint8 s1,s2;
    
	int i;
    for (i = 0; i < nLen/2; i++)
    {
        h1 = pbSrc[2*i];
        h2 = pbSrc[2*i + 1];

        s1 = toupper(h1) - 0x30;
        if (s1 > 9)
            s1 -= 7;

        s2 = toupper(h2) - 0x30;
        if (s2 > 9)
            s2 -= 7;

        pbDest[i] = s1 * 16 + s2;
    }
}

int main()
{
	char buf[1024] = {"FF 10 00 0D 00 06 0C 00 AA 00 55 00 55 00 55 00 55 00 55"};
	char temp[1024] = {0};
	int trime_len = TrimSpace(temp, buf, strlen(buf));
	printf("%s\n", temp);
	
	uint8 dest[1024] = {0};
	memset(dest, 0, sizeof(dest));
	StrToHex(dest, temp, strlen(temp));
	int i = 0;
	for (; i < strlen(buf) / 2; i++)
	{
		printf("%02X", buf[i]);
	}
	printf("\n");
	CRC16(dest,trime_len / 2);
	return 0;
}
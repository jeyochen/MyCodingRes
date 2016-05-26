// XZ_F10_API.cpp : Defines the entry point for the DLL application.
#include "XZ_F10_API_LINUX.h"
#include "CSerial.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include "CLog.h"
#include "StringOperate.h"


#define						FAILURE							-1
#define						SUCCESS							0

SUNSON_CSerial::CSerial g_Com;
static CLog   g_Log;

#ifdef __cplusplus
extern "C"
{
#endif
//向串口发送数据
////////////////////////////////////////////////////////////////////////
int SUNSON_SendDataToCom(unsigned  char *str,int length)
{
	int i;
	i=g_Com.Write_Comm(str,length);
	return (i);
}

//从串口读出数据
int SUNSON_ReadDataFromCom(unsigned char *str,int length)
{
	int iactuallen=g_Com.Read_Comm(str,length);
	return iactuallen;
}

//从串口读一个字节数据
int SUNSON_ReadOneByte(unsigned char *str)
{
	g_Log.WriteLog("SUNSON_ReadOneByte");
	unsigned char str1[2];
	int i;
	g_Com.SetTimeOut(1);
	i=g_Com.Read_Comm(str1,1);
	if(i>0)
	{
		str[0]=str1[0];
		return 1;
	}
	else
		return 0;
}

//读取串口数据
int SUNSON_Read_Com_String(bool bDelaytime,unsigned char *str)
{
	str[0] = 'F';
	str[1] = '7';
	unsigned char ucConnect[10];
	memset(ucConnect,0,10);
	unsigned char str2[2];
	memset(str2,0,2);
	unsigned char templen[3];
	memset(templen,0,3);

	int nReadDataLength=0;
	int nDelayCount=-1;

	if(bDelaytime)
		nDelayCount=20;
	else
		nDelayCount=5;
	int i=-1;
	for(int m=0;m<nDelayCount;m++)
	{
		i=g_Com.Read_Comm(ucConnect,1);
		if(i<=0)
			continue;
		else
			break;
	}
	if(ucConnect[0]!=0x02)
		return -1;

	i=g_Com.Read_Comm(ucConnect,4);
	if(i<=0)
	{
		return -1;
	}
	str[0]=ucConnect[2];
	str[1]=ucConnect[3];
	if((ucConnect[2]!=0x30) &&(ucConnect[3]!=0x34))
	{
		return -1;//没有获取有效值，返回错误信息代码
	}
	templen[0]=ucConnect[0];
	templen[1]=ucConnect[1];
	templen[2]='\0';

	HexStr_ASCIIStr(templen,str2);	//返回有效数据长度

	nReadDataLength=str2[0];
	nReadDataLength-=1; //去掉校验位
	nReadDataLength*=2; //以BCD码拆分后的形式输出

	unsigned char *str1=new unsigned char[nReadDataLength+2];
	memset(str1,0,(nReadDataLength+2));
	i=g_Com.Read_Comm(str1,(nReadDataLength+2));
	if(i>0)
	{
		for(i=0;i<nReadDataLength;i++)
			str[i]=str1[i];
		delete str1;
		str1=NULL;
		return nReadDataLength/2; //返回有效数据长度
	}
	delete str1;
	str1=NULL;
	return -1;
}

int Return_ST(unsigned char *errorinfo)
{
	errorinfo[0] = 'F';
	errorinfo[1] = '7';
	unsigned char ucConnect[16];
	memset(ucConnect,0,16);
	unsigned char str2[2];
	memset(str2,0,2);
	unsigned char templen[2];
	memset(templen,0,2);
	int i=-1;
	for(int x=0;x<5;x++)
	{
		i=g_Com.Read_Comm(ucConnect,1);
		if (i<=0)
			continue;
		else
			break;
	}

	if(ucConnect[0]!=0x02) //判断包头
				return -1;

	i = g_Com.Read_Comm(ucConnect,2);  //读取长度
	if(i <= 0)
	{
		return -1;
	}
	unsigned char uclen[2] = {0};
	HexStr_ASCIIStr(ucConnect,uclen); //返回长度
	uclen[0] = uclen[0]*2 + 2; //把度度加上最后的检验位
	i=g_Com.Read_Comm(ucConnect,uclen[0]);
	if(i<=0)
	{
		return -1;
	}
	errorinfo[0]=ucConnect[0];
	errorinfo[1]=ucConnect[1];
	if((ucConnect[0]!=0x30) &&(ucConnect[1]!=0x34))
	{
		return -1;//没有获取有效值
	}
	if((ucConnect[0]==0x30) &&(ucConnect[1]==0x34))
		return 0;//指令执行成功
	return -1;
}

//////////////////////////////////////////////////////////////////////////
/*********************************************************************
	串口有关操作
	打开串口，关闭串口，读写串口
 **********************************************************************/
//打开串口
int  SUNSON_OpenCom(const char * sztty, long nBaud)
{
	char csLog[1024] = {0};

	g_Com.ClosePort();
	g_Log.WriteLog("---打开串口---");
	if(!g_Com.OpenPort(sztty,nBaud))
	{
		sprintf(csLog,"OpenCom nPort = %s,nBaud = %ld,failure",sztty,nBaud);
		g_Log.WriteLog(csLog);
		return FAILURE;
	}
	sprintf(csLog,"OpenCom nPort = %s,nBaud = %ld,success",sztty,nBaud);
	g_Log.WriteLog(csLog);
	return SUCCESS;
}

//关闭串口
int SUNSON_CloseCom(void)
{
	g_Log.WriteLog("---关闭串口---");
	char csLog[1024] = {0};
	sprintf(csLog,"CloseCom");
	g_Log.WriteLog(csLog);
	g_Com.ClosePort();
	return SUCCESS;
}

//获取按下的按键键值
int  SUNSON_ScanKeyPress(unsigned char *ucKeyValue)
{
	//g_Log.WriteLog("---获取按下的按键键值---");
	char csLog[1024] = {0};
	unsigned char keyvalue[2];
	memset(keyvalue,0,2);
	int len=SUNSON_ReadOneByte(keyvalue);
	if(len==1)
	{
		g_Log.WriteLog("---获取按下的按键键值---");
		ucKeyValue[0]=keyvalue[0];
		sprintf(csLog,"SUNSON_ScanKeyPress success, ucKeyValue = %d",ucKeyValue[0]);
		g_Log.WriteLog(csLog);
		return SUCCESS;
	}
	return FAILURE;
}

//下载主密钥
int SUNSON_LoadMasterKey (unsigned char ucMKeyId,unsigned char ucMKeyLen,unsigned char *NewMasterKey,unsigned char * ReturnInfo)
{
	g_Log.WriteLog("---下载主密钥---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_LoadMasterKey, ucMKeyId = 0x%02x,ucMKeyLen = %d",ucMKeyId,ucMKeyLen);
	g_Log.WriteLog(csLog);
	int i = 0;
	int nReturnValue=-1;
	unsigned char Xor=0x00;
	unsigned char str[32];
	memset(str,0,32);
	unsigned char MKId[2];
	memset(MKId,0,2);
	unsigned char XorResult[2];
	memset(XorResult,0,2);
	unsigned char CommandData[64];
	memset(CommandData,0,64);
	CommandData[0]=0x02;
	if(ucMKeyLen==16)
	{
		CommandData[1]=0x31;
		CommandData[2]=0x32;
		CommandData[3]=0x33;
		CommandData[4]=0x32;
		Xor=0x12^0x32^ucMKeyId;
	}
	else if(ucMKeyLen==8)
	{
		CommandData[1]=0x30;
		CommandData[2]=0x41;
		CommandData[3]=0x33;
		CommandData[4]=0x32;
		Xor=0x0A^0x32^ucMKeyId;
	}
	ASCII_Hex(ucMKeyId,MKId);
	CommandData[5]=MKId[0];
	CommandData[6]=MKId[1];
	for(i=0;i<ucMKeyLen*2;i++)
		CommandData[7+i]=NewMasterKey[i];
	HexStr_ASCIIStr(NewMasterKey,str);
	for(i=0;i<ucMKeyLen;i++)
		Xor^=str[i];
	ASCII_Hex(Xor,XorResult);
	CommandData[7+ucMKeyLen*2]=XorResult[0];
	CommandData[8+ucMKeyLen*2]=XorResult[1];
	i=SUNSON_SendDataToCom(CommandData,9+ucMKeyLen*2);
	// 		if(i==9+len*2 )
	// 		{
	// 			nReturnValue=Return_ST(result);
	// 			char temp[1024] = {0};
	// 			memcpy(temp,result,2);
	// 			wsprintf(csLog,"SUNSON_LoadMasterKey, result = %c%c",temp[0],temp[1]);
	// 			g_Log.WriteLog(csLog);
	// 			return nReturnValue;
	// 		}
	// 		else
	if (i==9+ucMKeyLen*2)                   //有验证返回
	{
		nReturnValue=SUNSON_Read_Com_String(true,ReturnInfo);
		if (nReturnValue > 0)
		{
			char temp[1024] = {0};
			memcpy(temp,ReturnInfo,nReturnValue*2);
			sprintf(csLog,"SUNSON_LoadMasterKey, ReturnInfo = (0x)%s",temp);
			g_Log.WriteLog(csLog);
		}
		else
		{
			char ResultFlag[6] = {0};
			if (ReturnInfo[0] == '0' && ReturnInfo[1] == '4')
			{
				strcpy(ResultFlag,"成功");
			}
			else
			{
				strcpy(ResultFlag,"失败");
			}

			sprintf(csLog,"SUNSON_LoadMasterKey, ReturnInfo = 0x%c%c(%s)",ReturnInfo[0],ReturnInfo[1],ResultFlag);
			g_Log.WriteLog(csLog);
		}
		return nReturnValue;
	}
	else
		return FAILURE;
}

//下载工作密钥
int SUNSON_LoadWorkKey(unsigned char ucMKeyId,unsigned char ucWKeyId,unsigned char ucWKeyLen,unsigned char* WorkKeyCiphertext,unsigned char *ReturnInfo)
{
	g_Log.WriteLog("---下载工作密钥---");
	char csLog[1024] = {0};
	char temp[1024] = {0};
	memcpy(temp,WorkKeyCiphertext,ucWKeyLen*2);
	sprintf(csLog,"SUNSON_LoadWorkKey,ucMKeyId = 0x%02x,ucWKeyId = 0x%02x,ucWKeyLen = %d,WorkKeyCiphertext = (0x)%s",ucMKeyId,ucWKeyId,ucWKeyLen,temp);
	g_Log.WriteLog(csLog);

	int i = 0;
	int nReturnValue=-1;
	unsigned char Xor=0x00;
	unsigned char str[32];
	memset(str,0,32);
	unsigned char MKId[2];
	memset(MKId,0,2);
	unsigned char WKId[2];
	memset(WKId,0,2);
	unsigned char XorResult[2];
	memset(XorResult,0,2);
	unsigned char CommandData[64];
	memset(CommandData,0,64);
	CommandData[0]=0x02;
	if((ucWKeyLen==16)||(ucWKeyLen==20))
	{
		CommandData[1]=0x31;
		CommandData[2]=0x33;
		CommandData[3]=0x33;
		CommandData[4]=0x33;
		Xor=0x13^0x33^ucMKeyId^ucWKeyId;
	}
	else if((ucWKeyLen==8)||(ucWKeyLen==12))
	{
		CommandData[1]=0x30;
		CommandData[2]=0x42;
		CommandData[3]=0x33;
		CommandData[4]=0x33;
		Xor=0x0B^0x33^ucMKeyId^ucWKeyId;
	}
	ASCII_Hex(ucMKeyId,MKId);
	CommandData[5]=MKId[0];
	CommandData[6]=MKId[1];
	ASCII_Hex(ucWKeyId,WKId);
	CommandData[7]=WKId[0];
	CommandData[8]=WKId[1];
	int wklen=-1;
	if((ucWKeyLen==12)||(ucWKeyLen==20))
		wklen=ucWKeyLen-4;
	else
		wklen=ucWKeyLen;
	for(i=0;i<wklen*2;i++)
		CommandData[9+i]=WorkKeyCiphertext[i];
	HexStr_ASCIIStr(WorkKeyCiphertext,str);
	for(i=0;i<wklen;i++)
		Xor^=str[i];
	ASCII_Hex(Xor,XorResult);
	CommandData[9+wklen*2]=XorResult[0];
	CommandData[10+wklen*2]=XorResult[1];
	i=SUNSON_SendDataToCom(CommandData,11+wklen*2);
	if(i==11+wklen*2)
	{
		nReturnValue=SUNSON_Read_Com_String(true,ReturnInfo);
		if (nReturnValue>0)
		{
			char temp[1024] = {0};
			memcpy(temp,ReturnInfo,nReturnValue*2);
			sprintf(csLog,"SUNSON_LoadWorkKey, ReturnInfo = %s",temp);
			g_Log.WriteLog(csLog);
		}
		else
		{
			char ResultFlag[6] = {0};
			if (ReturnInfo[0] == '0' && ReturnInfo[1] == '4')
			{
				strcpy(ResultFlag,"成功");
			}
			else
			{
				strcpy(ResultFlag,"失败");
			}
			sprintf(csLog,"SUNSON_LoadWorkKey, ReturnInfo = 0x%c%c(%s)",ReturnInfo[0],ReturnInfo[1],ResultFlag);
			g_Log.WriteLog(csLog);
		}
		return nReturnValue;
	}
	// 			nReturnValue=Return_ST(result);
	// 			if (nReturnValue==1)
	// 			{
	// 				if((len==12)||(len==20)) //进行密钥值验证
	// 				{
	// 					g_Log.WriteLog("进行密钥值验证");
	// 					BYTE ucCheckValue[8];
	// 					memset(ucCheckValue,0,8);
	// 					for(int m=0;m<8;m++)
	// 						ucCheckValue[m]=WorkKeyCiphertext[wklen*2+m]; //校验值
	// 					int nActiveWK=SUNSON_ActiveWorkKey(MasterKeyId,WorkKeyId,result); //激活密钥
	// 					if(nActiveWK)
	// 					{
	// 						BYTE sourcedata[16];
	// 						for(m=0;m<16;m++)
	// 							sourcedata[m]=0x30;
	// 						int nEncryDataLen=8;
	// 						BYTE ucGetCheckValue[16];
	// 						memset(ucGetCheckValue,0,16);
	// 						//SUNSON_SetEncryAlgorithm(0x20,result);
	// 						int nEncryDataResult=SUNSON_EncryData(nEncryDataLen,sourcedata,ucGetCheckValue); //与8个0x00进行运算
	// 						if(nEncryDataResult)
	// 						{
	// 							for(m=0;m<8;m++) //取前四个字节数据进行比较
	// 							{
	// 								if(ucGetCheckValue[m]!=ucCheckValue[m])
	// 									return FAILURE; //校验失败
	// 							}
	// 							g_Log.WriteLog("校验成功");
	// 							return SUCCESS; //校验成功
	// 						}
	// 						return FAILURE; //加密数据操作失败
	// 					}
	// 					return FAILURE; //激活密钥操作失败
	// 				}//不需验证，直接返回
	// 				//g_Log.WriteLog("不需验证，直接返回");
	// 				return SUCCESS;
	// 			}
	// 			return FAILURE;
	else
		return FAILURE;
}

//激活工作密钥
int SUNSON_ActiveKey(unsigned char ucMkeyId,unsigned char ucWkeyId,unsigned char *ReturnInfo)
{
	g_Log.WriteLog("---激活工作密钥---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_ActiveKey, ucMkeyId = 0x%02x,ucWkeyId = 0x%02x",ucMkeyId,ucWkeyId);
	g_Log.WriteLog(csLog);

	int nReturnValue=-1;
	unsigned char XorResult[2];
	memset(XorResult,0,2);
	unsigned char MKId[2];
	memset(MKId,0,2);
	unsigned char WKId[2];
	memset(WKId,0,2);
	unsigned char CommandData[11]={0x02,0x30,0x33,0x34,0x33,0x00,0x00,0x00,0x00,0x00,0x00};
	ASCII_Hex(ucMkeyId,MKId);
	CommandData[5]=MKId[0];
	CommandData[6]=MKId[1];
	ASCII_Hex(ucWkeyId,WKId);
	CommandData[7]=WKId[0];
	CommandData[8]=WKId[1];
	unsigned char Xor=0x00;
	Xor=0x03^0x43^ucMkeyId^ucWkeyId;
	ASCII_Hex(Xor,XorResult);
	CommandData[9]=XorResult[0];
	CommandData[10]=XorResult[1];
	int i=SUNSON_SendDataToCom(CommandData,11);
	if(i==11)
	{
		nReturnValue=Return_ST(ReturnInfo);
		char temp[1024] = {0};
		memcpy(temp,ReturnInfo,2);
		char ResultFlag[6] = {0};
		if (ReturnInfo[0] == '0' && ReturnInfo[1] == '4')
		{
			strcpy(ResultFlag,"成功");
		}
		else
		{
			strcpy(ResultFlag,"失败");
		}
		sprintf(csLog,"SUNSON_ActiveKey, result = 0x%c%c(%s)",ReturnInfo[0],ReturnInfo[1],ResultFlag);
		g_Log.WriteLog(csLog);
		return nReturnValue;
	}
	else
		return FAILURE;
}

//数据mac运算
int SUNSON_MakeMac(unsigned char nMacDataLen,unsigned char* ucMacData,unsigned char *ReturnInfo)
{
	g_Log.WriteLog("---数据mac运算---");
	char csLog[1024] = {0};
	char temp[1024] = {0};
	memcpy(temp,ucMacData,nMacDataLen*2);
	sprintf(csLog,"SUNSON_MakeMac nMacDataLen = %d,ucMacData = (0x)%s",nMacDataLen,temp);
	g_Log.WriteLog(csLog);

	unsigned char Xor=0x00;
	int nReturnValue=-1;
	unsigned char str[2048];
	memset(str,0,2048);
	unsigned char maclen[2];
	memset(maclen,0,2);
	unsigned char XorResult[2];
	memset(XorResult,0,2);
	int DataLenth=nMacDataLen*2+7;
	unsigned char *CommandData=new unsigned char[DataLenth];
	CommandData[0]=0x02;
	ASCII_Hex((nMacDataLen+1),maclen);
	CommandData[1]=maclen[0];
	CommandData[2]=maclen[1];
	CommandData[3]=0x34;
	CommandData[4]=0x31;
	for(int m=0;m<nMacDataLen*2;m++)
		CommandData[5+m]=ucMacData[m];
	unsigned char tmpuchar=0x41;
	Xor=(nMacDataLen+1)^tmpuchar;
	HexStr_ASCIIStr(ucMacData,str);
	for(int i=0;i<nMacDataLen;i++)
		Xor^=str[i];
	ASCII_Hex(Xor,XorResult);
	CommandData[5+nMacDataLen*2]=XorResult[0];
	CommandData[6+nMacDataLen*2]=XorResult[1];
	int n=SUNSON_SendDataToCom(CommandData,7+nMacDataLen*2);
	if(n==7+nMacDataLen*2)
	{
		nReturnValue=SUNSON_Read_Com_String(true,ReturnInfo);
		if (nReturnValue >0)
		{
			char temp[1024] = {0};
			memcpy(temp,ReturnInfo,nReturnValue*2);
			sprintf(csLog,"SUNSON_MakeMac, ReturnInfo = %s",temp);
			g_Log.WriteLog(csLog);
		}
		else
		{
			sprintf(csLog,"SUNSON_MakeMac, ReturnInfo = 0x%c%c(失败)",ReturnInfo[0],ReturnInfo[1]);
			g_Log.WriteLog(csLog);
		}


		delete CommandData;
		CommandData=NULL;
		return nReturnValue;
	}
	else
	{
		delete CommandData;
		CommandData=NULL;
		return FAILURE;
	}
}

//键盘复位自检，所有密钥复位
int SUNSON_ResetEPP(bool bInitKey)
{
	unsigned char result[2];
	memset(result,0,2);
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_ResetEPP");
	g_Log.WriteLog(csLog);
	int nReturnValue=-1;
	if (!bInitKey)
	{
		g_Log.WriteLog("---键盘复位自检(预置密钥区)---");
		unsigned char CommandData[11]={0x02,0x30,0x32,0x33,0x31,0x33,0x38,0x30,0x42};
		int i=SUNSON_SendDataToCom(CommandData,9);
		if(i==9)
		{
			sleep(3);
			nReturnValue=Return_ST(result);
			return nReturnValue;
		}
		else
			return FAILURE;
	}
	else
	{
		g_Log.WriteLog("---键盘复位自检(不破坏密钥区)---");
		unsigned char CommandData[]={0x02,0x30,0x31,0x33,0x31,0x33,0x30};
		int i=SUNSON_SendDataToCom(CommandData,7);
		if(i==7)
		{
			sleep(3);
			nReturnValue=Return_ST(result);
			return nReturnValue;
		}
		else
			return FAILURE;
	}

}

//回送字符
int SUNSON_SendAscII(unsigned char Ascii,unsigned char *result)
{
	g_Log.WriteLog("---回送字符---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_SendAscII,Ascii = 0x%02x",Ascii);
	g_Log.WriteLog(csLog);

	int nReturnValue=-1;
	unsigned char XorResult[2];
	memset(XorResult,0,2);
	unsigned char ucAscii[2];
	memset(ucAscii,0,2);
	unsigned char CommandData[9]={0x02,0x30,0x32,0x34,0x34,0x00,0x00,0x00,0x00};
	ASCII_Hex(Ascii,ucAscii);
	CommandData[5]=ucAscii[0];
	CommandData[6]=ucAscii[1];
	unsigned char Xor=0x00;
	Xor=0x02^0x44^Ascii;
	ASCII_Hex(Xor,XorResult);
	CommandData[7]=XorResult[0];
	CommandData[8]=XorResult[1];
	int i=SUNSON_SendDataToCom(CommandData,9);
	if(i==9)
	{
		nReturnValue=SUNSON_Read_Com_String(false,result);
		if (nReturnValue>0 )
		{
			char temp[1024] = {0};
			memcpy(temp,result,nReturnValue*2);
			sprintf(csLog,"SUNSON_SendAscII, result = 0x%s",temp);
			g_Log.WriteLog(csLog);
		}
		else
		{
			sprintf(csLog,"SUNSON_SendAscII, result = 0x%c%c(失败)",result[0],result[1]);
			g_Log.WriteLog(csLog);
		}

		return nReturnValue;
	}
	else
		return FAILURE;
}
//打开键盘仿真
int SUNSON_UseEppPlainTextMode(unsigned char ucTextModeFormat,unsigned char *ReturnInfo)
{
	g_Log.WriteLog("---打开键盘仿真---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_UsePlainKeyboard,ucTextModeFormat = 0x%02x",ucTextModeFormat);
	g_Log.WriteLog(csLog);
	int nReturnValue=-1;
	unsigned char XorResult[2];
	memset(XorResult,0,2);
	unsigned char keypara[2];
	memset(keypara,0,2);
	unsigned char CommandData[9]={0x02,0x30,0x32,0x34,0x35,0x00,0x00,0x00,0x00};
	ASCII_Hex(ucTextModeFormat,keypara);
	CommandData[5]=keypara[0];
	CommandData[6]=keypara[1];
	unsigned char Xor=0x00;
	Xor=0x02^0x45^ucTextModeFormat;
	ASCII_Hex(Xor,XorResult);
	CommandData[7]=XorResult[0];
	CommandData[8]=XorResult[1];
	int i=SUNSON_SendDataToCom(CommandData,9);
	if(i==9)
	{
		nReturnValue=Return_ST(ReturnInfo);
		g_Com.ClearTxRx();
		char temp[1024] = {0};
		memcpy(temp,ReturnInfo,2);
		char ResultFlag[6] = {0};
		if (ReturnInfo[0] == '0' && ReturnInfo[1] == '4')
		{
			strcpy(ResultFlag,"成功");
		}
		else
		{
			strcpy(ResultFlag,"失败");
		}
		sprintf(csLog,"SUNSON_UsePlainKeyboard, result = 0x%c%c(%s)",temp[0],temp[1],ResultFlag);
		g_Log.WriteLog(csLog);
		return nReturnValue;
	}
	else
		return FAILURE;
}

//启动密码键盘加密
int SUNSON_StartEpp(unsigned char ucPinLen,unsigned char AlgorithmMode,unsigned char timeout,unsigned char *ReturnInfo)
{
	g_Log.WriteLog("---启动密码键盘加密---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_StartEpp, ucPinLen = %d,AlgorithmMode = 0x%02x,timeout = %d",ucPinLen,AlgorithmMode,timeout);
	g_Log.WriteLog(csLog);

	int nReturnValue=-1;
	unsigned char XorResult[2];
	memset(XorResult,0,2);
	unsigned char ucparaA[2];
	memset(ucparaA,0,2);
	unsigned char ucparaB[2];
	memset(ucparaB,0,2);
	unsigned char ucparaC[2];
	memset(ucparaC,0,2);

	unsigned char CommandData[17]={0x02,0x30,0x36,0x33,0x35,0x00,0x00,0x30,0x31,0x00,0x00,0x30,0x30,0x00,0x00,0x00,0x00};
	ASCII_Hex(ucPinLen,ucparaA);
	CommandData[5]=ucparaA[0];
	CommandData[6]=ucparaA[1];

	ASCII_Hex(AlgorithmMode,ucparaB);
	CommandData[9]=ucparaB[0];
	CommandData[10]=ucparaB[1];

	ASCII_Hex(timeout,ucparaC);
	CommandData[13]=ucparaC[0];
	CommandData[14]=ucparaC[1];

	unsigned char Xor=0x00;
	Xor=0x06^0x35^0x01^0x00^ucPinLen^AlgorithmMode^timeout;
	ASCII_Hex(Xor,XorResult);
	CommandData[15]=XorResult[0];
	CommandData[16]=XorResult[1];
	int i=SUNSON_SendDataToCom(CommandData,17);
	if(i==17)
	{
		nReturnValue=Return_ST(ReturnInfo);
		char temp[1024] = {0};
		memcpy(temp,ReturnInfo,2);
		char ResultFlag[6] = {0};
		if (ReturnInfo[0] == '0' && ReturnInfo[1] == '4')
		{
			strcpy(ResultFlag,"成功");
		}
		else
		{
			strcpy(ResultFlag,"失败");
		}
		sprintf(csLog,"SUNSON_StartEpp, ReturnInfo = 0x%c%c(%s)",temp[0],temp[1],ResultFlag);
		g_Log.WriteLog(csLog);
		return nReturnValue;
	}
	else
		return FAILURE;
}

//取PIN密文
int SUNSON_ReadCypherPin(unsigned char *ReturnInfo)
{
	g_Log.WriteLog("---取PIN密文---");
	char csLog[100] = {0};
	sprintf(csLog,"SUNSON_ReadCypherPin");
	g_Log.WriteLog(csLog);
	int nReturnValue=-1;
	unsigned char CommandData[7]={0x02,0x30,0x31,0x34,0x32,0x34,0x33};
	int i=SUNSON_SendDataToCom(CommandData,7);
	if(i==7)
	{
		nReturnValue=SUNSON_Read_Com_String(false,ReturnInfo);
		if (nReturnValue > 0)
		{
			char temp[1024] = {0};
			memcpy(temp,ReturnInfo,16);
			sprintf(csLog,"SUNSON_ReadCypherPin, ReturnInfo = %s",temp);
			g_Log.WriteLog(csLog);
		}
		else
		{
			sprintf(csLog,"SUNSON_ReadCypherPin, ReturnInfo = (0x)%c%c(失败)",ReturnInfo[0],ReturnInfo[1]);
			g_Log.WriteLog(csLog);
		}

		return nReturnValue;
	}
	else
		return FAILURE;
}

//下载账号
int SUNSON_LoadCardNumber(unsigned char* ucCardNumber,unsigned char *ReturnInfo)
{
	g_Log.WriteLog("---下载账号---");
	char temp[1024] = {0};
	char csLog[1024] = {0};
	memcpy(temp,ucCardNumber,12);
	sprintf(csLog,"SUNSON_LoadCardNumber,ucCardNumber = %s",temp);
	g_Log.WriteLog(csLog);

	int nReturnValue=-1;
	int i = 0;
	//unsigned char CommandData[31]={0x02,0x30,0x44,0x33,0x34,0x33,0x30,0x33,0x30,0x33,0x30,0x33,0x30,0x33,0x30,0x33,0x30,0x33,0x30,0x33,0x30,0x33,0x30,0x33,0x30,0x33,0x30,0x33,0x30,0x33,0x39};
	unsigned char uccardno[24];
	memset(uccardno,0,24);
	unsigned char str[12];
	memset(str,0,12);
	unsigned char XorResult[2];
	memset(XorResult,0,2);
	unsigned char Xor=0x00;
	unsigned char CommandData[31];
	memset(CommandData,0,31);
	CommandData[0]=0x02;
	CommandData[1]=0x30;
	CommandData[2]=0x44;
	CommandData[3]=0x33;
	CommandData[4]=0x34;
	Xor=0x0d^0x34;
	ASCIIStr_HexStr(ucCardNumber,uccardno,12);
	for(i=0;i<24;i++)
		CommandData[5+i]=uccardno[i];
	for(i=0;i<12;i++)
		Xor^=ucCardNumber[i];
	ASCII_Hex(Xor,XorResult);
	CommandData[29]=XorResult[0];
	CommandData[30]=XorResult[1];
	i=SUNSON_SendDataToCom(CommandData,31);
	if(i==31)
	{
		nReturnValue=Return_ST(ReturnInfo);
		char temp[1024] = {0};
		memcpy(temp,ReturnInfo,2);
		char ResultFlag[6] = {0};
		if (ReturnInfo[0] == '0' && ReturnInfo[1] == '4')
		{
			strcpy(ResultFlag,"成功");
		}
		else
		{
			strcpy(ResultFlag,"失败");
		}
		sprintf(csLog,"SUNSON_LoadCardNumber, ReturnInfo = (0x)%c%c(%s)",temp[0],temp[1],ResultFlag);
		g_Log.WriteLog(csLog);
		return nReturnValue;
	}
	else
		return FAILURE;
}

//下载终端号 02h+0Bh+34h+<TRANS-Code>+<BCC>
int SUNSON_LoadTerminalNumber(unsigned char* ucTerminalNo,unsigned char *ReturnInfo)
{
	g_Log.WriteLog("---下载终端号---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_LoadTerminalNumber ucTerminalNo = %s",ucTerminalNo);
	g_Log.WriteLog(csLog);
	int i = 0;
	int nReturnValue=-1;
	unsigned char ucterminalno[20];
	memset(ucterminalno,0,20);
	unsigned char str[12];
	memset(str,0,12);
	unsigned char XorResult[2];
	memset(XorResult,0,2);
	unsigned char Xor=0x00;
	unsigned char CommandData[27];
	memset(CommandData,0,27);
	CommandData[0]=0x02;
	CommandData[1]=0x30;
	CommandData[2]=0x42;
	CommandData[3]=0x33;
	CommandData[4]=0x34;
	Xor=0x0b^0x34;
	ASCIIStr_HexStr(ucTerminalNo,ucterminalno,10);
	for(i=0;i<20;i++)
		CommandData[5+i]=ucterminalno[i];
	for(i=0;i<10;i++)
		Xor^=ucTerminalNo[i];
	ASCII_Hex(Xor,XorResult);
	CommandData[25]=XorResult[0];
	CommandData[26]=XorResult[1];
	i=SUNSON_SendDataToCom(CommandData,27);
	if(i==27)
	{
		nReturnValue=Return_ST(ReturnInfo);
		char temp[1024] = {0};
		memcpy(temp,ReturnInfo,2);
		char ResultFlag[6] = {0};
		if (ReturnInfo[0] == '0' && ReturnInfo[1] == '4')
		{
			strcpy(ResultFlag,"成功");
		}
		else
		{
			strcpy(ResultFlag,"失败");
		}
		sprintf(csLog,"SUNSON_LoadTerminalNumber, ReturnInfo = 0x%c%c(%s)",temp[0],temp[1],ResultFlag);
		g_Log.WriteLog(csLog);
		return nReturnValue;
	}
	else
		return FAILURE;
}

//数据加密运算
int SUNSON_DataEncrypt(unsigned  char ucDataLen,unsigned char* SourceData,unsigned char *ReturnInfo)
{
	g_Log.WriteLog("---数据加密运算---");
	char csLog[1024] = {0};
	char temp[1024] = {0};
	memcpy(temp,SourceData,ucDataLen*2);
	sprintf(csLog,"SUNSON_EncryData, ucDataLen = %d,SourceData = (0x)%s",ucDataLen,temp);
	g_Log.WriteLog(csLog);

	unsigned char Xor=0x00;
	int nReturnValue=-1;
	unsigned char str[2048];
	memset(str,0,2048);
	unsigned char datalen[2];
	memset(datalen,0,2);
	unsigned char XorResult[2];
	memset(XorResult,0,2);
	int nDataLenth=ucDataLen*2+7;
	unsigned char *CommandData=new unsigned char[nDataLenth];
	CommandData[0]=0x02;
	ASCII_Hex((ucDataLen+1),datalen);
	CommandData[1]=datalen[0];
	CommandData[2]=datalen[1];
	CommandData[3]=0x33;
	CommandData[4]=0x36;
	for(int m=0;m<ucDataLen*2;m++)
		CommandData[5+m]=SourceData[m];
	Xor=(ucDataLen+1)^0x36;
	HexStr_ASCIIStr(SourceData,str);
	for(int i=0;i<ucDataLen;i++)
		Xor^=str[i];
	ASCII_Hex(Xor,XorResult);
	CommandData[5+ucDataLen*2]=XorResult[0];
	CommandData[6+ucDataLen*2]=XorResult[1];
	int n=SUNSON_SendDataToCom(CommandData,7+ucDataLen*2);
	if(n==7+ucDataLen*2)
	{
		int resultlen=0;
		if(ucDataLen%8==0)
			resultlen=ucDataLen;
		else
			resultlen=((ucDataLen/8)+1)*8;
		resultlen*=2;
		nReturnValue=SUNSON_Read_Com_String(false,ReturnInfo);
		if (nReturnValue >0)
		{
			char temp[1024] = {0};
			memcpy(temp,ReturnInfo,nReturnValue*2);
			sprintf(csLog,"SUNSON_EncryData, ReturnInfo = %s",temp);
			g_Log.WriteLog(csLog);
		}
		else
		{
			sprintf(csLog,"SUNSON_EncryData, ReturnInfo = 0x%c%c(失败)",ReturnInfo[0],ReturnInfo[1]);
			g_Log.WriteLog(csLog);
		}
		delete CommandData;
		CommandData=NULL;

		return nReturnValue;
	}
	else
	{
		delete CommandData;
		CommandData=NULL;
		return FAILURE;
	}
}

//数据解密运算
int SUNSON_DataDecrypt(unsigned  char ucDataLen,unsigned char* SourceData,unsigned char *ReturnInfo)
{
	g_Log.WriteLog("---数据解密运算---");
	char temp[1024] = {0};
	memcpy(temp,SourceData,ucDataLen*2);

	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_DecryData, ucDataLen = %d,SourceData = (0x)%s",ucDataLen,temp);
	g_Log.WriteLog(csLog);
	unsigned char Xor=0x00;
	int nReturnValue=-1;
	unsigned char str[2048];
	memset(str,0,2048);
	unsigned char datalen[2];
	memset(datalen,0,2);
	unsigned char XorResult[2];
	memset(XorResult,0,2);
	int nDataLenth=ucDataLen*2+7;
	unsigned char *CommandData=new unsigned char[nDataLenth];
	CommandData[0]=0x02;
	ASCII_Hex((ucDataLen+1),datalen);
	CommandData[1]=datalen[0];
	CommandData[2]=datalen[1];
	CommandData[3]=0x33;
	CommandData[4]=0x37;
	for(int m=0;m<ucDataLen*2;m++)
		CommandData[5+m]=SourceData[m];
	Xor=(ucDataLen+1)^0x37;
	HexStr_ASCIIStr(SourceData,str);
	for(int i=0;i<ucDataLen;i++)
		Xor^=str[i];
	ASCII_Hex(Xor,XorResult);
	CommandData[5+ucDataLen*2]=XorResult[0];
	CommandData[6+ucDataLen*2]=XorResult[1];
	int n=SUNSON_SendDataToCom(CommandData,7+ucDataLen*2);
	if(n==7+ucDataLen*2)
	{
		int resultlen=0;
		if(ucDataLen%8==0)
			resultlen=ucDataLen;
		else
			resultlen=((ucDataLen/8)+1)*8;
		resultlen*=2;
		nReturnValue=SUNSON_Read_Com_String(false,ReturnInfo);

		if (nReturnValue >0)
		{
			char temp[1024] = {0};
			memcpy(temp,ReturnInfo,nReturnValue*2);
			sprintf(csLog,"SUNSON_DecryData, ReturnInfo = %s",temp);
			g_Log.WriteLog(csLog);
		}
		else
		{
			sprintf(csLog,"SUNSON_DecryData, ReturnInfo = 0x%c%c(失败)",ReturnInfo[0],ReturnInfo[1]);
			g_Log.WriteLog(csLog);
		}

		delete CommandData;
		CommandData=NULL;
		return nReturnValue;
	}
	else
	{
		delete CommandData;
		CommandData=NULL;
		return FAILURE;
	}
}

//设置IC卡卡座及类型
int SUNSON_SetSimCardIdAndKind(unsigned char CardId,unsigned char CardKind,unsigned char *result)
{
	g_Log.WriteLog("---设置IC卡卡座及类型---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_SetSimCardIdAndKind, CardId = 0x%02x",CardId);
	g_Log.WriteLog(csLog);
	int nReturnValue=-1;
	unsigned char XorResult[2];
	memset(XorResult,0,2);
	unsigned char ucCardId[2];
	memset(ucCardId,0,2);
	unsigned char ucCardKind[2];
	memset(ucCardKind,0,2);
	unsigned char CommandData[11]={0x02,0x30,0x33,0x35,0x39,0x00,0x00,0x00,0x00,0x00,0x00};
	ASCII_Hex(CardId,ucCardId);
	CommandData[5]=ucCardId[0];
	CommandData[6]=ucCardId[1];
	ASCII_Hex(CardKind,ucCardKind);
	CommandData[7]=ucCardKind[0];
	CommandData[8]=ucCardKind[1];
	unsigned char Xor=0x00;
	Xor=0x03^0x59^CardId^CardKind;
	ASCII_Hex(Xor,XorResult);
	CommandData[9]=XorResult[0];
	CommandData[10]=XorResult[1];
	int i=SUNSON_SendDataToCom(CommandData,11);
	if(i==11)
	{
		nReturnValue=Return_ST(result);
		char temp[1024] = {0};
		memcpy(temp,result,2);
		char ResultFlag[6] = {0};
		if (result[0] == '0' && result[1] == '4')
		{
			strcpy(ResultFlag,"成功");
		}
		else
		{
			strcpy(ResultFlag,"失败");
		}
		sprintf(csLog,"SUNSON_SetSimCardIdAndKind, result = 0x%c%c(%s)",temp[0],temp[1],ResultFlag);
		g_Log.WriteLog(csLog);
		return nReturnValue;
	}
	else
		return FAILURE;
}

//获取IC卡卡座及类型
int SUNSON_GetSimCardIdAndKind(unsigned char CardId,unsigned char *result)
{
	g_Log.WriteLog("---获取IC卡卡座及类型---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_GetSimCardIdAndKind, CardId = 0x%02x,",CardId);
	g_Log.WriteLog(csLog);
	int nReturnValue=-1;
	unsigned char XorResult[2];
	memset(XorResult,0,2);
	unsigned char ucCardId[2];
	memset(ucCardId,0,2);
	unsigned char CommandData[9]={0x02,0x30,0x32,0x35,0x41,0x00,0x00,0x00,0x00};
	ASCII_Hex(CardId,ucCardId);
	CommandData[5]=ucCardId[0];
	CommandData[6]=ucCardId[1];
	unsigned char Xor=0x00;
	Xor=0x02^0x5A^CardId;
	ASCII_Hex(Xor,XorResult);
	CommandData[7]=XorResult[0];
	CommandData[8]=XorResult[1];
	int i=SUNSON_SendDataToCom(CommandData,9);
	if(i==9)
	{
		nReturnValue=SUNSON_Read_Com_String(false,result);
		if (nReturnValue >0)
		{
			char temp[1024] = {0};
			memcpy(temp,result,nReturnValue*2);
			sprintf(csLog,"SUNSON_GetSimCardIdAndKind, result = %s",temp);
			g_Log.WriteLog(csLog);
		}
		else
		{
			sprintf(csLog,"SUNSON_GetSimCardIdAndKind, result = 0x%c%c(失败)",result[0],result[1]);
			g_Log.WriteLog(csLog);
		}

		return nReturnValue;
	}
	else
		return FAILURE;
}

//上电复位IC卡
int SUNSON_CardPowerOn(unsigned char *result)
{
	g_Log.WriteLog("---上电复位IC卡---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_CardPowerOn");
	g_Log.WriteLog(csLog);
	int nReturnValue=-1;
	unsigned char CommandData[7]={0x02,0x30,0x31,0x34,0x39,0x34,0x38};
	int i=SUNSON_SendDataToCom(CommandData,7);
	if(i==7)
	{
		nReturnValue=SUNSON_Read_Com_String(false,result);
		if (nReturnValue >0)
		{
			char temp[1024] = {0};
			memcpy(temp,result,nReturnValue*2);
			sprintf(csLog,"SUNSON_CardPowerOn, result = %s",temp);
			g_Log.WriteLog(csLog);
		}
		else
		{
			sprintf(csLog,"SUNSON_CardPowerOn, result = %c%c(失败)",result[0],result[1]);
			g_Log.WriteLog(csLog);
		}

		return nReturnValue;
	}
	else
		return FAILURE;
}

//IC卡下电
int SUNSON_CardPowerOff(unsigned char *result)
{
	g_Log.WriteLog("---IC卡下电---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_CardPowerOff");
	g_Log.WriteLog(csLog);
	int nReturnValue=-1;
	unsigned char CommandData[7]={0x02,0x30,0x31,0x35,0x42,0x35,0x41};
	int i=SUNSON_SendDataToCom(CommandData,7);
	if(i==7)
	{
		nReturnValue=Return_ST(result);
		char temp[1024] = {0};
		memcpy(temp,result,2);
		char ResultFlag[6] = {0};
		if (result[0] == '0' && result[1] == '4')
		{
			strcpy(ResultFlag,"成功");
		}
		else
		{
			strcpy(ResultFlag,"失败");
		}
		sprintf(csLog,"SUNSON_CardPowerOff, result = 0x%c%c(%s)",temp[0],temp[1],ResultFlag);
		g_Log.WriteLog(csLog);
		return nReturnValue;
	}
	else
		return FAILURE;
}

//访问COS
int SUNSON_UseAPDU(unsigned  char len,unsigned char* apdu,unsigned char *result)
{
	g_Log.WriteLog("---访问COS---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_UseAPDU, len = %d,apdu = %s",len,apdu);
	g_Log.WriteLog(csLog);
	unsigned char Xor=0x00;
	int nReturnValue=-1;
	unsigned char str[2048];
	memset(str,0,2048);
	unsigned char apdulen[2];
	memset(apdulen,0,2);
	unsigned char XorResult[2];
	memset(XorResult,0,2);
	int DataLenth=len*2+7;
	unsigned char *CommandData=new unsigned char[DataLenth];
	CommandData[0]=0x02;
	ASCII_Hex((len+1),apdulen);
	CommandData[1]=apdulen[0];
	CommandData[2]=apdulen[1];
	CommandData[3]=0x34;
	CommandData[4]=0x38;
	for(int m=0;m<len*2;m++)
		CommandData[5+m]=apdu[m];
	unsigned char tmpuchar=0x48;
	Xor=(len+1)^tmpuchar;
	HexStr_ASCIIStr(apdu,str);
	for(int i=0;i<len;i++)
		Xor^=str[i];
	ASCII_Hex(Xor,XorResult);
	CommandData[5+len*2]=XorResult[0];
	CommandData[6+len*2]=XorResult[1];
	int n=SUNSON_SendDataToCom(CommandData,7+len*2);
	if(n==7+len*2)
	{
		nReturnValue=SUNSON_Read_Com_String(false,result);
		delete CommandData;
		CommandData=NULL;
		if (nReturnValue > 0)
		{
			char temp[1024] = {0};
			memcpy(temp,result,nReturnValue*2);
			sprintf(csLog,"SUNSON_UseAPDU, result = %s",temp);
			g_Log.WriteLog(csLog);
		}
		else
		{
			sprintf(csLog,"SUNSON_UseAPDU, result = 0x%c%c(失败)",result[0],result[1]);
			g_Log.WriteLog(csLog);
		}

		return nReturnValue;
	}
	else
	{
		delete CommandData;
		CommandData=NULL;
		return FAILURE;
	}
}

//取PIN数据(用于sam加密)
int SUNSON_GetPinFromSIM(unsigned  char len,unsigned char* apdu,unsigned char *result)
{
	g_Log.WriteLog("---取PIN数据(用于sam加密)---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_GetPinFromSIM, len = %d,apdu = %s",len,apdu);
	g_Log.WriteLog(csLog);
	unsigned char Xor=0x00;
	int nReturnValue=-1;
	unsigned char str[2048];
	memset(str,0,2048);
	unsigned char apdulen[2];
	memset(apdulen,0,2);
	unsigned char XorResult[2];
	memset(XorResult,0,2);
	int DataLenth=len*2+7;
	unsigned char *CommandData=new unsigned char[DataLenth];
	CommandData[0]=0x02;
	ASCII_Hex((len+1),apdulen);
	CommandData[1]=apdulen[0];
	CommandData[2]=apdulen[1];
	CommandData[3]=0x34;
	CommandData[4]=0x37;
	for(int m=0;m<len*2;m++)
		CommandData[5+m]=apdu[m];
	unsigned char tmpuchar=0x47;
	Xor=(len+1)^tmpuchar;
	HexStr_ASCIIStr(apdu,str);
	for(int i=0;i<len;i++)
		Xor^=str[i];
	ASCII_Hex(Xor,XorResult);
	CommandData[5+len*2]=XorResult[0];
	CommandData[6+len*2]=XorResult[1];
	int n=SUNSON_SendDataToCom(CommandData,7+len*2);
	if(n==7+len*2)
	{
		nReturnValue=SUNSON_Read_Com_String(false,result);
		delete CommandData;
		CommandData=NULL;
		if (nReturnValue >0)
		{
			char temp[1024] = {0};
			memcpy(temp,result,nReturnValue*2);
			sprintf(csLog,"SUNSON_GetPinFromSIM, result = %s",temp);
			g_Log.WriteLog(csLog);
		}
		else
		{
			sprintf(csLog,"SUNSON_GetPinFromSIM, result = %c%c(失败)",result[0],result[1]);
			g_Log.WriteLog(csLog);
		}

		return nReturnValue;
	}
	else
	{
		delete CommandData;
		CommandData=NULL;
		return FAILURE;
	}
}

//读取版本号
int SUNSON_GetVersion(unsigned char *version)
{
	g_Log.WriteLog("---读取版本号---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_GetVersion");
	g_Log.WriteLog(csLog);

	int nReturnValue=-1;
	unsigned char ucver[52];
	memset(ucver,0,52);
	unsigned char CommandData[7]={0x02,0x30,0x31,0x33,0x30,0x33,0x31};
	int i=SUNSON_SendDataToCom(CommandData,7);
	if(i==7)
	{
		nReturnValue=SUNSON_Read_Com_String(false,ucver);
		if(nReturnValue==26)
		{
			for(i=0;i<10;i++)
				version[i]=ucver[20+i];
		}
		if (nReturnValue >0)
		{
			char temp[1024] = {0};
			memcpy(temp,version,10);
			sprintf(csLog,"SUNSON_GetVersion, result = (0x)%s",temp);
			g_Log.WriteLog(csLog);
		}
		else
		{
			sprintf(csLog,"SUNSON_GetVersion, result = 0x%c%c(失败)",ucver[0],ucver[1]);
			g_Log.WriteLog(csLog);
		}

		return nReturnValue;
	}
	else
		return FAILURE;

}

//设置产品终端号
int SUNSON_SetSerialNumber(unsigned char *SerialNumber,unsigned char *ReturnInfo)
{
	g_Log.WriteLog("---设置产品序列号---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_SetSerialNumber,SerialNumber = %s",SerialNumber);
	g_Log.WriteLog(csLog);
	int nReturnValue=-1;
	unsigned char str[32];
	unsigned char XorResult[2];

	unsigned char CommandData[24]={0x02,0x30,0x39,0x33,0x38};

	for (int i = 0; i < 16; i++)
	{
		CommandData[5+i] = SerialNumber[i];
	}

	unsigned char Xor=0x00;
	Xor = Xor^0x09^0x38;

	HexStr_ASCIIStr(SerialNumber,str);
	for(int i=0;i<8;i++)
		Xor^=str[i];
	ASCII_Hex(Xor,XorResult);

	CommandData[21]=XorResult[0];
	CommandData[22]=XorResult[1];
	CommandData[23]=0x03;


	int i=SUNSON_SendDataToCom(CommandData,24);
	if(i==24)
	{
		nReturnValue=Return_ST(ReturnInfo);
		char temp[1024] = {0};
		memcpy(temp,ReturnInfo,2);
		char ResultFlag[6] = {0};
		if (ReturnInfo[0] == '0' && ReturnInfo[1] == '4')
		{
			strcpy(ResultFlag,"成功");
		}
		else
		{
			strcpy(ResultFlag,"失败");
		}
		sprintf(csLog,"SUNSON_SetSerialNumber, result = 0x%c%c(%s)",temp[0],temp[1],ResultFlag);
		g_Log.WriteLog(csLog);
		return nReturnValue;
	}
	else
		return FAILURE;
}

//写产品版本号
int SUNSON_SetVersion(unsigned char*version,unsigned char*SerialNo,unsigned char *ReturnInfo)
{
	g_Log.WriteLog("---写产品版本号---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_SetVersion");
	g_Log.WriteLog(csLog);

	int nReturnValue=-1;
	unsigned char ucver[52];
	memset(ucver,0,52);
	unsigned char XorResult[2] = {0};
	unsigned char CommandData[59]={0};
	unsigned char Xor=0x00;
	unsigned char versionNo[32];
	unsigned char SerialNoHex[16];

	CommandData[0]=0x02;
	CommandData[1]=0x31;
	CommandData[2]=0x42;
	CommandData[3]=0x36;
	CommandData[4]=0x30;

	Xor = Xor^0x1b^0x60;

	ASCIIStr_HexStr(version,versionNo,16);
	for(int i=0;i<32;i++)
		CommandData[5+i]=versionNo[i];
	for(int i=0;i<16;i++)
		Xor^=version[i];

	ASCIIStr_HexStr(SerialNo,SerialNoHex,8);
	for(int i=0;i<16;i++)
		CommandData[37+i]=SerialNoHex[i];
	for(int i=0;i<8;i++)
		Xor^=SerialNo[i];


	for( int i=0;i<4;i++)
	{
		CommandData[53+i]=0x30;
	}

	ASCII_Hex(Xor,XorResult);
	CommandData[57]=XorResult[0];
	CommandData[58]=XorResult[1];

	int i=SUNSON_SendDataToCom(CommandData,59);
	if(i==59)
	{
		nReturnValue=Return_ST(ReturnInfo);
		char temp[1024] = {0};
		memcpy(temp,ReturnInfo,2);
		char ResultFlag[6] = {0};
		if (ReturnInfo[0] == '0' && ReturnInfo[1] == '4')
		{
			strcpy(ResultFlag,"成功");
		}
		else
		{
			strcpy(ResultFlag,"失败");
		}
		sprintf(csLog,"SUNSON_SetVersion, result = 0x%c%c(%s)",temp[0],temp[1],ResultFlag);
		g_Log.WriteLog(csLog);
		return nReturnValue;
	}
	else
		return FAILURE;

}

//读产品终端号
int SUNSON_GetSerialNumber(unsigned char *SerialNumber)
{
	g_Log.WriteLog("---读产品序列号---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_GetSerialNumber");
	g_Log.WriteLog(csLog);
	int nReturnValue=-1;
	unsigned char ucSN[32] = {0};
	unsigned char CommandData[7]={0x02,0x30,0x31,0x33,0x38,0x33,0x39};
	int i=SUNSON_SendDataToCom(CommandData,7);
	if(i==7)
	{
		nReturnValue=SUNSON_Read_Com_String(false,ucSN);
		if(nReturnValue>=8)
		{
			for(i=0;i<16;i++)
				SerialNumber[i]=ucSN[i];
		}
		if (nReturnValue >0)
		{
			char temp[1024] = {0};
			memcpy(temp,ucSN,16);
			sprintf(csLog,"SUNSON_GetSerialNumber, result = %s",temp);
			g_Log.WriteLog(csLog);
		}
		else
		{
			sprintf(csLog,"SUNSON_GetSerialNumber, result = 0x%c%c(失败)",ucSN[0],ucSN[1]);
			g_Log.WriteLog(csLog);
		}

		return nReturnValue;
	}
	else
		return FAILURE;
}

//UBC mac
int SUNSON_MakeUBCMac(int nMacLen,unsigned char* macdata,unsigned char *result,unsigned char *hexresult)
{
	g_Log.WriteLog("---UBC算法mac---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_MakeUBCMac, nMacLen = %d",nMacLen);
	g_Log.WriteLog(csLog);
	int nDataLine=0; //Mac数据长度
	if (nMacLen%8==0)
	{
		nDataLine=nMacLen/8;
	}
	else
		nDataLine=(nMacLen/8+1);
	int nRealMacDataLen=nDataLine*8;
	unsigned char *ucRealMacData=new unsigned char[nRealMacDataLen];
	memset(ucRealMacData,0,nRealMacDataLen);//Mac数据填充

	unsigned char str[2048];
	memset(str,0,2048);
	HexStr_ASCIIStr(macdata,str);
	for (int m=0;m<nRealMacDataLen;m++)
	{
		ucRealMacData[m]=str[m];
	}
	unsigned char ucXorData[8];
	memset(ucXorData,0,8);
	int i=-1;
	//8字节Mac数据，无需异或
	if (nDataLine==1)
	{
		for (i=0;i<8;i++)
		{
			ucXorData[i]=ucRealMacData[i];
		}
	}
	else if (nDataLine>=2) //Mac数据长度大于16字节
	{
		for (i=0;i<8;i++) //先将前16字节依次异或
		{
			ucXorData[i]=ucRealMacData[i]^ucRealMacData[i+8];
		}
		if (nDataLine>2) //将前16个字节的异或值与后续的每8个字节做异或
		{
			for(int k=2;k<nDataLine;k++)
			{
				for(i=0;i<8;i++)
					ucXorData[i]^=ucRealMacData[i+k*8];
			}
		}
	}
	//将异或值按ASCII拆分，1位变两位
	unsigned char ucHexMacData[16];
	memset(ucHexMacData,0,16);
	ASCIIStr_HexStr(ucXorData,ucHexMacData,8);

	unsigned char ucQianHexData[8]; //异或值前八个字节
	memset(ucQianHexData,0,8);
	unsigned char ucHouHexData[8];  //异或值后八个字节
	memset(ucHouHexData,0,8);

	for (i=0;i<8;i++)
	{
		ucQianHexData[i]=ucHexMacData[i];
		ucHouHexData[i]=ucHexMacData[i+8];
	}

	unsigned char ucFirstData[16];  //前八字节数据做ＤＥＳ运算结果
	memset(ucFirstData,0,16);

	unsigned char ucFirstDataHex[16];  //前八字节数据转为HEX运算结果
	memset(ucFirstDataHex,0,16);
	ASCIIStr_HexStr(ucQianHexData,ucFirstDataHex,8);

	unsigned char nEncryDataLen=8;
	int nEncryDataResult=SUNSON_DataEncrypt(nEncryDataLen,ucFirstDataHex,ucFirstData);

	unsigned char strtemp[2048];
	memset(strtemp,0,2048);
	HexStr_ASCIIStr(ucFirstData,strtemp);

	unsigned char ucFirstXorData[8]; // 存储前8字节加密结果与后八字节异或运算的结果
	memset(ucFirstXorData,0,8);
	if (nEncryDataResult >=0)
	{
		for (i=0;i<8;i++)
		{
			ucFirstXorData[i]=strtemp[i]^ucHouHexData[i];
		}
	}
	else
	{
		delete ucRealMacData;
		ucRealMacData=NULL;
		return FAILURE;
	}

	unsigned char ucSecondData[16];  //异或结果做DES运算
	memset(ucSecondData,0,16);

	unsigned char ucSecondHex[16];  //后八字节数据转为HEX运算结果
	memset(ucSecondHex,0,16);
	ASCIIStr_HexStr(ucFirstXorData,ucSecondHex,8);

	nEncryDataResult=SUNSON_DataEncrypt(nEncryDataLen,ucSecondHex,ucSecondData);
	if (nEncryDataResult>=0)
	{
		unsigned char ucLastData[32]; //加密结果做ASCII拆分
		memset(ucLastData,0,32);
		ASCIIStr_HexStr(ucSecondData,ucLastData,16);
		for (i=0;i<16;i++)
		{

			hexresult[i]=ucLastData[i]; //存储最后的mac结果
		}
		for (i=0;i<8;i++)
		{

			result[i]=ucSecondData[i]; //存储最后的mac结果
		}
		delete ucRealMacData;
		ucRealMacData=NULL;
		char temp[1024] = {0};
		memcpy(temp,result,16);
		sprintf(csLog,"SUNSON_MakeUBCMac, result = 0x%s",temp);
		g_Log.WriteLog(csLog);
		return SUCCESS;
	}
	delete ucRealMacData;
	ucRealMacData=NULL;
	return FAILURE;
}

int SUNSON_MakeX99ECBMac(int nMacLen,unsigned char* macdata,unsigned char *hexresult)
{
	g_Log.WriteLog("---ECB格式X9.9格式算法mac---");
	char csLog[1024] = {0};
	char temp[1024] = {0};
	memcpy(temp,macdata,nMacLen*2);
	sprintf(csLog,"SUNSON_MakeX99ECBMac,nMacLen = %d，macdata = %s",nMacLen,temp);
	g_Log.WriteLog(csLog);
	int m = 0;
	if(nMacLen<=0)
		return FAILURE;
	int nDataLine=0; //Mac数据长度
	if (nMacLen%8==0)
	{
		nDataLine=nMacLen/8;
	}
	else
		nDataLine=(nMacLen/8+1);
	int nRealMacDataLen=nDataLine*8;
	unsigned char *ucRealMacData=new unsigned char[nRealMacDataLen];
	memset(ucRealMacData,0,nRealMacDataLen);//Mac数据填充

	unsigned char str[2048];
	memset(str,0,2048);
	HexStr_ASCIIStr(macdata,str);
	for (m=0;m<nRealMacDataLen;m++)
	{
		ucRealMacData[m]=str[m];    //数据填充完毕
	}

	int i=-1;
	int n=-1;
	unsigned char ucEncryData[8];
	memset(ucEncryData,0,8);
	unsigned char ucXorData[8];
	memset(ucXorData,0,8);
	unsigned char ucHexEncryData[16];
	memset(ucHexEncryData,0,16);
	unsigned char errorinfo[8];
	memset(errorinfo,0,8);
	unsigned char ucResult[8];
	memset(ucResult,0,8);
	unsigned char uchexResult[16];
	memset(uchexResult,0,16);
	int nEncryDataLen=8;
	//第一组数据
	for(i=0;i<8;i++)
	{
		ucEncryData[i]=ucRealMacData[i];
	}
	int iret=SUNSON_SetAlgorithmParameter(0x01,0x20,errorinfo);
	if (iret)
	{
		int nEncryDataResult=-1;
		for (m=0;m<nDataLine;m++)
		{
			if(m!=0)
			{
				for(i=0;i<8;i++)
				{
					ucEncryData[i]^=ucRealMacData[i+m*8];
				}
			}
			ASCIIStr_HexStr(ucEncryData,ucHexEncryData,8);
			nEncryDataResult=SUNSON_DataEncrypt(nEncryDataLen,ucHexEncryData,uchexResult);
			if (nEncryDataResult>=0)
			{
				memset(str,0,2048);
				HexStr_ASCIIStr(uchexResult,str);  //转换为8字节数据
				for (n=0;n<8;n++)
				{
					ucEncryData[n]=str[n];    //数据填充完毕
				}
			}
			else
			{
				delete ucRealMacData;
				ucRealMacData=NULL;
				return FAILURE;
			}
		}
		for (i=0;i<16;i++)
		{
			hexresult[i]=uchexResult[i];
		}

		char temp[1024] = {0};
		memcpy(temp,hexresult,16);
		sprintf(csLog,"SUNSON_MakeX99ECBMac, result = 0x%s",temp);
		g_Log.WriteLog(csLog);
		delete ucRealMacData;
		ucRealMacData=NULL;
		return SUCCESS;
	}
	delete ucRealMacData;
	ucRealMacData=NULL;
	return FAILURE;
}

int SUNSON_MakeBaseMac(int nMacLen,unsigned char* macdata,unsigned char *hexresult)
{
	g_Log.WriteLog("---标准算法mac---");
	char csLog[1024] = {0};
	char temp[1024] = {0};
	memcpy(temp,macdata,nMacLen*2);
	sprintf(csLog,"SUNSON_MakeBaseMac,nMacLen = %d,macdata = %s",nMacLen,temp);
	g_Log.WriteLog(csLog);
	if(nMacLen<=0)
		return FAILURE;
	int m = 0;
	int nDataLine=0; //Mac数据长度
	if (nMacLen%8==0)
	{
		nDataLine=nMacLen/8;
	}
	else
		nDataLine=(nMacLen/8+1);
	int nRealMacDataLen=nDataLine*8;
	unsigned char *ucRealMacData=new unsigned char[nRealMacDataLen];
	memset(ucRealMacData,0,nRealMacDataLen);//Mac数据填充

	unsigned char str[2048];
	memset(str,0,2048);
	HexStr_ASCIIStr(macdata,str);
	for (m=0;m<nRealMacDataLen;m++)
	{
		ucRealMacData[m]=str[m];    //数据填充完毕
	}
	int i=-1;
	unsigned char ucXorData[8];
	memset(ucXorData,0,8);
	for(i=0;i<8;i++)
	{
		ucXorData[i]=ucRealMacData[i]^ucRealMacData[i+8];
	}

	for (m=2;m<nDataLine;m++)
	{
		for(i=0;i<8;i++)
		{
			ucXorData[i]^=ucRealMacData[i+m*8];
		}
	}
	unsigned char ucHexEncryData[16];
	memset(ucHexEncryData,0,16);
	unsigned char uchexResult[16];
	memset(uchexResult,0,16);
	ASCIIStr_HexStr(ucXorData,ucHexEncryData,8);
	unsigned char errorinfo[16];
	memset(errorinfo,0,16);
	int nEncryDataLen=8;
	int iret=SUNSON_SetAlgorithmParameter(0x01,0x20,errorinfo); //设置DES加密模式
	if (iret)
	{
		int nEncryDataResult=-1;
		nEncryDataResult=SUNSON_DataEncrypt(nEncryDataLen,ucHexEncryData,uchexResult);
		if (nEncryDataResult>=0)
		{
			for (i=0;i<16;i++)
			{
				hexresult[i]=uchexResult[i];
			}
			char temp[1024] = {0};
			memcpy(temp,hexresult,16);
			sprintf(csLog,"SUNSON_MakeBaseMac, result = 0x%s",temp);
			g_Log.WriteLog(csLog);
			delete ucRealMacData;
			ucRealMacData=NULL;
			return SUCCESS;
		}
		else
		{
			delete ucRealMacData;
			ucRealMacData=NULL;
			return FAILURE;
		}
	}
	delete ucRealMacData;
	ucRealMacData=NULL;
	return FAILURE;
}

int SUNSON_MakeX919ECBMac(int nMacLen,unsigned char* macdata,unsigned char *hexresult)
{
	g_Log.WriteLog("---ECB格式X9.19格式算法mac---");
	char csLog[1024] = {0};
	char temp[1024] = {0};
	memcpy(temp,macdata,nMacLen*2);
	sprintf(csLog,"SUNSON_MakeX919ECBMac, nMacLen = %d,macdata= %s",nMacLen,temp);
	g_Log.WriteLog(csLog);
	if(nMacLen<=0)
		return FAILURE;
	int m = 0;
	int nDataLine=0; //Mac数据长度
	if (nMacLen%8==0)
	{
		nDataLine=nMacLen/8;
	}
	else
		nDataLine=(nMacLen/8+1);
	int nRealMacDataLen=nDataLine*8;
	unsigned char *ucRealMacData=new unsigned char[nRealMacDataLen];
	memset(ucRealMacData,0,nRealMacDataLen);//Mac数据填充

	unsigned char str[2048];
	memset(str,0,2048);
	HexStr_ASCIIStr(macdata,str);
	for (m=0;m<nRealMacDataLen;m++)
	{
		ucRealMacData[m]=str[m];			//数据填充完毕
	}

	int i=-1;
	int n=-1;
	unsigned char ucEncryData[8];
	memset(ucEncryData,0,8);
	unsigned char ucXorData[8];
	memset(ucXorData,0,8);
	unsigned char ucHexEncryData[16];
	memset(ucHexEncryData,0,16);
	unsigned char errorinfo[8];
	memset(errorinfo,0,8);
	unsigned char ucResult[8];
	memset(ucResult,0,8);
	unsigned char uchexResult[16];
	memset(uchexResult,0,16);
	int nEncryDataLen=8;
	//第一组数据
	for(i=0;i<8;i++)
	{
		ucEncryData[i]=ucRealMacData[i];
	}
	int iret=SUNSON_SetAlgorithmParameter(0x01,0x20,errorinfo);
	if (iret)
	{
		int nEncryDataResult=-1;
		for (m=0;m<nDataLine;m++)
		{
			if(m!=0)
			{
				for(i=0;i<8;i++)
				{
					ucEncryData[i]^=ucRealMacData[i+m*8];
				}
			}
			if(m!=(nDataLine-1))
			{
				ASCIIStr_HexStr(ucEncryData,ucHexEncryData,8);
				nEncryDataResult=SUNSON_DataEncrypt(nEncryDataLen,ucHexEncryData,uchexResult);
				if (nEncryDataResult>=0)
				{
					memset(str,0,2048);
					HexStr_ASCIIStr(uchexResult,str);  //转换为8字节数据
					for (n=0;n<8;n++)
					{
						ucEncryData[n]=str[n];    //数据填充完毕
					}
				}
				else
				{
					delete ucRealMacData;
					ucRealMacData=NULL;
					return FAILURE;
				}
			}
			else
			{
				iret=SUNSON_SetAlgorithmParameter(0x01,0x30,errorinfo);
				if (iret)
				{
					ASCIIStr_HexStr(ucEncryData,ucHexEncryData,8);
					nEncryDataResult= SUNSON_DataEncrypt(nEncryDataLen,ucHexEncryData,uchexResult);
					if (nEncryDataResult>=0)
					{
						for (i=0;i<16;i++)
						{
							hexresult[i]=uchexResult[i];
						}
						char temp[1024] = {0};
						memcpy(temp,hexresult,16);
						sprintf(csLog,"SUNSON_MakeX919ECBMac, result = 0x%s",temp);
						g_Log.WriteLog(csLog);
						delete ucRealMacData;
						ucRealMacData=NULL;
						return SUCCESS;
					}
					else
					{
						delete ucRealMacData;
						ucRealMacData=NULL;
						return FAILURE;
					}

				}
				else
				{
					delete ucRealMacData;
					ucRealMacData=NULL;
					return FAILURE;
				}
			}
		}

	}
	delete ucRealMacData;
	ucRealMacData=NULL;
	return FAILURE;
}

int SUNSON_MakeX99CBCMac(int nMacLen,unsigned char* initvector,unsigned char* macdata,unsigned char *hexresult)
{
	g_Log.WriteLog("---CBC格式X9.9算法mac---");
	char csLog[1024] = {0};
	char temp[1024] = {0};
	memcpy(temp,macdata,nMacLen*2);
	sprintf(csLog,"SUNSON_MakeX99CBCMac, nMacLen = %d,macdata = %s",nMacLen,temp);
	g_Log.WriteLog(csLog);
	if(nMacLen<=0)
		return FAILURE;
	int m = 0;
	int nDataLine=0; //Mac数据长度
	if (nMacLen%8==0)
	{
		nDataLine=nMacLen/8;
	}
	else
		nDataLine=(nMacLen/8+1);
	int nRealMacDataLen=nDataLine*8;
	unsigned char *ucRealMacData=new unsigned char[nRealMacDataLen];
	memset(ucRealMacData,0,nRealMacDataLen);//Mac数据填充

	unsigned char str[2048];
	memset(str,0,2048);
	HexStr_ASCIIStr(macdata,str);
	for (m=0;m<nRealMacDataLen;m++)
	{
		ucRealMacData[m]=str[m];    //数据填充完毕
	}

	int i=-1;
	int n=-1;
	unsigned char ucEncryData[8];
	memset(ucEncryData,0,8);
	unsigned char ucXorData[8];
	memset(ucXorData,0,8);
	unsigned char ucHexEncryData[16];
	memset(ucHexEncryData,0,16);
	unsigned char errorinfo[8];
	memset(errorinfo,0,8);
	unsigned char ucResult[8];
	memset(ucResult,0,8);
	unsigned char uchexResult[16];
	memset(uchexResult,0,16);
	int nEncryDataLen=8;

	memset(str,0,2048);
	HexStr_ASCIIStr(initvector,str);
	//第一组数据
	for(i=0;i<8;i++)
	{
		ucEncryData[i]=str[i]^ucRealMacData[i];  //初始向量与第一组数据异或
	}
	int iret=SUNSON_SetAlgorithmParameter(0x01,0x20,errorinfo);
	if (iret)
	{
		int nEncryDataResult=-1;
		for (m=0;m<nDataLine;m++)
		{
			if(m!=0)
			{
				for(i=0;i<8;i++)
				{
					ucEncryData[i]^=ucRealMacData[i+m*8];
				}
			}
			ASCIIStr_HexStr(ucEncryData,ucHexEncryData,8);
			nEncryDataResult=SUNSON_DataEncrypt(nEncryDataLen,ucHexEncryData,uchexResult);
			if (nEncryDataResult>=0)
			{
				memset(str,0,2048);
				HexStr_ASCIIStr(uchexResult,str);  //转换为8字节数据
				for (n=0;n<8;n++)
				{
					ucEncryData[n]=str[n];    //数据填充完毕
				}
			}
			else
			{
				delete ucRealMacData;
				ucRealMacData=NULL;
				return FAILURE;
			}
		}
		for (i=0;i<16;i++)
		{
			hexresult[i]=uchexResult[i];
		}
		delete ucRealMacData;
		ucRealMacData=NULL;
		char temp[1024] = {0};
		memcpy(temp,hexresult,16);
		sprintf(csLog,"SUNSON_MakeX99CBCMac, result = 0x%s",temp);
		g_Log.WriteLog(csLog);
		return SUCCESS;
	}
	delete ucRealMacData;
	ucRealMacData=NULL;
	return FAILURE;
}

///设置算法处理参数
int SUNSON_SetAlgorithmParameter(unsigned char PPara,unsigned char FPara,unsigned char *result)
{
	g_Log.WriteLog("---设置算法处理参数---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_SetAlgorithmParameter,PPara = 0x%02x,FPara = 0x%02x",PPara,FPara);
	g_Log.WriteLog(csLog);

	int nReturnValue=-1;
	unsigned char XorResult[2];
	memset(XorResult,0,2);

	unsigned char ucAlgorithmPPara[2];
	memset(ucAlgorithmPPara,0,2);
	unsigned char ucAlgorithmFPara[2];
	memset(ucAlgorithmFPara,0,2);

	unsigned char CommandData[11]={0x02,0x30,0x33,0x34,0x36,0x30,0x36,0x00,0x00,0x00,0x00};

	ASCII_Hex(PPara,ucAlgorithmPPara);
	CommandData[5]=ucAlgorithmPPara[0];
	CommandData[6]=ucAlgorithmPPara[1];

	ASCII_Hex(FPara,ucAlgorithmFPara);
	CommandData[7]=ucAlgorithmFPara[0];
	CommandData[8]=ucAlgorithmFPara[1];

	unsigned char Xor=0x00;
	Xor=0x03^0x46^PPara^FPara;
	ASCII_Hex(Xor,XorResult);
	CommandData[9]=XorResult[0];
	CommandData[10]=XorResult[1];
	int i=SUNSON_SendDataToCom(CommandData,11);
	if(i==11)
	{
		nReturnValue=Return_ST(result);
		char temp[1024] = {0};
		memcpy(temp,result,2);
		char ResultFlag[6] = {0};
		if (result[0] == '0' && result[1] == '4')
		{
			strcpy(ResultFlag,"成功");
		}
		else
		{
			strcpy(ResultFlag,"失败");
		}
		sprintf(csLog,"SUNSON_SetAlgorithmParameter, result = 0x%c%c(%s)",temp[0],temp[1],ResultFlag);
		g_Log.WriteLog(csLog);
		return nReturnValue;
	}
	else
		return FAILURE;
}

int SUNSON_Authentication(unsigned char authFlag,unsigned char* customerData, unsigned char * result)
{
	g_Log.WriteLog("---用户认证命令---");
	char csLog[1024] = {0};

	unsigned char ucTempstr[32] = {0};
	ASCIIStr_HexStr(customerData,ucTempstr,16);
	char temp[1024] = {0};
	memcpy(temp,ucTempstr,32);

	sprintf(csLog,"SUNSON_Authentication, authFlag = 0x%02x,customerData = (0x)%s",authFlag,temp);
	g_Log.WriteLog(csLog);

	int nReturnValue=-1;
	int i = 0;
	unsigned char ucAuthMode[2];
	memset(ucAuthMode,0,2);
	ASCII_Hex(authFlag,ucAuthMode);

	unsigned char XorResult[2];
	memset(XorResult,0,2);
	unsigned char Xor=0x00;
	Xor = Xor^0x12^0x40^authFlag;
	for(i=0;i<16;i++)
		Xor^=customerData[i];
	ASCII_Hex(Xor,XorResult);

	unsigned char CommandData[64]={0x02,0x31,0x32,0x34,0x30,ucAuthMode[0],ucAuthMode[1]};

	for (i = 0; i < 32; i++)
	{
		CommandData[7+i] = ucTempstr[i];
	}

	CommandData[39]=XorResult[0];
	CommandData[40]=XorResult[1];
	CommandData[41]=0x03;

	i=SUNSON_SendDataToCom(CommandData,42);
	if(i==42)
	{
		nReturnValue=SUNSON_Read_Com_String(true,result);
		if (nReturnValue > 0)
		{
			char temp[1024] = {0};
			memcpy(temp,result,nReturnValue*2);
			sprintf(csLog,"SUNSON_Authentication, ReturnInfo = (0x)%s",temp);
			g_Log.WriteLog(csLog);
		}
		else
		{
			char ResultFlag[6] = {0};
			if (result[0] == '0' && result[1] == '4')
			{
				strcpy(ResultFlag,"成功");
			}
			else
			{
				strcpy(ResultFlag,"失败");
			}

			sprintf(csLog,"SUNSON_Authentication, ReturnInfo = 0x%c%c(%s)",result[0],result[1],ResultFlag);
			g_Log.WriteLog(csLog);
		}
		return nReturnValue;
	}
	else
		return FAILURE;
}

int SUNSON_MoveCertification(unsigned char Movemode,unsigned char *errorinfo)
{
	g_Log.WriteLog("---打开/关闭移出功能---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_MoveCertification, Movemode = 0x%02x",Movemode);
	g_Log.WriteLog(csLog);

	int i=0;
	int nReturnValue=-1;
	unsigned char modepara[2] = {0};
	unsigned char XorResult[2] = {0};

	unsigned char CommandData[]={0x02,0x30,0x32,0x36,0x31,0x00,0x00,0x00,0x00};

	ASCII_Hex(Movemode,modepara);

	CommandData[5]=modepara[0];
	CommandData[6]=modepara[1];

	unsigned char Xor=0x00;

	Xor = Xor^0x02^0x61^Movemode;
	ASCII_Hex(Xor,XorResult);

	CommandData[7]=XorResult[0];
	CommandData[8]=XorResult[1];

	i=SUNSON_SendDataToCom(CommandData,9);
	if(i==9)
	{
		nReturnValue=Return_ST(errorinfo);
		char temp[1024] = {0};
		memcpy(temp,errorinfo,2);
		char ResultFlag[6] = {0};
		if (errorinfo[0] == '0' && errorinfo[1] == '4')
		{
			strcpy(ResultFlag,"成功");
		}
		else
		{
			strcpy(ResultFlag,"失败");
		}
		sprintf(csLog,"SUNSON_MoveCertification, result = 0x%c%c(%s)",errorinfo[0],errorinfo[1],ResultFlag);
		g_Log.WriteLog(csLog);
		return nReturnValue;
	}
	else
		return 0;
}

//利用SAM卡下载工作密钥
int SUNSON_LoadWorkKeyBySAM(unsigned char APDULen,unsigned char MKeyID,unsigned char WKeyID, unsigned char * APDU,unsigned char *errorinfo)
{
	g_Log.WriteLog("---利用SAM卡下载工作密钥---");
	char csLog[1024] = {0};
	sprintf(csLog,"SUNSON_LoadWorkKeyBySAM, APDULen = 0x%02x,MKeyID = 0x%02x,WKeyID = 0x%02x,APDU = 0x%s",APDULen,MKeyID,WKeyID,APDU);
	g_Log.WriteLog(csLog);

	int i=0;
	int nReturnValue=-1;
	unsigned char APDULenHex[2] = {0};
	unsigned char MKeyIDHex[2] = {0};
	unsigned char WKeyIDHex[2] = {0};

	unsigned char XorResult[2] = {0};


	unsigned char * CommandData = new unsigned char[strlen((char *)APDU) + 11];

	ASCII_Hex(APDULen,APDULenHex);
	ASCII_Hex(MKeyID,MKeyIDHex);
	ASCII_Hex(WKeyID,WKeyIDHex);

	CommandData[0]=0x02;
	CommandData[1]=APDULenHex[0];
	CommandData[2]=APDULenHex[1];

	CommandData[3]=0x34;
	CommandData[4]=0x41;

	CommandData[5]=MKeyIDHex[0];
	CommandData[6]=MKeyIDHex[1];

	CommandData[7]=WKeyIDHex[0];
	CommandData[8]=WKeyIDHex[1];

	for (int i = 0; i < strlen((char *)APDU); i++)
	{
		CommandData[9+i] = APDU[i];
	}
	unsigned char Xor=0x00;
	Xor = Xor^APDULen^0x4A^MKeyID^WKeyID;
	ASCII_Hex(Xor,XorResult);

	unsigned char str[2048];
	memset(str,0,2048);
	HexStr_ASCIIStr(APDU,str);
	for(i=0;i<strlen((char *)APDU)/2;i++)
		Xor^=str[i];

	ASCII_Hex(Xor,XorResult);

	CommandData[9+strlen((char *)APDU)]=XorResult[0];
	CommandData[10+strlen((char *)APDU)]=XorResult[1];

	i=SUNSON_SendDataToCom(CommandData,strlen((char *)APDU) + 11);
	if(i == strlen((char *)APDU) + 11)
	{
		nReturnValue=Return_ST(errorinfo);
		char temp[1024] = {0};
		memcpy(temp,errorinfo,2);
		char ResultFlag[6] = {0};
		if (errorinfo[0] == '0' && errorinfo[1] == '4')
		{
			strcpy(ResultFlag,"成功");
		}
		else
		{
			strcpy(ResultFlag,"失败");
		}
		sprintf(csLog,"SUNSON_MoveCertification, result = 0x%c%c(%s)",errorinfo[0],errorinfo[1],ResultFlag);
		g_Log.WriteLog(csLog);
		return nReturnValue;
	}
	else
		return 0;
}

#ifdef __cplusplus
};
#endif

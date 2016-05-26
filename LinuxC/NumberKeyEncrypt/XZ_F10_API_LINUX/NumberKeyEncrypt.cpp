#include "NumberKeyEncrypt.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "XZ_F10_API_LINUX.h"

const int MAX_MSG_LEN = 1024;


CNumberKeyEncrypt* CNumberKeyEncrypt::m_pInstance = NULL;
CNumberKeyEncrypt::CNumberKeyEncrypt()
	:m_port(-1)
	,m_baud(9600)
{
	
}

CNumberKeyEncrypt::~CNumberKeyEncrypt()
{
	DestroyInstance();
}

bool CNumberKeyEncrypt::SetSerialParam(int iPort, long lBaud)
{
	if (0 > iPort || 256 < iPort)
	{
		return false;
	}
	
	m_port = iPort;
	m_baud = lBaud;
	char buf[64] = {0};
#ifdef _TTY_USB
	snprintf(buf, sizeof(buf), "/dev/ttyUSB%d", m_port);
#else
	snprintf(buf, sizeof(buf), "/dev/ttyS%d", m_port);
#endif
	
	m_portname = buf;
	return true;
}

void CNumberKeyEncrypt::GetSerialParam(int &iPort, long &lBaud)
{
	iPort = m_port;
	lBaud = m_baud;
}

bool CNumberKeyEncrypt::SetWorkKey(const char *szData)
{
	string strData = szData;
	if (80 > strData.length())
	{
		PrintLog("工作密钥长度小于80个字符 :%s", strData.c_str());
		return false;
	}
	unsigned char chKey[32+1] = {0};
	unsigned char chReturnInfo[1024] = {0};
	string strWorkKey = strData.substr(0, 32);
	string strMACKey = strData.substr(40, 16);
	string strCheckValue = strData.substr(32, 8);
	string strSuccessCode = "04";
	string strTemp;
	string strReturnInfo;
	bool bResult = false;
	int iRet = -1;

	PrintLog("灌注密码键盘workkey 开始 :%s", strData.c_str());
	iRet = SUNSON_OpenCom(m_portname.c_str(), m_baud);
	if (0 != iRet)
	{
		PrintLog("SUNSON_OpenCom 失败, portname:%s", m_portname.c_str());
		goto RET;
	}
	
	iRet = SUNSON_UseEppPlainTextMode(0x03, chReturnInfo);
	if (0 != iRet)
	{
		PrintLog("SUNSON_UseEppPlainTextMode 失败");
		goto RET;
	}

	iRet =  SUNSON_SetAlgorithmParameter(0x00, 0x30, chReturnInfo);
	if (0 != iRet)
	{
		PrintLog("SUNSON_SetAlgorithmParameter 0x00,0x30  失败");
		goto RET;
	}

	iRet =  SUNSON_SetAlgorithmParameter(0x05, 0x04, chReturnInfo);
	if (0 != iRet)
	{
		PrintLog("SUNSON_SetAlgorithmParameter 0x05,0x04  失败");
		goto RET;
	}

	//灌注工作密钥
	memcpy(chKey, strWorkKey.c_str(), 32);
	memset(chReturnInfo, 0x00, sizeof(chReturnInfo));
	iRet =  SUNSON_LoadWorkKey(0, 0, 16, chKey, chReturnInfo);

	if (0 != iRet)
	{
		PrintLog("灌注工作密钥SUNSON_LoadWorkKey 失败:%s", (char *)chReturnInfo);
		goto RET;
	}

	strReturnInfo = (char *)chReturnInfo;
	//strReturnInfo = strReturnInfo.substr(0, 8);
	// 返回04为注入成功
	if(0 != strncmp(strSuccessCode.c_str(), strReturnInfo.c_str(), strSuccessCode.length()))
	{
		PrintLog("工作密钥校验 失败 :%s :%s", strCheckValue.c_str(), strReturnInfo.c_str());
		goto RET;
	}
	PrintLog("工作密钥校验 成功 :%s :%s", strCheckValue.c_str(), strReturnInfo.c_str());

	//灌注MAC密钥
	strCheckValue = strData.substr(72, 8);
	strMACKey += strMACKey;
	memcpy(chKey, strMACKey.c_str(), 32);
	memset(chReturnInfo, 0x00, sizeof(chReturnInfo));
	iRet =  SUNSON_LoadWorkKey(0, 1, 16, chKey, chReturnInfo);
	if (0 != iRet)
	{
		PrintLog("灌注MAC密钥 SUNSON_LoadWorkKey 失败:%s", (char *)chReturnInfo);
		goto RET;
	}
	
	strReturnInfo = (char *)chReturnInfo;
	//strReturnInfo = strReturnInfo.substr(0, 8);

	if(0 != strncmp(strSuccessCode.c_str(), strReturnInfo.c_str(), strSuccessCode.length()))
	{
		PrintLog("MAC密钥校验 失败 :%s :%s", strCheckValue.c_str(), strReturnInfo.c_str());
		goto RET;
	}
	PrintLog("MAC密钥校验 成功 :%s :%s", strCheckValue.c_str(), strReturnInfo.c_str());
	bResult = true;
RET:
	 SUNSON_CloseCom();
	 PrintLog("灌注密码键盘workkey 结束 :%d", bResult);
	 return bResult;
}


const char* CNumberKeyEncrypt::Encrypt(const int iType, const char *szData)
{
	string strData = szData;
	string strEncrypt;
	string strDebug;
	PrintLog("密码键盘加密 开始 :%d :%s", iType, strData.c_str());
	
	unsigned char chCardNumber[12+1] = {0};
	unsigned char chReturnInfo[1024] = {0};
	string strTemp;
	string strReturnInfo;
	int iRet = -1;
	bool bOk = false;
	int i = 0;

	iRet = SUNSON_OpenCom(m_portname.c_str(), m_baud);
	if (0 != iRet)
	{
		PrintLog("SUNSON_OpenCom 失败, m_portname=%s", m_portname.c_str());
		goto RET;
	}

	//pin 加密
	if (0 == iType)
	{
#if 0
		memcpy(chCardNumber, strData.c_str(), 12);
		//下载卡号
		iRet = SUNSON_LoadCardNumber(chCardNumber, chReturnInfo);
		if(-1 == iRet )
		{
			goto RET;
		}

		iRet = SUNSON_ActiveKey(0, 0, chReturnInfo);
		if(-1 == iRet )
		{
			goto RET;
		}

		//设置补偿模式
		iRet = SUNSON_SetAlgorithmParameter(0x02, 0x0f, chReturnInfo);
		if(-1 == iRet)
		{
			goto RET;
		}

		//设定密码长度
		iRet = SUNSON_SetAlgorithmParameter(0x09, 0x06, chReturnInfo);
		if(-1 == iRet)
		{
			goto RET;
		}

		iRet = SUNSON_SetAlgorithmParameter(0x01, 0x30, chReturnInfo);
		if(-1 == iRet)
		{
			goto RET;
		}

		//设置自动回送回车
		iRet = SUNSON_SetAlgorithmParameter(0x05, 0x01, chReturnInfo);
		if(-1 == iRet)
		{
			goto RET;
		}

		//设置Pin运算格式
		iRet = SUNSON_SetAlgorithmParameter(0x04, 0x10, chReturnInfo);
		if(-1 == iRet)
		{
			goto RET;
		}
		
		//打开蜂鸣器
		iRet = SUNSON_UseEppPlainTextMode(0x02, chReturnInfo); 
		if (iRet == -1)
		{
			goto RET;
		}

		fprintf(stdout, "输入密码：\n");
		//启动加密
		iRet = SUNSON_StartEpp(6, 0x01, 20, chReturnInfo);
		
		if(iRet == 0)
		{
			for (i = 0; i < 200; i++)
			{
				PrintLog("扫描按键 :%d", i);
		
				unsigned char ucbuf[8];
				memset(ucbuf, 0, 8);
				iRet = SUNSON_ScanKeyPress(ucbuf);
				if(iRet == 0)
				{
					PrintLog("用户按键 :%02X :%s", ucbuf[0], (char *)ucbuf);
					
					unsigned char strKey = ucbuf[0];
					if ( 0X0D == strKey) 
					{
						bOk = true;
						PrintLog("用户按确认键 :%02X :%s", ucbuf[0], (char *)ucbuf);
						break;
					}
					else if (0X1B == strKey)
					{
						bOk = false;
						PrintLog("用户按取消键 :%02X :%s", ucbuf[0], (char *)ucbuf);
						break;
					}
				}
			}

			if ((i >= 100) ||(!bOk))
			{
				goto RET;
			}
			unsigned char ciphertext[28];
			memset(ciphertext, 0, 28);
			iRet = SUNSON_ReadCypherPin(ciphertext);
			if (iRet == 13)
			{
				strEncrypt = (char *)ciphertext;
				strEncrypt = strEncrypt.substr(0, 16);
				PrintLog("PIN加密结果:%s :%s", (char *)ciphertext, strEncrypt.c_str());
			}
		}
		else
		{
			goto RET;
		}
#endif
	} 
	//mac加密
	else if (1 == iType)
	{
		//设置密钥
		iRet = SUNSON_ActiveKey(0, 1, chReturnInfo);
		if(-1 == iRet )
		{
			PrintLog("激活加密密钥失败！");
			goto RET;
		}

		//设置加密方式
		iRet = SUNSON_SetAlgorithmParameter(0x01, 0x30, chReturnInfo);
		if(-1 == iRet)
		{
			PrintLog("设置加密方式失败！");
			goto RET;
		}

		//设置Mac算法
		iRet = SUNSON_SetAlgorithmParameter(0x06, 0x04, chReturnInfo);
		if(-1 == iRet)
		{
			PrintLog("设置加密算法失败！");
			goto RET;
		}

		//Mac运算
		unsigned char ucMacData[MAX_MSG_LEN] = {0};
		unsigned char ucHexReturnInfo[1024] = {0};
		int iLen = strData.length();
		memcpy(ucMacData, strData.c_str(), iLen);
		iRet = SUNSON_MakeUBCMac(iLen, ucMacData, ucHexReturnInfo, chReturnInfo);
		if(iRet != -1)
		{
			strEncrypt = (char *)chReturnInfo;
			PrintLog("MAC加密结果 :%s", (char *)chReturnInfo);
		}
	}

RET:
	SUNSON_CloseCom();
	PrintLog("密码键盘加密 结束 :%s", strEncrypt.c_str());
	
	return strEncrypt.c_str();
}

// 输出日志信息
int CNumberKeyEncrypt::PrintLog(const char *format, ...)
{
#ifdef _DEBUG
	va_list ap;
	
	va_start(ap, format);
	int n = vfprintf(stdout, format, ap);
	fprintf(stdout, "\n");
	va_end(ap);
	
	return n;
#else
	return 0;
#endif
}
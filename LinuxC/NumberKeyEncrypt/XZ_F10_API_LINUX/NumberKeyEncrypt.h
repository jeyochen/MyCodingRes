#pragma once

#include <string>
using namespace std;

class CNumberKeyEncrypt
{
public:
	~CNumberKeyEncrypt();
	
	static CNumberKeyEncrypt* GetInstance()
	{
		if (NULL == m_pInstance)
		{
			m_pInstance = new CNumberKeyEncrypt();
		}
		return m_pInstance;
	}

protected:
	CNumberKeyEncrypt();
	
protected:
	void DestroyInstance()
	{
		if (NULL != m_pInstance)
		{
			delete m_pInstance;
			m_pInstance = NULL;
		}
	}
	
public:
	/*************************************************
	* 功能：设置串口端口和波特率
	* 入参：iPort 串口号
	*       lBaud 波特率
	* 返回：true设置成功，false设置失败
	***************************************************/
	bool SetSerialParam(int iPort, long lBaud = 9600);
	
	/*************************************************
	* 功能：获取当前串口端口和波特率
	* 出参：iPort 串口号
	*       lBaud 波特率
	***************************************************/
	void GetSerialParam(int &iPort, long &lBaud);
	
	/*************************************************
	* 功能：设置工作密钥和MAC密钥
	* 入参：strData 工作密码和MAC密钥
	* 返回：true密钥设置成功， false 密钥设置失败
	**************************************************/
	bool SetWorkKey(const char *szData);
	
	/*************************************************
	* 功能：根据加密类型，加密从键盘输入的内容
	* 入参：iType 加密类型 0：PIN加密， 1:MAC加密
	*       strData 参与加密的字符串
	* 返回：加密后的字符串
	**************************************************/
	const char* Encrypt(const int iType, const char *szData);
	
protected:
	// 输出日志信息
	int PrintLog(const char *format, ...);
	
private:
	static CNumberKeyEncrypt *m_pInstance;
	
private:
	int m_port;
	long m_baud;
	string m_portname;
};
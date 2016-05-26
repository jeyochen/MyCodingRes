#pragma once

#include <string>
using namespace std;

#define SOAPI extern "C" 

int m_port = -1;
int m_baud = 9600;
string m_portname;

/*************************************************
* 功能：设置串口端口和波特率
* 入参：iPort 串口号
*       lBaud 波特率
* 返回：true设置成功，false设置失败
* 备注：可供java和C++调用
***************************************************/
SOAPI bool SetSerialParam(int iPort, int iBaud = 9600);

/*************************************************
* 功能：获取当前串口端口和波特率
* 出参：iPort 串口号
*       lBaud 波特率
* 备注: C++调用
***************************************************/
SOAPI void GetSerialParam(int &iPort, int &iBaud);

/*************************************************
* 功能：设置工作密钥和MAC密钥
* 入参：strData 工作密码和MAC密钥
* 返回：true密钥设置成功， false 密钥设置失败
* 备注：可供java和C++调用
**************************************************/
SOAPI bool SetWorkKey(char* szData);

/*************************************************
* 功能：根据加密类型，加密从键盘输入的内容
* 入参：iType 加密类型 0：PIN加密， 1:MAC加密
*       strData 参与加密的字符串
* 返回：加密后的字符串
* 备注：可供java和C++调用
**************************************************/
SOAPI const char* Encrypt(int iType, char* szData);

// 输出日志信息，内部调用，不对外开放
int PrintLog(const char *format, ...);

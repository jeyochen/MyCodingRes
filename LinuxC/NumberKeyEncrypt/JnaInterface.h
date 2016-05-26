#pragma once

#define EXPORT_SOA_PI extern "C"

/*************************************************
* 功能：设置串口端口和波特率
* 入参：iPort 串口号
*       lBaud 波特率
* 返回：true设置成功，false设置失败
***************************************************/
EXPORT_SOA_PI bool SetSerialParam(int iPort, long lBaud = 9600);

/*************************************************
* 功能：设置工作密钥和MAC密钥
* 入参：strData 工作密码和MAC密钥
* 返回：true密钥设置成功， false 密钥设置失败
**************************************************/
EXPORT_SOA_PI bool SetWorkKey(const char *szData);

/*************************************************
* 功能：根据加密类型，加密从键盘输入的内容
* 入参：iType 加密类型 0：PIN加密， 1:MAC加密
*       strData 参与加密的字符串
* 返回：加密后的字符串
**************************************************/
EXPORT_SOA_PI const char* Encrypt(const int iType, const char *szData);
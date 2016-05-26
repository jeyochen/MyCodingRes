#include "JnaInterface.h"
#include "NumberKeyEncrypt.h"

// 设置串口端口和波特率
bool SetSerialParam(int iPort, long lBaud)
{
	return CNumberKeyEncrypt::GetInstance()->SetSerialParam(iPort);
}

// 设置工作密钥和MAC密钥
bool SetWorkKey(const char *szData)
{
	return CNumberKeyEncrypt::GetInstance()->SetWorkKey(szData);
}

// 根据加密类型，加密从键盘输入的内容
const char* Encrypt(const int iType, const char *szData)
{
	return CNumberKeyEncrypt::GetInstance()->Encrypt(iType, szData);
}
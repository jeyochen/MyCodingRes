#ifndef __XZ_F30_API_H__
#define __XZ_F30_API_H__
extern "C"
{
	//打开串口
	int  SUNSON_OpenCom(const char * sztty, long nBaud);
	//关闭串口
	int  SUNSON_CloseCom(void);
	//获取按下的按键键值
	int  SUNSON_ScanKeyPress(unsigned char *ucKeyValue);
	//使用键盘仿真
	int  SUNSON_UseEppPlainTextMode(unsigned char ucTextModeFormat,unsigned char *ReturnInfo);
	//键盘复位自检，所有密钥复位
	int  SUNSON_ResetEPP(bool bInitKey = false);
	//下载主密钥
	int  SUNSON_LoadMasterKey (unsigned char ucMKeyId,unsigned char ucMKeyLen,unsigned char *NewMasterKey,unsigned char * ReturnInfo);
	//下载工作密钥
	int  SUNSON_LoadWorkKey(unsigned char ucMKeyId,unsigned char ucWKeyId,unsigned char ucWKeyLen,unsigned char* WorkKeyCiphertext,unsigned char *ReturnInfo);
	//激活工作密钥
	int  SUNSON_ActiveKey(unsigned char MasterKeyId,unsigned char ucWkeyId,unsigned char *ReturnInfo);
	//启动密码键盘加密
	int  SUNSON_StartEpp(unsigned char ucPinLen,unsigned char AlgorithmMode,unsigned char timeout,unsigned char *ReturnInfo);
	//取PIN密文
	int  SUNSON_ReadCypherPin(unsigned char *ReturnInfo);
	//下载账号
	int  SUNSON_LoadCardNumber(unsigned char* ucCardNumber,unsigned char *ReturnInfo);
	//下载终端号
	int  SUNSON_LoadTerminalNumber(unsigned char* ucTerminalNo,unsigned char *ReturnInfo);
	//数据加密运算
	int  SUNSON_DataEncrypt(unsigned  char ucDataLen,unsigned char* SourceData,unsigned char *ReturnInfo);
	//数据解密运算
	int  SUNSON_DataDecrypt(unsigned  char ucDataLen,unsigned char* SourceData,unsigned char *ReturnInfo);
	//数据mac运算
	int  SUNSON_MakeMac(unsigned char nMacDataLen,unsigned char* ucMacData,unsigned char *ReturnInfo);
	//回送字符
	int  SUNSON_SendAscII(unsigned char Ascii,unsigned char *ReturnInfo);
	//设置IC卡卡座及类型
	int  SUNSON_SetSimCardIdAndKind(unsigned char CardId,unsigned char CardKind,unsigned char *ReturnInfo);
	//获取IC卡卡座及类型
	int  SUNSON_GetSimCardIdAndKind(unsigned char CardId,unsigned char *ReturnInfo);
	//上电复位IC卡
	int  SUNSON_CardPowerOn(unsigned char *ReturnInfo);
	//IC卡下电
	int  SUNSON_CardPowerOff(unsigned char *ReturnInfo);
	//访问COS
	int  SUNSON_UseAPDU(unsigned  char len,unsigned char* apdu,unsigned char *ReturnInfo);
	//取PIN数据(用于sam加密)
	int  SUNSON_GetPinFromSIM(unsigned  char len,unsigned char* apdu,unsigned char *ReturnInfo);
	//写产品版本号
	int  SUNSON_SetVersion(unsigned char*version,unsigned char*SerialNo,unsigned char *ReturnInfo);
	//读取版本号
	int  SUNSON_GetVersion(unsigned char *version);
	//读产品终端号
	int  SUNSON_GetSerialNumber(unsigned char *SerialNumber);
	//写产品终端号
	int  SUNSON_SetSerialNumber(unsigned char *SerialNumber,unsigned char *ReturnInfo);
	//ubc mac 计算
	int  SUNSON_MakeUBCMac(int nMacDataLen,unsigned char* ucMacData,unsigned char *ReturnInfo,unsigned char *hexReturnInfo);
	//X9.19的ECB mac
	int  SUNSON_MakeX919ECBMac(int nMacDataLen,unsigned char* ucMacData,unsigned char *hexReturnInfo);
	//X9.9的ECB mac
	int  SUNSON_MakeX99ECBMac(int nMacDataLen,unsigned char* ucMacData,unsigned char *hexReturnInfo);
	//标准的 mac
	int  SUNSON_MakeBaseMac(int nMacDataLen,unsigned char* ucMacData,unsigned char *hexReturnInfo);
	////X9.9的CBC mac
	int  SUNSON_MakeX99CBCMac(int nMacDataLen,unsigned char* initvector,unsigned char* ucMacData,unsigned char *hexReturnInfo);
	///设置算法处理参数
	int  SUNSON_SetAlgorithmParameter(unsigned char ucPPara,unsigned char ucFPara,unsigned char *ReturnInfo);
	///认证命令 F11新加指令
	int  SUNSON_Authentication(unsigned char ucAuthFlag,unsigned char* ucCustomerData, unsigned char * ReturnInfo);
	//打开/关闭移出功能
	int  SUNSON_MoveCertification(unsigned char Movemode,unsigned char *errorinfo);
	//利用SAM卡下载工作密钥
	int  SUNSON_LoadWorkKeyBySAM(unsigned char APDULen,unsigned char MKeyID,unsigned char WKeyID, unsigned char * APDU,unsigned char *errorinfo);
};
#endif

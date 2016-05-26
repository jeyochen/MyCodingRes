#pragma once

/***************************************************************
*  金木雨JMY6073模块 电子钱包通信类
*  Create by 2016-04-06
****************************************************************/

#include <pthread.h>
#include <string>
#include <vector>

using namespace std;

typedef unsigned char uint8;

const int MAX_PATH = 128;
const int BUF_SIZE = 4096;
const int CMD_BUF_SIZE = 1024;

// 以下为自定义的错误码
const string PORT_NOT_OPEN = "FFFF"; //串口未打开
const string UNKNOW_ERROR = "EEEE"; // 位置错误
const string CLOSE_ANTE_FAILE = "1000"; //关闭天线失败
const string OPEN_ANTE_FAILE = "10001"; //打开天线失败
const string FIND_CARD_FAILE = "10002"; //寻卡失败
const string CARD_RESET_FAILE = "1003"; //卡复位失败
const string SEND_CMD_FAILE = "1004"; //发送命令失败
const string GET_AID_FAILE = "1005"; //获取AID失败

// 以下代码的定义来自于 
// 中国金融集成电路（IC）卡规范 第5部分：借记贷记应用卡片规范.pdf
// 19页 6.4.1目录选择方式
const string SUCCESS_CODE = "9000"; //命令执行成功
const string CARD_LOCK = "6A81"; //卡片锁定或者选择（SELECT）命令不支持
const string NO_PSE = "6A82"; //卡片中没有 PSE, 或 卡片不再有应用有此 AID
const string PSE_LOCK = "6283"; //PSE 锁定或应用锁定
const string NO_RECORD = "6A83"; //请求的记录不存在
const string NOT_SUPPORT = "6985"; //卡片不支持此应用

class CJMYPassPay
{
public:
    CJMYPassPay(void);
    ~CJMYPassPay(void);

public:
    /***********************************************************
    * 功能: 打开串口
    * 入参：port 串口编号
    * 出参：
    * 返回值: true 设置成功， false 设置失败
    *************************************************************/
    bool OpenSerial(int port);

    /************************************************************
    * 功能：发送命令
    *************************************************************/
    /******************************************
    * 功能: 写数据
    * 入参: srcCmd 待发送的命令
    * 出参:
	* 返回值: true 发送成功， false 发送失败
    *******************************************/
    bool WriteData(string &srcCmd);

    /************************************************************
    * 功能: 读数据
    * 入参: readBuff 存放读取结果的缓冲区
    *       readLen 要读取的字节数
    * 出参：
	* 返回值: 实际收到的字节数
    ************************************************************/
    int ReadData(uint8 *readBuff, int readLen);

    /************************************************************
    * 功能：关闭串口
    * 入参：
    * 出参：
    * 返回值: true 关闭成功， false 关闭失败
    ************************************************************/
    bool ClosePort();

    /************************************************************
    * 功能：串是否已打开
    * 入参：
    * 出参：
    * 返回值: true 已打开， false 未打开
    ************************************************************/
    bool IsOpenPort();

    /************************************************************
    * 功能：支付消费金额并返回55域内容
    * 入参: money 支付金额，单位为分 
    * 出参：field55 返回55域内容
    *       cardno 返回支付卡号
    *       sw1sw2 返回卡片响应状态字    
    * 返回：true 成功，false失败
    *************************************************************/
    bool Pay(string &field55, string &cardno, string &sw1sw2, const int money = 0);
	
	/************************************************************
	* 功能： 查询余额
    * 入参： 
	* 出数： balance 返回卡的余额，单位为分
    *        sw1sw2 返回卡片响应状态字 
	* 返回:  true 查询成功， false 查询失败
	*************************************************************/
	bool QueryBalance(string &balance, string &sw1sw2);

	/************************************************************
	* 功能： 读卡号
	* 入参：
    * 出参： cardno 返回卡号
    *        sw1sw2 返回卡片响应状态字 
	* 返回:  true 读取成功， false 读取失败
	*************************************************************/
	bool ReadCardNo(string &cardno, string &sw1sw2);
	
protected:	
    //  字符串转十六进制
    void StrToHex(uint8 *pbDest, const char *pbSrc, int nLen);

    // 十六进制转字符串
    void HexToStr(uint8 *pbDest, uint8 *pbSrc, int nLen);

    // 过滤空格
    int TrimSpace(char* pDest, const char *src, int nLen);

    // 计算BCC校验码
    uint8 CaculateBCC(uint8 *cmd, int len, uint8 &bcc);
	
	// 关支付模块的天线
	bool CloseAntenna(string &cmd);
	
	// 开支付模块的天线
	bool OpenAntenna(string &cmd);
	
	// 寻卡
	bool FindCard(string &cmd);
	
	// 复位
	bool RestCard(string &cmd);
	
	// 选择支付环境（PSE）
	bool SelectPSE(string &cmd, string &appidcmd, string &errcode);
	
	// 选应用ID
	bool SelectAppId(string &cmd, string &gpo, string &errcode, const int money = 0);
	
	// 发送GPO指令
	bool SendGpoCmd(string &cmd, string &errcode);

    // 根据引用列表选择
    bool SelectByAID(string &field55, string &cardno, string &sw1sw2, 
        const int money, string &aid);
	
	// 读取记录
	bool ReadRecords(const uint8 *readBuf, const int readLen, string &errcode);

    // 组装gpo指令
    bool GetGPOCmd(const uint8 *pdol, const int plen, string& gpo, const int money);

    // 从选择应用ID响应报文里查找指定tag
    bool GetValueFromAidRes(const uint8 *readBuf, const int readLen);

    // 从GPO响应报文里查找指定的tag
    bool GetValueFromGpoRes(const uint8 *readBuf, const int readLen);

    // 在选读记录命令的响应报文里查找9F74 tag标签的值
    bool Get9F74Value(const uint8 *readBuf, const int readLen);

    // 获取卡片的生效日期
    bool GetValidate(const uint8 *readBuf, const int readLen);

    // 获取卡片的失效日期
    bool GetInvalidate(const uint8 *readBuf, const int readLen);

    // 读取接口设备序列号
    bool Get9F1EValue(string &tag9F1E);

    // 组装银联55域的报文
    bool Get55Filed(string &filed55);

    // 在支付是获取卡号
    bool PayGetCardNo(string &cardno, string &sw1sw2);

    // 读取终端维护的应用列表
    void GetAPIDList();

    // 保存终端维护的应用列表
    void SaveAPIDList();

private:
    int m_fd; // 串口描述符

    // 以下为组织55域报文所需要的字段
    string m_Field9F26  ;  //应用密文：     
    string m_Field9F27  ;  //密文信息数据： 
    string m_Field9F10  ;  //发卡行应用数据:
    string m_Field9F37  ;  //不可预知数:    
    string m_Field9F36  ;  //应用交易计数器:
    string m_Field95    ;  //终端验证结果:  
    string m_Field9A    ;  //交易日期:      
    string m_Field9C    ;  //交易类型:      
    string m_Field9F02  ;  //授权金额:      
    string m_Field5F2A  ;  //交易货币代码:  
    string m_Field82    ;  //应用交互特征:  
    string m_Field9F1A  ;  //终端国家代码:  
    string m_Field9F03  ;  //其它金额:      
    string m_Field9F33  ;  //终端性能:      
    string m_Field9F35  ;  //接口设备序列号:
    string m_Field9F1E  ;  //专用文件名称:  
    string m_Field84    ;  //应用版本号:    
    string m_Field9F09  ;  //交易序列计数器:
    string m_Field9F41  ;  //终端类型:      
    string m_Field9F74  ;  //电子现金发卡行授权码:
    string m_Field8A    ;  //授权响应码: 

    string m_Validate; // 生效日期
    string m_Invalidate; // 失效日期
    string m_CurrDate; // 当前日期

    // 候选AppId列表
    vector<string> m_vecAppId;

    pthread_t m_thid;
};


#pragma once

/*******************************************************
* 智能照明控制器
* create by Chenjiyou On 2015-05-13
*********************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>

#include <string>

#include "../common/typedef.h"

class CSixLine
{
public:
    CSixLine(void);
    ~CSixLine(void);

public:
    /****************************************** 
    * 功能: 打开串口
    * 参数: port: 串口端口号, 从0开始编号
    * 返回值: true 打开成功， false 打开失败
    *******************************************/
    bool OpenSerialPort(uint8 port);

    /******************************************
    * 功能: 设置串口波特率
    * 参数: buad 波特率,可选 38400, 19200, 9600,
    *       4800,2400, 1200, 300, 38400, 19200, 
    *       9600, 4800, 2400,1200, 300  
    * 返回值: true 设置成功， false 设置失败
    *******************************************/
    bool SetBaudrate(int buad);

    /******************************************
    * 功能: 设置奇偶校验
    * 参数: databits 数据位数 7 或 8 位
    *       stopbits 停止位 1 或 2 位
    *       parity 奇偶校验 'n', 'N' 无奇偶校验
    *              'o' 或 'O' 奇校验
    *              'e' 或 'E' 偶校验
    *              's' 或 'S' space 奇偶校验
	* 返回值: true 设置成功，false设置失败
    *******************************************/
    bool SetParity(uint8 databits, uint8 stopbits, uint8 parity);

    /******************************************
    * 功能: 读数据
    * 参数: pbuf 存放读取结果的缓冲区
    *       len  预读取的字节数，确保len不超过pbuf的容量
    *       timeout 读取超时时间，默认1秒
	* 返回值: 实际收到的字节数
    *******************************************/
    int ReadData(unsigned char *pbuf, const int len, const uint8 timeout = 1);

    /******************************************
    * 功能: 写数据
    * 参数: pbuf 待发送的数据缓冲区
    *       len  待发送的字节数
    *       timeout 发送超时时间，默认1秒
	* 返回值: true 发送成功， false 发送失败
    *******************************************/
    bool WriteData(const char *pbuf, const int len, const uint8 timeout = 1);

    /******************************************
    * 功能: 关闭串口
	* 返回值:true关闭成功，false 关闭失败
    *******************************************/
    bool ClosePort();
	
protected:
    // 创建发送线程
    //int StartSendThread();

    // 创建接收线程
    //int StartReceiveThread();
	
	// 计算16位CRC码
	void CRC16_Modbus(const uint8 *ptr, uint8 len, uint8 crc[2]);

private:
    int m_fd; // 串口描述符
    //pthread_t m_RecvTid;  //发送线程句柄
    //pthread_t m_SendTid;  //接收线程句柄
	
	// 字符串转16进制
	void StrToHex(uint8 *pbDest, const char *pbSrc, int nLen);
	
	// 16进制转字符串
	void HexToStr(uint8 *pbDest, uint8 *pbSrc, int nLen);
	
	// 过滤空格
	int TrimSpace(char* pDest, const char *src, int nLen);
};


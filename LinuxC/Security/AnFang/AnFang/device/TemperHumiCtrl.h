#pragma once

/*******************************************************
* 温湿度采集器及空调遥控器
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

class CTemperHumiCtrl
{
public:
    CTemperHumiCtrl(void);
    ~CTemperHumiCtrl(void);

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
    //  串口是否打开
    bool IsOpen();
    // 计算CRC校验码
    void CRC16_Modbus(const uint8 *ptr, uint8 len, uint8 crc[2]);
    // 字符串转16进制
    void StrToHex(uint8 *pbDest, const char *pbSrc, int nLen);
    // 十六机制转字符串
    void HexToStr(uint8 *pbDest, uint8 *pbSrc, int nLen);
    // 删除空格
    int TrimSpace(char* pDest, const char *src, int nLen);
private:
    int m_fd; // 串口描述符
};


#pragma once

/*******************************************************
* iTrust Adapter1-3k UPS
* create by Chenjiyou On 2015-05-17
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

class CUpsDevice
{
public:
    CUpsDevice(void);
    ~CUpsDevice(void);

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
    bool SetBaudrate(uint16 buad);

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
    * 参数: addr 设备地址
    *       cid2  命令信息：控制标识码
    *             响应信息： 返回码RTN 
    *       context 待发送的内容
    *       contextLen 待发送的内容的长度
	* 返回值: true 发送成功， false 发送失败
    *******************************************/
    bool WriteData(uint8 addr, uint8 cid2, const char *pbuf, uint16 len, 
        const uint8 timeout = 1);

    /******************************************
    * 功能: 关闭串口
	* 返回值:true关闭成功，false 关闭失败
    *******************************************/
    bool ClosePort();

protected:
    //  串口是否打开
    bool IsOpen();
    // 字符串转16进制
    void StrToHex(uint8 *pbDest, const char *pbSrc, int nLen);
    // 十六机制转字符串
    void HexToStr(uint8 *pbDest, uint8 *pbSrc, int nLen);
    // 删除空格
    int TrimSpace(char* pDest, const char *src, int nLen);

    // 计算LENGHT，4位LCHKSUM， 12位LENID
    uint16 CalcLength(uint16 len);

    // 计算CHKSUM
    uint16 CalcChksum(const uint8 *context, int len);

    // 解析告警状态
    bool ParseAlarmStatus(const uint8 *pbuf, int len);
private:
    int m_fd; // 串口描述符
};


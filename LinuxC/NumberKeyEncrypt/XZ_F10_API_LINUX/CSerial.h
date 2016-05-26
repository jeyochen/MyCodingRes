/*
 * CSerial.h
 *
 *  Created on: Nov 23, 2011
 *      Author: xwq
 */

#ifndef CSERIAL_H_
#define CSERIAL_H_
#include 	 <pthread.h>
#include 	 <time.h>
#include 	 <stdio.h>
#include     <sys/types.h>
#include     <sys/stat.h>
#include     <fcntl.h>      /*文件控制定义*/
#include     <termios.h>    /*PPSIX 终端控制定义*/
#include     <errno.h>      /*错误号定义*/
#include 	 <string>
#include "CCriticalSection.h"

#define      BUFFER_LENGTH 10*1024
#define 	 CMD_SUCCESS  1    //成功
#define      CMD_FAILS    0    //失败


using namespace std;
namespace SUNSON_CSerial {
class CSerial
{
private:
	string m_PortName;
	int  m_baudrate;
	int m_databits;
	int  m_stopbits;
	char m_parity;
     //通讯线程标识符ID
     pthread_t      m_thread;
     //创建互斥变量（临界区）
     CCriticalSection  mutex;

     struct timeval TimeOut; //串口读写超时
public:
     CSerial();
     virtual ~CSerial();
     // 已打开的串口文件描述符
     int        		m_fd;
     int        		m_DatLen;
     unsigned char  	DatBuf[BUFFER_LENGTH];
     int        		m_ExitThreadFlag;

     // 按照指定的串口参数打开串口,并创建串口接收线程
     int OpenPort(const char * nPordId, int baudrate);
     // 关闭串口并释放相关资源
     int ClosePort(void);
     // 向串口写数据
     int Write_Comm(unsigned char* Buf, int len);
     //读数据到串口
     int Read_Comm( unsigned char* Buf, int len );
     //设定超时
     int SetTimeOut(int val);
     //清除输入输出缓冲区数据
     int ClearTxRx(void);
     // 接收串口数据处理函数
//     virtual int PackagePro( unsigned char* Buf, int len );
private:
	void set_speed(int fd, int speed);
	int  set_Parity(int fd,int databits,int stopbits,char parity);
};

}

#endif /* CSERIAL_H_ */

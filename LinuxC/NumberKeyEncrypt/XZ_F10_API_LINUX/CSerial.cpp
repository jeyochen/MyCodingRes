/*
 * CSerial.cpp
 *
 *  Created on: Nov 23, 2011
 *      Author: xwq
 */
#include <sys/select.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>     /*Unix 标准函数定义*/
#include <string.h>
#include "CSerial.h"
#include "CLog.h"

static CLog   g_Log;

namespace SUNSON_CSerial {
int speed_arr[] = {B115200,B38400, B19200, B9600, B4800, B2400, B1200, B300, B38400,
		B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {115200,38400, 19200, 9600, 4800, 2400, 1200, 300, 38400, 19200,
		9600, 4800, 2400, 1200, 300, };

CSerial::CSerial()
{
	m_ExitThreadFlag = false;
	m_PortName = "/dev/ttyS0";
	m_baudrate = 9600;
	m_databits = 8;
	m_stopbits = 1;
	m_parity = 'n';
}

CSerial::~CSerial()
{
	ClosePort();
}

int CSerial::OpenPort(const char * strPortId, int baudrate)
{
	/*以读写方式打开串口*/
	int arrFd[6] = {0};
	m_fd = open(strPortId, O_RDWR);
	int cnt = 0;
	if (-1 == m_fd)
	{
		g_Log.WriteLog("打开串口:%s 失败", strPortId);
		return CMD_FAILS;
	}
	while(3 > m_fd)
	{
		sleep(1);
		arrFd[cnt++] = m_fd;
		g_Log.WriteLog("重新打开串口");
		m_fd = open(strPortId, O_RDWR);
		if (-1 == m_fd)
		{
			g_Log.WriteLog("打开串口:%s 失败", strPortId);
			return CMD_FAILS;
		}
	}
	
	// for (int i = 0; i < cnt; i++)
	// {
		// g_Log.WriteLog("关闭fd:%d", arrFd[i]);
		// close(arrFd[i]);
	// }
	
	if (-1 == m_fd)
	{
		/* 不能打开串口一*/
		perror(" 提示错误！");
		return CMD_FAILS;
	}
	else
	{
		m_PortName = strPortId;
		m_baudrate = baudrate;
		//设置参数
		set_speed(m_fd, baudrate);
		set_Parity(m_fd, m_databits, m_stopbits, m_parity);
		SetTimeOut(2);
		g_Log.WriteLog("串口句柄:%d", m_fd);
		return CMD_SUCCESS;
	}
}
int CSerial::ClosePort( )
{
	 m_ExitThreadFlag = true;
	 close(m_fd);
	 return CMD_SUCCESS;
}
/*
 *@brief  设置串口通信速率
 *@param  fd     类型 int  打开串口的文件句柄
 *@param  speed  类型 int  串口速度
 *@return  void
 */
void CSerial::set_speed(int fd, int speed) {
	unsigned int i;
	int status;
	struct termios Opt;
	tcgetattr(fd, &Opt);
	for (i = 0; i < sizeof(speed_arr) / sizeof(int); i++) {
		if (speed == name_arr[i]) {
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			if (status != 0) {
				perror("tcsetattr fd1");
				return;
			}
			tcflush(fd, TCIOFLUSH);
		}
	}
}
/**
 *@brief   设置串口数据位，停止位和效验位
 *@param  fd     类型  int  打开的串口文件句柄
 *@param  databits 类型  int 数据位   取值 为 7 或者8
 *@param  stopbits 类型  int 停止位   取值为 1 或者2
 *@param  parity  类型  int  效验类型 取值为N,E,O,,S
 */
int CSerial::set_Parity(int fd, int databits, int stopbits, char parity) {
	struct termios options;
	if (tcgetattr(fd, &options) != 0) {
		perror("SetupSerial 1");
		return (false);
	}
	options.c_cflag &= ~CSIZE;
	switch (databits) /*设置数据位数*/
	{
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "Unsupported data size\n");
		return (false);
	}
	switch (parity) {
	case 'n':
	case 'N':
		options.c_cflag &= ~PARENB; /* Clear parity enable */
		options.c_iflag &= ~INPCK; /* Enable parity checking */
		break;
	case 'o':
	case 'O':
		options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
		options.c_iflag |= INPCK; /* Disnable parity checking */
		break;
	case 'e':
	case 'E':
		options.c_cflag |= PARENB; /* Enable parity */
		options.c_cflag &= ~PARODD; /* 转换为偶效验*/
		options.c_iflag |= INPCK; /* Disnable parity checking */
		break;
	case 'S':
	case 's': /*as no parity*/
		options.c_cflag &= ~PARENB;
		options.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported parity\n");
		return (false);
	}
	/* 设置停止位*/
	switch (stopbits) {
	case 1:
		options.c_cflag &= ~CSTOPB;
		break;
	case 2:
		options.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr, "Unsupported stop bits\n");
		return (false);
	}

	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN);  /*Input*/

	options.c_iflag  &= ~(IXON | IXOFF | IXANY);//关流控
	options.c_iflag  &= ~(INLCR | IGNCR | ICRNL | IUCLC | IGNBRK|BRKINT|PARMRK|ISTRIP);

	options.c_oflag  &= ~(ONLCR | OCRNL | ONLRET | ONOCR | OLCUC | OFILL | CRTSCTS);
	options.c_oflag  &= ~OPOST;   /*Output*/

	/* Set input parity option */
	if (parity != 'n')
		options.c_iflag |= INPCK;
	tcflush(fd, TCIFLUSH);
	options.c_cc[VTIME] = 1; /* 设置超时15 seconds*/
	options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		perror("SetupSerial 3");
		return (false);
	}
	return (true);
}
int CSerial::Write_Comm( unsigned char* Buf, int len )
{
	mutex.Lock();
	int Writelen = 0;
	Writelen = write(m_fd,Buf,len);
	mutex.Unlock();
	return Writelen;
}

int CSerial::Read_Comm( unsigned char* Buf, int len )
{
		mutex.Lock();
		int ToRead = len;
		int hasRead = 0;
		int wPos = 0;
		while(ToRead > 0){
			hasRead = read(m_fd, Buf + wPos, ToRead);
			if(hasRead <= 0) break;
			ToRead -= hasRead;
			wPos += hasRead;
		}
		mutex.Unlock();
		return len - ToRead;
}

int CSerial::SetTimeOut(int val)
{
	fd_set fdRead;
	FD_ZERO(&fdRead);
	FD_SET(m_fd,&fdRead);
	int ret = -1;
	if(m_fd > 0)
	{
		TimeOut.tv_sec = 0;
		TimeOut.tv_usec = val*50;
		ret = select(m_fd + 1, &fdRead, NULL, NULL, &TimeOut);
	}
	return ret;
}

int CSerial::ClearTxRx(void)
{
	if (tcflush(m_fd, TCIOFLUSH) < 0)
		return CMD_FAILS;
	else
		return CMD_SUCCESS;
}

}

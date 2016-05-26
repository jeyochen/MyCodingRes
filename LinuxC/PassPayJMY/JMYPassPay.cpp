#include "JMYPassPay.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include <fstream>

static uint8 g_readBuff[BUF_SIZE] = {0}; 
static short g_readLen = 0;

// 对串口的线程是否需要退出
static bool  g_isExit;

// 信号量
static sem_t g_sem;

// 输出日志的函数
static int PrintLog(const char *format, ...)
{
    int n  = 0;
    FILE *fp = NULL;
    time_t now;
    struct tm *timenow;
    char sLogFile[MAX_PATH + 1] = "";
    static  char sbuf[4096] = {0}; //  log输出缓冲区
    
    time(&now);
    timenow = localtime(&now);
    sprintf(sLogFile, "./logPassPay%04d%02d%02d.txt",
        1900 + timenow->tm_year, timenow->tm_mon
        + 1, timenow->tm_mday);

    fp = fopen(sLogFile, "a+");
    if (fp)
    {
        va_list ap;
        va_start(ap, format);
        
        // 取当前时间精确到微秒
        timeval usec;
        gettimeofday(&usec, NULL);
        snprintf(sbuf, sizeof(sbuf), "[%04d-%02d-%02d %02d:%02d:%02d.%03d]", 1900
            + timenow->tm_year, timenow->tm_mon + 1, timenow->tm_mday,
            timenow->tm_hour, timenow->tm_min, timenow->tm_sec, usec.tv_usec/1000);

        n = vsprintf(&sbuf[strlen(sbuf)], format, ap);
        va_end(ap);
#ifdef _OUTFILE
        fprintf(fp, "%s\n", sbuf);
#endif

#ifdef _OUTSCREEN
        fprintf(stdout, "%s\n", sbuf);
#endif

#ifdef _BOTHLOG
        fprintf(fp, "%s\n", sbuf);
        fprintf(stdout, "%s\n", sbuf);
#endif
        fclose(fp);
    }
    return n;
}

// 同步读串口数据的线程，等待一次读数据的完成
bool GetReadRes()
{
    sem_wait(&g_sem);
    return true;
}

void *ReadThread(void *arg)
{
   
    int *pint = (int *)arg;
    int fd = *pint;

    int pos  = 0;
    int readlen = 0;
    fd_set read_fds;
    struct timeval tv;

    for (;;)
    {
        if (g_isExit)
        {
            PrintLog("读串口数据线程退出\n");
            break;
        }

        tv.tv_sec = 0;
        tv.tv_usec = 50000; // 监听周期为50毫秒
        FD_ZERO(&read_fds);
        FD_SET(fd, &read_fds);
        int nfds = select(fd+1, &read_fds, NULL, NULL, &tv);
        if (nfds < 0)
        {
            PrintLog("select 监听出错,读数据线程退出");
            usleep(10);
            continue;
        }
        else if (0 == nfds)
        {
            usleep(10);
            continue;
        }

        int nread = read(fd, &g_readBuff[pos], sizeof(g_readBuff) - pos - 1);
        if (nread > 0)
        {
            pos += nread;
        }

        // 取长度
        if (pos >= 2)
        {
            readlen |= g_readBuff[0] << 8;
            readlen |= g_readBuff[1];
        }

        // 已读完输出内容
        if ((pos == (readlen + 1)) && (pos > 2))
        {
            g_readLen = pos;
            g_readLen = readlen + 1;
            pos = 0;
            readlen = 0;
            sem_post(&g_sem);
        }
    }

    return NULL;
}

CJMYPassPay::CJMYPassPay(void)
	:m_fd(-1)
{
    m_Field9F27 = "0100";
    m_Field95 = "050000000000";
    m_Field9C = "0100";
    m_Field5F2A = "020156";
    m_Field9F1A = "020156";
    m_Field9F03 = "06000000000000";
    m_Field9F33 = "03000000";
    m_Field9F09 = "020000";
    m_Field9F41 = "020000";
    m_Field9F35 = "0100";
    m_Field8A = "025931";
    GetAPIDList();
}


CJMYPassPay::~CJMYPassPay(void)
{
    ClosePort();
    SaveAPIDList();
    sem_destroy(&g_sem);
}


bool CJMYPassPay::OpenSerial(int port)
{
    if (IsOpenPort())
    {
        return true;
    }
    g_isExit  = false;
    // 打开串口
    char port_path[MAX_PATH] = {0};

    // 正式环境用的是串口，测试环境用的是USB转串口
#ifdef _NORMAL_ENV
    snprintf(port_path, sizeof(port_path), "/dev/ttyS%d", port);
#else
    snprintf(port_path, sizeof(port_path), "/dev/ttyUSB%d", port);
#endif
    
    m_fd = open(port_path, O_RDWR|O_NOCTTY|O_NDELAY);
    if (-1 == m_fd)
    {
		PrintLog("Open %s failed! %s:%d", port_path, __FILE__, __LINE__);
        return false;
    }

    int flags = fcntl(m_fd, F_GETFL, 0);
    fcntl(m_fd, F_SETFL, flags | O_NONBLOCK | O_NDELAY);

    // 设置波特率为19200
	struct termios opt;
    if (tcgetattr(m_fd, &opt) != 0 )
    {
        perror("SetBaudrate tcgetattr fail!");
        return false;
    }
	
    int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300, B38400, B19200, B9600, B4800, B2400, B1200, B300};
    int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200, 300, 38400, 19200, 9600, 4800, 2400, 1200, 300};
    int i = 0;
    int arr_len = sizeof(name_arr)/sizeof(int);
	
    for (i = 0; i < arr_len; i++)
    {
        if (19200 == name_arr[i])
        {
            tcflush(m_fd, TCSANOW);
            cfsetispeed(&opt, speed_arr[i]);
            cfsetospeed(&opt, speed_arr[i]);

            if (tcsetattr(m_fd, TCSADRAIN, &opt) != 0)
            {
                PrintLog("SetBaudrate tcsetattr fail! %s:%d", __FILE__, __LINE__);
                return false;
            }
            break;
        }
    }
    if (i >= arr_len)
    {
        PrintLog("SetBaudrate failed! %s:%d", __FILE__, __LINE__);
        return false;
    }
    
    if(tcgetattr(m_fd, &opt) != 0)
    {
        PrintLog("SetParity tcgetattr fail. %s:%d", __FILE__, __LINE__);
        return false;
    }

    // 设置数据位位8位
    opt.c_cflag |= (CLOCAL | CREAD);
    opt.c_cflag &= ~CSIZE;
    opt.c_cflag |= CS8;

    // 设置无奇偶校验
    opt.c_cflag &= ~PARENB;
    opt.c_iflag &= ~INPCK;

    // 设置1位停止位
    opt.c_cflag &= ~CSTOPB; 

    // 这两个选项可以保证程序不会变成端口的所有者,而端口所有者必须去处理
    // 发散性作业控制和挂断信号,同时还保证了串行接口驱动会读取过来的数据字节.
    opt.c_cflag |= (CLOCAL | CREAD);

    //  不用输入回车即可发送字符
    opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // input
    opt.c_oflag &= ~OPOST; // output

    opt.c_oflag &= ~(ONLCR | OCRNL);
    opt.c_iflag &= ~(ICRNL | INLCR);
    opt.c_iflag &= ~(IXON | IXOFF | IXANY);
    opt.c_iflag &= ~ (INLCR | ICRNL | IGNCR);

    tcflush(m_fd, TCIFLUSH);
    opt.c_cc[VTIME] = 10; // 设置超时0.1 seconds , 单位为百毫秒
    opt.c_cc[VMIN] = 0;

    if(tcsetattr(m_fd, TCSANOW, &opt) != 0)
    {
        PrintLog("tcsetattr fd. %s:%d", __FILE__, __LINE__);
        return false;
    }

    if (0 != pthread_create(&m_thid, NULL, ReadThread, (void *)(&m_fd)))
    {
        PrintLog("创建读线程失败！");
    }
    PrintLog("创建成功的线程ID：%0X", m_thid);
    return true;
}

bool CJMYPassPay::WriteData(string &srcCmd)
{	
	if (-1 == m_fd)
    {
		PrintLog("写数据失败，串口已关闭。");
        return false;
    }
	
    // 判断读线程是否存在
    if (ESRCH == pthread_kill(m_thid, 0))
    {
        PrintLog("线程已退出,重启读数据线程");
        pthread_create(&m_thid, NULL, ReadThread, (void *)(&m_fd));
    }

    static char srcBuff[CMD_BUF_SIZE] = {0}; // 存放原始字符串
    static uint8 sendBuf[CMD_BUF_SIZE] = {0}; // 存储待发送的十六进制命令
    static char  tempBuf[CMD_BUF_SIZE] = {0}; // 临时存放去除空格的命令字符串

    int srcLen = srcCmd.length(); // 待发送的命令长度
    if (0 >= srcLen || srcLen > CMD_BUF_SIZE - 4) // 预留两个字节的长度和1个自己的BCC校验码
    {
		PrintLog("cmd length must low then %d", CMD_BUF_SIZE - 4);
        return false;
    }

    memcpy(srcBuff, srcCmd.c_str(), srcLen);
    memset(tempBuf, 0x00, sizeof(tempBuf));
    int tempLen = TrimSpace(tempBuf, srcBuff, srcLen);
    if (tempLen % 2 != 0)
    {
		PrintLog("cmd len must mod 2 == 0");
        return  false;
    }

    // 计算命令串的总长度，不包括一个字节的BCC长度,命令长度 +  两个字节的长度。
    memset(sendBuf, 0x00, sizeof(sendBuf));
    StrToHex(&sendBuf[3], tempBuf, tempLen);

    int cmdLen = tempLen / 2;
    // 加上两个字节的长度,加一个字节的设备地址
    cmdLen += 3;
    sendBuf[0] = cmdLen & 0XFF00;
    sendBuf[1] = cmdLen & 0X00FF;
    sendBuf[2] = 0X00;

    // 计算BCC校验码
    uint8 bcc = 0;
    CaculateBCC(sendBuf, cmdLen, bcc);

    // 命令尾部加上bcc校验码
    sendBuf[cmdLen] = bcc;
    cmdLen += 1;

    char temp[CMD_BUF_SIZE];
    memset(temp, 0x00, sizeof(temp));
    for (int i = 0; i < cmdLen; i++)
    {
        sprintf(temp + i * 3, "%02X ", sendBuf[i]);
    }
    srcCmd = temp;

    int left_len = cmdLen; // 待发送的命令长度
	
    int nfds = 0; // 可写句柄数量
    fd_set write_fds; // 句柄集合
    struct timeval tv;  // select 等待时间
    tv.tv_sec = 0;
    tv.tv_usec = 1;

    FD_ZERO(&write_fds);
    FD_SET(m_fd, &write_fds);

    uint8 *p = sendBuf;
    while (left_len > 0)
    {
        nfds = select(m_fd + 1, NULL, &write_fds, NULL, &tv);
        if (nfds < 0)
        {
            break;
        }
        else if(0 == nfds)
        {
            //PrintLog("no fd use write!");
            break;
        }

        int nwrite = write(m_fd, p, left_len);
        if (nwrite > 0)
        {
            left_len -= nwrite;
            p += nwrite;
        }
        else
        {
            break;
        }
    }
    if (left_len != 0)
    {
        return false;
    }

    return true;
}


int CJMYPassPay::ReadData(uint8 *readBuff, int readLen)
{
	GetReadRes();
    int len = readLen > g_readLen ? g_readLen : readLen;
    memcpy(readBuff, g_readBuff, readLen);
    return len;
}

bool CJMYPassPay::ClosePort()
{
    if(-1 != m_fd)
    {
        // 关闭端口
        close(m_fd);
        m_fd = -1;
        
        // 通知线程关闭
        g_isExit = true;
        pthread_join(m_thid, NULL);
    }

    return true;
}

bool CJMYPassPay::QueryBalance(string &balance, string &sw1sw2)
{
    int readLen = 0;
    string cmd;
    static char temp[BUF_SIZE] = {0};
	
    //关天线
    cmd = "11 00";
    if (false == CloseAntenna(cmd))
    {
        sw1sw2 = CLOSE_ANTE_FAILE;
        return false;
    }
    //开天线
    cmd = "11 01";
    if (false == OpenAntenna(cmd))
    {
        sw1sw2 = OPEN_ANTE_FAILE;
        return false;
    }
	// 寻卡
    cmd = "2000";
    if (false == FindCard(cmd))
	{
		return false;
	}
    // 复位
    cmd = "30";
    if (false == RestCard(cmd))
	{
		return false;
	}
    // 选择PSE
	string app_id_cmd;
    cmd = "31 00 A4 04 00 0E 32 50 41 59 2E 53 59 53 2E 44 44 46 30 31";
    if (false == SelectPSE(cmd, app_id_cmd, sw1sw2))
    {
        return false;
    }
    // 选应用ID
	string gpo;
    if (false == SelectAppId(app_id_cmd, gpo, sw1sw2))
	{
		return false;
	}
	// 查询余额
	cmd = "31 80CA9F79";
    memset(g_readBuff, 0X00, sizeof(g_readBuff));
	if (false == WriteData(cmd))
	{
		PrintLog("send cmd:%s failed", cmd.c_str());
		return false;
	}

    GetReadRes();
    readLen = g_readLen;
	
    memset(temp, 0x00, sizeof(temp));
    for (int i = 0; i < readLen; i++)
    {
        sprintf(temp + i * 3, "%02X ", g_readBuff[i]);
    }
	PrintLog("send cmd:%s\n recv:%s", cmd.c_str(), temp);

	for (int i = 0; i < readLen; i++)
	{
		if (0X9F == g_readBuff[i] && 0X79 == g_readBuff[i+1])
		{
			int money_len = g_readBuff[i+2];
			
			int start = i + 3;
			memset(temp, 0x00, sizeof(temp));
			for (int j = 0; j < money_len; j++)
			{
				sprintf(temp + j * 2, "%02X ", g_readBuff[start++]);
			}
			balance = temp;
			break;
		}
	}
    sw1sw2 = SUCCESS_CODE;
	return true;
}

// 读卡号
bool CJMYPassPay::ReadCardNo(string &cardno, string &sw1sw2)
{
	string cmd;
    int readLen = 0;
    static char temp[BUF_SIZE] = {0};

    timeval usec1;
    gettimeofday(&usec1, NULL);
    //关天线
    cmd = "11 00";
    if (false == CloseAntenna(cmd))
    {
        sw1sw2 = CLOSE_ANTE_FAILE;
        return false;
    }

    //开天线
    cmd = "11 01";
    if (false == OpenAntenna(cmd))
    {
        sw1sw2 = OPEN_ANTE_FAILE;
        return false;
    }

	// 寻卡
    cmd = "2000";
    if (false == FindCard(cmd))
	{
        sw1sw2 = FIND_CARD_FAILE;
		return false;
	}

    // 复位
    cmd = "30";
    if (false == RestCard(cmd))
	{
        sw1sw2 = CARD_RESET_FAILE;
		return false;
	}

    // 选择PSE
	string app_id_cmd;
    cmd = "31 00 A4 04 00 0E 32 50 41 59 2E 53 59 53 2E 44 44 46 30 31";
    if (false == SelectPSE(cmd, app_id_cmd, sw1sw2))
    {
        return false;
    }
   
    // 选应用ID
	string gpo;
    if (false == SelectAppId(app_id_cmd, gpo, sw1sw2))
	{
		return false;
	}
	
	// 读取卡号的命令
	cmd = "31 00 B2 01 0C";
    memset(g_readBuff, 0, sizeof(g_readBuff));
	if (false == WriteData(cmd))
	{
		PrintLog("Send cmd:%s failed!", cmd.c_str());
		return false;
	}

    GetReadRes();
    readLen = g_readLen;
    memset(temp, 0x00, sizeof(temp));
    for (int i = 0; i < readLen; i++)
    {
        sprintf(temp + i * 2, "%02X", g_readBuff[i]);
    }
	PrintLog("Send cmd:%s\n, recv:%s", cmd.c_str(), temp);

    string str_card = temp;
    int pos = str_card.find("5712");
    if (pos > 0)
    {
        cardno = str_card.substr(pos + 4,19);
    }
	PrintLog("cardno:%s", cardno.c_str());
    sw1sw2 = SUCCESS_CODE;

    timeval usec2;
    gettimeofday(&usec2, NULL);
    int msec = (usec2.tv_sec * 1000 + usec2.tv_usec/1000) - (usec1.tv_sec * 1000 +  usec1.tv_usec/1000);
    PrintLog("***********elapse=%.2f**********", msec/1000.0);
	return true;
}

bool CJMYPassPay::PayGetCardNo(string &cardno, string &sw1sw2)
{
    string cmd;
    int readLen = 0;
    static char temp[BUF_SIZE] = {0};

    // 读取卡号的命令
    cmd = "31 00 B2 01 0C";
    memset(g_readBuff, 0, sizeof(g_readBuff));
    if (false == WriteData(cmd))
    {
        PrintLog("Send cmd:%s failed!", cmd.c_str());
        return false;
    }

    GetReadRes();
    readLen = g_readLen;
    memset(temp, 0x00, sizeof(temp));
    for (int i = 0; i < readLen; i++)
    {
        sprintf(temp + i * 2, "%02X", g_readBuff[i]);
    }
    PrintLog("Send cmd:%s\n, recv:%s", cmd.c_str(), temp);

    string str_card = temp;
    int pos = str_card.find("5712");
    if (pos > 0)
    {
        cardno = str_card.substr(pos + 4,19);
    }
    PrintLog("cardno:%s", cardno.c_str());
    sw1sw2 = SUCCESS_CODE;
    return true;
}

bool CJMYPassPay::Pay(string &field55, string &cardno, string &sw1sw2, const int money)
{
    if (-1 == m_fd)
    {
		PrintLog("Serial port was closed");
		sw1sw2 = PORT_NOT_OPEN;
        return false;
    }
    else if (0 >= money || money >= 100000)
    {
        PrintLog("金额应该大于零小于100000(分),当前为：%d(分)", money);
        return false;
    }

    timeval usec1;
    gettimeofday(&usec1, NULL);
    int readLen = 0;
    string cmd;
    static char temp[BUF_SIZE] = {0};

    //关天线
    cmd = "11 00";
	if (false == CloseAntenna(cmd))
	{
		sw1sw2 = CLOSE_ANTE_FAILE;
        PrintLog("关天线失败");
		return false;
	}

    //开天线
    cmd = "11 01";
    if (false == OpenAntenna(cmd))
	{
		sw1sw2 = OPEN_ANTE_FAILE;
        PrintLog("开天线失败");
		return false;
	}

    // 寻卡
    cmd = "2000";
    if (false == FindCard(cmd))
	{
		sw1sw2 = FIND_CARD_FAILE;
        PrintLog("寻卡线失败");
		return false;
	}

    // 复位
    cmd = "30";
    if (false == RestCard(cmd))
	{
		sw1sw2 = CARD_RESET_FAILE;
        PrintLog("复位失败");
		return false;
	}

    // 选择PSE 用支付系统目录方法进行应用选择
	string app_id_cmd;
    cmd = "31 00 A4 04 00 0E 32 50 41 59 2E 53 59 53 2E 44 44 46 30 31";
    if (false == SelectPSE(cmd, app_id_cmd, sw1sw2))
	{
        // 如果IC卡上没有PSE 那么IC卡应该对PSE的选择（SELECT）命令回送状态字“6A82”
        // 如果PSE被锁定，IC卡应该回送状态字“6283”
        // 这两种情况都要转为列表方式选择
        if (sw1sw2 == NO_PSE || sw1sw2 == PSE_LOCK)
        {
            vector<string>::iterator iter = m_vecAppId.begin();
            for (int i = 0; i < m_vecAppId.size(); ++i)
            {
                string aid = m_vecAppId[i];
                SelectByAID(field55, cardno, sw1sw2, money, aid);
                if (sw1sw2 == SUCCESS_CODE)
                {
                    break;
                }
            }
            if (sw1sw2 != SUCCESS_CODE)
            {
                return false;
            }
            return true;
        }
        PrintLog("选择PSE失败");
		return false;
	}

    // 选应选择
	string gpo;
    if (false == SelectAppId(app_id_cmd, gpo, sw1sw2, money))
	{
		PrintLog("send cmd:%s failed", cmd.c_str());
        PrintLog("选择应用失败");
		return false;
	}

    // 应用初始化
    cmd = gpo;
    PrintLog("gpo:%s", gpo.c_str());
    memset(g_readBuff, 0, sizeof(g_readBuff));
	int read_len = 0;
	if(false == SendGpoCmd(cmd, sw1sw2))
	{
        PrintLog("发送gpo失败");
		return false;
	}

    // 读取应用数据
    if (false == ReadRecords(g_readBuff, g_readLen, sw1sw2))
	{
        PrintLog("读记录失败");
		return false;
	}

    // 处理限制 已经在读记录做了。
    // 1.失效日期检查
    // 2. 失效日期检查

    // 终端风险管理
    // 1. 新卡检查

    // 终端行为分析
    // 1.联机脱机决定

    // 交易结束


    Get55Filed(field55);
   
    // 读卡号
    //if (false == PayGetCardNo(cardno, sw1sw2))
    //{
    //    return false;
    //}
	
    timeval usec2;
    gettimeofday(&usec2, NULL);
    int msec = (usec2.tv_sec * 1000 + usec2.tv_usec/1000) - (usec1.tv_sec * 1000 +  usec1.tv_usec/1000);
	PrintLog("***********elapse=%.2f**********", msec/1000.0);

    sw1sw2 = SUCCESS_CODE;
    return true;
}

// 关支付模块的天线
bool CJMYPassPay::CloseAntenna(string &cmd)
{
	if (false == IsOpenPort())
	{
		PrintLog("Serial port was closed");
		return false;
	}
	
    if (false == WriteData(cmd))
	{
		PrintLog("Send cmd:%s failed", cmd.c_str());
		return false;
	}

    GetReadRes();
    memset(g_readBuff, 0, sizeof(g_readBuff));
	return true;
}

// 开支付模块的天线
bool CJMYPassPay::OpenAntenna(string &cmd)
{
	if (false == IsOpenPort())
	{
		PrintLog("Serial port was closed");
		return false;
	}
	
	if (false == WriteData(cmd))
	{
		PrintLog("Send cmd:%s failed", cmd.c_str());
		return false;
	}

    GetReadRes();
    memset(g_readBuff, 0, sizeof(g_readBuff));
	return true;
}

// 寻卡
bool CJMYPassPay::FindCard(string &cmd)
{
	static uint8 readBuf[BUF_SIZE] = {0};

    int readLen = 0;
    static char temp[BUF_SIZE] = {0};
	if (false == IsOpenPort())
	{
		PrintLog("Serial port was closed");
		return false;
	}
	
	if (false  == WriteData(cmd))
	{
		PrintLog("Send cmd:%s failed", cmd.c_str());
		return false;
	}

    GetReadRes();
    readLen = g_readLen;

    // 寻卡失败
    int timeout = 0;
    while (readLen < 6)
    {
        cmd = "2000";
        if (false == WriteData(cmd))
		{
			PrintLog("Send cmd:%s failed", cmd.c_str());
			return false;
		}
        sleep(1);
        memset(readBuf, 0x00, sizeof(readBuf));
        GetReadRes();
        //readBuf = g_readBuff;
        readLen = g_readLen;
        // 20秒内寻卡失败，则返回失败
        if (++timeout > 20)
        {
			PrintLog("Find Card Timeout");
            return false;
        }
    }

    memset(temp, 0x00, sizeof(temp));
	
    for (int i = 0; i < readLen; i++)
    {
        sprintf(temp + i * 3, "%02X ", g_readBuff[i]);
    }
	PrintLog("send cmd:%s\n recv:%s", cmd.c_str(), temp);
	
	return true;
}

// 复位
bool CJMYPassPay::RestCard(string &cmd)
{
	static uint8 readBuf[BUF_SIZE] = {0};

    int readLen = 0;
    static char temp[BUF_SIZE] = {0};
	if (false == IsOpenPort())
	{
		PrintLog("Serial port was closed");
		return false;
	}
	
	if (false == WriteData(cmd))
	{
		PrintLog("send cmd:%s failed", cmd.c_str());
		return false;
	}
    
    GetReadRes();
    readLen = g_readLen;

    memset(temp, 0x00, sizeof(temp));
    for (int i = 0; i < readLen; i++)
    {
        sprintf(temp + i * 3, "%02X ", g_readBuff[i]);
    }
	PrintLog("send cmd:%s\n recv:%s", cmd.c_str(), temp);

	return true;
}

// 选择支付环境（PSE）
bool CJMYPassPay::SelectPSE(string &cmd, string &appidcmd, string &errcode)
{
	static uint8 readBuf[BUF_SIZE] = {0};

    int readLen = 0;
    static char temp[BUF_SIZE] = {0};
	if (false == IsOpenPort())
	{
		PrintLog("Serial port was closed");
        errcode = PORT_NOT_OPEN;
		return false;
	}
	
	if (false == WriteData(cmd))
	{
		PrintLog("send cmd:%s failed", cmd.c_str());
        errcode = SEND_CMD_FAILE;
		return false;
	}

    GetReadRes();
    readLen = g_readLen;

    memset(temp, 0x00, sizeof(temp));
    for (int i = 0; i < readLen; i++)
    {
        sprintf(temp + i * 3, "%02X ", g_readBuff[i]);
    }
	PrintLog("send cmd:%s\n recv:%s", cmd.c_str(), temp);

	// 判断PSE选择是否成功
	if (readLen > 3)
	{
		char sw1sw2[6] = {0};
		snprintf(sw1sw2, sizeof(sw1sw2), "%02X%02X", g_readBuff[readLen-3], g_readBuff[readLen-2]);
		if (sw1sw2 == CARD_LOCK)
		{
			//如果卡片锁定或者选择（SELECT）命令不支持，卡片响应 SW1 SW2=“6A81”
			PrintLog("卡片锁定或者选择（SELECT）命令不支持，cmd:%s", cmd.c_str());
			errcode = CARD_LOCK;
			return false;
		}
		else if(sw1sw2 == NO_PSE)
		{
			//如果卡片中没有 PSE，卡片响应选择（SELECT）命令指出文件不存在（SW1 SW2=“6A82”）；
			PrintLog("卡片中没有 PSE: cmd:%s", cmd.c_str());
            errcode = NO_PSE;
			return false;
		}
		else if(sw1sw2 == PSE_LOCK)
		{
			//如果 PSE 锁定，卡片响应“6283”；
			PrintLog("PSE 锁定,cmd:%s", cmd.c_str());
			errcode =  PSE_LOCK;
			return false;
		}
		else if (sw1sw2 != SUCCESS_CODE)
		{
			// 只有返回9000才为PSE选择成功
			PrintLog("PSE选择失败,cmd:%s", cmd.c_str());
			errcode = UNKNOW_ERROR;
			return false;
		}
	}

    // 寻找DF名称，在4F后面,即下一步要选择的应用ID
    int i = 0;
    for(; i < readLen; i++)
    {
        if (0X4F == g_readBuff[i])
        {
            char app_id[256] = {0};
            int len = g_readBuff[i + 1]; // ID的长度
            int startPos = i + 1;
            for (int j = 0; j <= len; j++)
            {
                sprintf(&app_id[j * 3], "%02X ", g_readBuff[startPos + j]);
            }
            // 删除最后一个空格
            app_id[strlen(app_id) - 1] = 0;
            appidcmd = app_id;            
            break;
        }
    }
    if (i >= readLen)
    {
		PrintLog("Get appId failed");
        errcode  = GET_AID_FAILE;
        return false;
    }
	return true;
}

// 选应用ID
bool CJMYPassPay::SelectAppId(string &cmd, string &gpo, string &errcode, const int money)
{
	static uint8 readBuf[BUF_SIZE] = {0};

    int readLen = 0;
    static char temp[BUF_SIZE] = {0};

    string appId = cmd;
    cmd = std::string("31 00 A4 04 00 ") + cmd;
	
	if (false == IsOpenPort())
	{
		PrintLog("Serial port was closed");
        errcode = PORT_NOT_OPEN;
		return false;
	}
	
	if (false == WriteData(cmd))
	{
		PrintLog("send cmd:%s failed", cmd.c_str());
		return false;
	}

    GetReadRes();
    readLen = g_readLen;
	
    memset(temp, 0x00, sizeof(temp));
    for (int i = 0; i < readLen; i++)
    {
        sprintf(temp + i * 3, "%02X ", g_readBuff[i]);
    }
	PrintLog("send cmd:%s\n recv:%s", cmd.c_str(), temp);

	if (readLen > 3)
	{
        char sw1sw2[6] = {0};
        snprintf(sw1sw2, sizeof(sw1sw2), "%02X%02X", g_readBuff[readLen-3], g_readBuff[readLen-2]);
		//如果卡片锁定或不支持选择（SELECT）命令，卡片响应状态字 SW1 SW2=“6A81”
		if (sw1sw2 == CARD_LOCK)
		{
			PrintLog("如果卡片锁定或不支持选择命令:%s\nerrcode:%s", cmd.c_str(), sw1sw2);
            errcode = CARD_LOCK;
			return false;
		}
		else if (sw1sw2 == NO_PSE)
		{
			//当卡片不再有应用有此 AID，卡片响应“6A82”表明所有匹配的应用都已经选择
			PrintLog("不再有应用有此 AID:s", cmd.c_str());
            errcode = NO_PSE;
			return false;
		}
        else if (sw1sw2 == PSE_LOCK)
        {
            PrintLog("应用锁定 AID:s", cmd.c_str());
            errcode = PSE_LOCK;
            return false;
        }
        else if (sw1sw2 != SUCCESS_CODE)
        {
            // 只有返回9000才为PSE选择成功
            PrintLog("PSE选择失败,cmd:%s", cmd.c_str());
            errcode = UNKNOW_ERROR;
            return false;
        }
	}

    // 把AppId 加入终端支持的AppId列表
    bool bFind = false;
    for (int i =0; i < m_vecAppId.size(); ++i)
    {
        string tmpAppId = m_vecAppId[i];
        if (appId == tmpAppId)
        {
            bFind = true;
            break;
        }
    }
    if (false == bFind)
    {
        m_vecAppId.push_back(appId);
        PrintLog("******************appid to vector:[%s]***************", appId.c_str());
    }

    if ( false == GetValueFromAidRes(g_readBuff, readLen))
    {
		PrintLog("从选择应用ID里找Value失败!");
        return false;
    }

    // 寻找PDOL,标签为9F38;
    for (int i = 0; i < readLen; i++)
    {
        if (0X9F == g_readBuff[i] && 0X38 == g_readBuff[i + 1])
        {
            // PDOL读取正确，开始组建GPO命令
            uint8 strPDOL[BUF_SIZE] = {0};
            int pdolLen = g_readBuff[i+2];
            pdolLen = sizeof(strPDOL) > pdolLen ? pdolLen : sizeof(strPDOL);
            memcpy(strPDOL, g_readBuff + i + 3, pdolLen);
            
            GetGPOCmd(strPDOL, pdolLen, gpo, money);
            break;
        }
    }
    if (0 >= gpo.length())
    {
		PrintLog("Generic gpo failed!");
        return false;
    }
	return true;
}


bool CJMYPassPay::SelectByAID(string &field55, string &cardno, string &sw1sw2, 
    const int money, string &aid)
{
    static uint8 readBuf[BUF_SIZE] = {0};

    int readLen = 0;
    string cmd;
    static char temp[BUF_SIZE] = {0};

    // 选应用ID
    string gpo;
    if (false == SelectAppId(aid, gpo, sw1sw2, money))
    {
        PrintLog("send cmd:%s failed", aid.c_str());
        return false;
    }

    cmd = gpo;
    //发送GPO指令
    PrintLog("gpo:%s", gpo.c_str());
    memset(readBuf, 0x00, sizeof(readBuf));
    int read_len = 0;
    if(false == SendGpoCmd(cmd, sw1sw2))
    {
        return false;
    }

    if (false == ReadRecords(readBuf, read_len, sw1sw2))
    {
        return false;
    }

    Get55Filed(field55);

    // 读卡号
    if (false == PayGetCardNo(cardno, sw1sw2))
    {
        return false;
    }

    sw1sw2 = SUCCESS_CODE;
    return true;
}

// 发送GPO指令
bool CJMYPassPay::SendGpoCmd(string &cmd, string &errcode)
{
    int readLen = 0;
    static char temp[BUF_SIZE] = {0};
	
	if (false == IsOpenPort())
	{
		PrintLog("Serial port was closed");
		return false;
	}
	
    memset(g_readBuff, 0, sizeof(g_readBuff));
	if (false == WriteData(cmd))
	{
		PrintLog("send cmd:%s failed", cmd.c_str());
		return false;
	}

    GetReadRes();
    readLen = g_readLen;

    memset(temp, 0x00, sizeof(temp));
    for (int i = 0; i < readLen; i++)
    {
        sprintf(temp + i * 3, "%02X ", g_readBuff[i]);
    }
	PrintLog("send cmd:%s\n recv:%s", cmd.c_str(), temp);

    // 检查GPO指令是否执行成功
    if (readLen > 3)
    {
        char sw1sw2[6] = {0};
        snprintf(sw1sw2, sizeof(sw1sw2), "%02X%02X", g_readBuff[readLen-3], g_readBuff[readLen-2]);
        if (sw1sw2 != SUCCESS_CODE)
        {
            errcode = NOT_SUPPORT;
            return false;
        }
    }

    if (false == GetValueFromGpoRes(g_readBuff, readLen))
    {
		PrintLog("从GPO响应命令里读取tag值失败");
        return false;
    }

	return true;
}

// 读取记录
bool CJMYPassPay::ReadRecords(const uint8 *readBuf, const int readLen, string &errcode)
{
    static char temp[BUF_SIZE] = {0};
    static uint8 initBuf[BUF_SIZE] = {0};

    int initLen = g_readLen;
    memcpy(initBuf, g_readBuff, g_readLen);
	
	// 找出应用文件定位器，tag为94，取出短文件标识符读记录
    int i = 0;
    for (; i < initLen; i++)
    {
        if (0X94 == initBuf[i])
        {
            // 找到了短文件标识符,获取短文件标识符的长度
            int sfiLen = initBuf[i + 1];
            if (0 != sfiLen % 4)
            {
                // 短文件标识符长度有错
				PrintLog("Find SFI length error!");
                return false;
            }

            // 把短文件标识符读出
            int iStart = i + 2;
            memset(temp, 0x00, sizeof(temp));
            for (int l = 0; l < sfiLen; l++)
            {
                sprintf(&temp[l*3], "%02X ", initBuf[iStart++]);
            }
			PrintLog("SFI:%s", temp);

            // 短文件标识符每四个字节为一组，分别是：短文件标示，第一记录编号，最后一条记录编号，存放用于脱机数据认证的数据的连续记录个数
            iStart = i + 2; // 指向第一组数据
            int iGroup  = sfiLen / 4;
            bool bRead9F74 = false;
            bool bRead5F25 = false;
            bool bRead5F24 = false;

            for (int j = 0; j < iGroup; j++)
            {
                // 开始处理一组数据
                uint8 sfi = initBuf[iStart + (j * 4)]; // sfi
                uint8 first = initBuf[iStart + (j * 4) + 1]; // start record
                uint8 last = initBuf[iStart + (j * 4) + 2]; // end record
                uint8 otherRec = initBuf[iStart + (j * 4) + 3];

                uint8 iReadType = sfi | 0X04; //读指定标识符的记录 0100

                int readRecLen  = 0;
                char hexBuf[4] = {0};
                string  readRecCmd;
                static uint8 readRecBuff[4096] = {0};
                for(int k = first; k <= last; k++)
                {
                    readRecCmd = "31 00 B2 ";
                    memset(hexBuf,0X00, sizeof(hexBuf));
                    sprintf(hexBuf, "%02X ", k); // 指定要读取的记录编号
                    readRecCmd += hexBuf;
                    
                    memset(hexBuf,0X00, sizeof(hexBuf));
                    sprintf(hexBuf, "%02X ", iReadType); // 指定读取的方式为读取当前记录
                    readRecCmd += hexBuf;
                   
                    memset(g_readBuff, 0x00, sizeof(g_readBuff));
                    PrintLog("record cmd:%s", readRecCmd.c_str());
                    if ( false == WriteData(readRecCmd))
                    {
						PrintLog("Send cmd:%s filed", readRecCmd.c_str());
                        return false;
                    }

                    GetReadRes();
                    readRecLen = g_readLen;

                    memset(temp, 0x00, sizeof(temp));
                    for (int i = 0; i < readRecLen; i++)
                    {
                        sprintf(temp + i * 3, "%02X ", g_readBuff[i]);
                    }
                    PrintLog("send cmd:%s\n recv:%s", readRecCmd.c_str(), temp);
					
					if (g_readLen > 3)
					{
                        char sw1sw2[6] = {0};
                        snprintf(sw1sw2, sizeof(sw1sw2), "%02X%02X", g_readBuff[g_readLen-3], g_readBuff[g_readLen-2]);
                        //PrintLog("************sw1sw = %s************", sw1sw2);
						//请求的记录不存在，卡片返回 SW1 SW2=“6A83”
						if(sw1sw2 == NO_RECORD)
						{
							PrintLog("命令:%s 读记录失败!", readRecCmd.c_str());
                            errcode = NO_RECORD;
                            continue;
						}
                        else if (sw1sw2 !=  SUCCESS_CODE)
                        {
                            PrintLog("发生了未知的错误:sw1sw2=%s", sw1sw2);
                            errcode = UNKNOW_ERROR;
                            continue;
                        }
					}
					

                    // 读取 卡行授权码
                    if (false == bRead9F74)
                    {
                        bRead9F74 = Get9F74Value(g_readBuff, readLen);
                    }

                    // 读取卡片生效日期
                    if (false ==  bRead5F25)
                    {
                        bRead5F25 = GetValidate(g_readBuff, readLen);
                        if(bRead5F25)
                        {
                            int currDateValue =  atoi(m_CurrDate.c_str());
                            // 生效日期检查
                            if (m_Validate.length() > 0)
                            {
                                int validateValue = atoi(m_Validate.c_str());
                                PrintLog("生效日期：%d", validateValue);
                                if (validateValue > currDateValue)
                                {
                                    PrintLog("卡片未生效");
                                    return false;
                                }
                            }
                        }
                    }

                    // 读取卡片失效日期
                    if (false == bRead5F24)
                    {
                        bRead5F24 = GetInvalidate(g_readBuff, readLen);
                        if (bRead5F24)
                        {
                            // 失效日期检查
                            int currDateValue =  atoi(m_CurrDate.c_str());
                            int invalidateValue = atoi(m_Invalidate.c_str());
                            PrintLog("失效日期：%d", invalidateValue);
                            if (currDateValue > invalidateValue)
                            {
                                PrintLog("卡片已失效");
                                return false;
                            }
                        }
                    }
                }
            }
            if (false == bRead9F74)
            {
                // 所有记录都读完毕了，还没有读9F74则认为是失败
				PrintLog("Find tag9F74 value failed!");
                return false;
            }
			break;
        }
    }
    if (i >= initLen)
    {
        PrintLog("读记录时，查找短文件标识符失败");
        return false;
    }
	return true;
}
	
// 组装gpo指令
bool CJMYPassPay::GetGPOCmd(const uint8 *pdol, const int plen, string &gpo, const int money)
{
    string strCmd = "";
    int  cmdLen = 0;

    int i = 0;
    // 9F66 终端交易属性 4  个字节
    for (i = 0; i < plen; i++)
    {
        if (0X9F == pdol[i] && 0X66 == pdol[i+1])
        {
            strCmd += "36000000";
            cmdLen += 4;
            break;
        }
    }

    // DF60 交易指示位 1个字节， 0：表示终端不支持扩展应用
    for (i = 0; i < plen; i++)
    {
        if (0XDF == pdol[i] && 0X60 == pdol[i+1])
        {
            strCmd += "00";
            cmdLen += 1;
            break;
        }
    }

    // 9F02 授权金额  占六个字节
    for (i = 0; i < plen; i++)
    {
        if (0X9F == pdol[i] && 0X02 == pdol[i+1])
        {
            char smoney[64] = {0};
            sprintf(smoney, "%012d", money);
            strCmd += smoney;
            m_Field9F02 = "06";
            m_Field9F02 += smoney;
            cmdLen += 6;
            break;
        }
    }

    // 9F03 其它金额 占六个字节
    for (i = 0; i < plen; i++)
    {
        if (0X9F == pdol[i] && 0X03 == pdol[i+1])
        {
            strCmd += "000000000000";
            cmdLen += 6;
            break;
        }
    }

    // 9F1A 终端国家代码占两个字节
    //gpo += "0156 ";
    for (i = 0; i < plen; i++)
    {
        if (0X9F == pdol[i] && 0X1A == pdol[i+1])
        {
            strCmd += "0156";
            cmdLen += 2;
            break;
        }
    }

    // 95 终端验证结果占5个字节
    //gpo += "0000000000 ";
    for (i = 0; i < plen; i++)
    {
        if (0X95 == pdol[i])
        {
            strCmd += "0000000000";
            cmdLen += 5;
            break;
        }
    }

    // 5F2A 交易货币代码占2个字节
    //gpo += "0156 ";
    for (i = 0; i < plen; i++)
    {
        if (0X5F == pdol[i] && 0X2A == pdol[i+1])
        {
            strCmd += "0156";
            cmdLen += 2;
            break;
        }
    }

    // 9A 交易日期 占三个字节
    for (i = 0; i < plen; i++)
    {
        if (0X9A == pdol[i])
        {
            time_t t;
            struct tm *st = 0;
            char curr_time[32] = {0};
            t = time(0);
            st = localtime(&t);
            sprintf(curr_time, "%04d%02d%02d", 
                st->tm_year + 1900, st->tm_mon + 1, st->tm_mday);

            string short_date = curr_time;
            short_date = short_date.substr(2, short_date.length() - 2);
            strCmd += short_date;
            m_CurrDate = short_date;
            m_Field9A = "03" + short_date;
            cmdLen += 3;
            break;
        }
    }

    //9C 交易类型占一个字节
    for (i = 0; i < plen; i++)
    {
        if (0X9C == pdol[i])
        {
            strCmd += "22";
            cmdLen += 1;
            break;
        }
    }

    //9F37  不可预知数，取随机数 占四个字节
    srand((unsigned)time(NULL));
    char random[4] = {0};
    for (int i = 0; i < 4; i++)
    {
        sprintf(random, "%02X", rand() % 0XFF);
        strCmd += random;
    }
    cmdLen += 4;

    char cmdHead[250] = {0};
    snprintf(cmdHead, sizeof(cmdHead), "31 80 A8 00 00 %02X 83 %02X", cmdLen + 2, cmdLen);
    gpo = cmdHead + strCmd;
    return true;
}

// 从选择应用ID响应报文里查找指定tag
bool CJMYPassPay::GetValueFromAidRes(const uint8 *readBuf, const int readLen)
{
    bool bGetRes = false;
    m_Field9F37 = "";
    m_Field84 = "";
    for (int i = 0; i < readLen; i++)
    {
        if (0X9F == readBuf[i] && 0X37 == readBuf[i + 1])
        {
            int iStart = i + 2;
            int len = readBuf[iStart]; // 9F37tag后面的内容长度
            char ch[4] = {0};
            for(int j = 0; j <= len; j++) // 连同长度一起取出来
            {
                memset(ch, 0x00, sizeof(ch));
                sprintf(ch, "%02X", readBuf[iStart + j]);
                m_Field9F37 += ch;
            }
            i += len;
        }
        else if (0X84 == readBuf[i])
        {
            int iStart = i + 1;
            int len = readBuf[iStart]; // tag82后面的内容长度
            char ch[4] = {0};
            for(int j = 0; j <= len; j++) // 连同长度一起取出来
            {
                memset(ch, 0x00, sizeof(ch));
                sprintf(ch, "%02X", readBuf[iStart + j]);
                m_Field84 += ch;
            }
            i += len;
        }
    }
    if (m_Field9F37.length() > 0 && m_Field84.length() > 0)
    {
        bGetRes = true;
    }
    
    return bGetRes;
}

// 从响应报文里查找指定的tag
bool CJMYPassPay::GetValueFromGpoRes(const uint8 *readBuf, const int readLen)
{
    bool bGetRes = false;

    m_Field9F26 = "";
    m_Field9F10 = "";
    m_Field9F36 = "";
    m_Field82 = "";
    for (int i = 0; i < readLen; i++)
    {
        if (0X9F == readBuf[i] && 0X26 == readBuf[i + 1])
        {
            int iStart = i + 2;
            int len = readBuf[iStart]; // 9F26tag后面的内容长度
            char ch[4] = {0};
            for(int j = 0; j <= len; j++) // 连同长度一起取出来
            {
                memset(ch, 0x00, sizeof(ch));
                sprintf(ch, "%02X", readBuf[iStart + j]);
                m_Field9F26 += ch;
            }
            // 跳过已读取过的部分
            i += len;
        }
        else if (0X9F == readBuf[i] && 0X10 == readBuf[i + 1])
        {
            int iStart = i + 2;
            int len = readBuf[iStart]; // 9F26tag后面的内容长度
            char ch[4] = {0};
            for(int j = 0; j <= len; j++) // 连同长度一起取出来
            {
                memset(ch, 0x00, sizeof(ch));
                sprintf(ch, "%02X", readBuf[iStart + j]);
                m_Field9F10 += ch;
            }
            // 跳过已读取过的部分
            i += len;
        }
        else if (0X9F == readBuf[i] && 0X36 == readBuf[i + 1])
        {
            int iStart = i + 2;
            int len = readBuf[iStart]; // 9F26tag后面的内容长度
            char ch[4] = {0};
            for(int j = 0; j <= len; j++) // 连同长度一起取出来
            {
                memset(ch, 0x00, sizeof(ch));
                sprintf(ch, "%02X", readBuf[iStart + j]);
                m_Field9F36 += ch;
            }
            // 跳过已读取过的部分
            i += len;
        }
        else if (0X82 == readBuf[i])
        {
            int iStart = i + 1;
            int len = readBuf[iStart]; // tag82后面的内容长度
            char ch[4] = {0};
            if (2 == len)
            {
                for(int j = 0; j <= len; j++) // 连同长度一起取出来
                {
                    memset(ch, 0x00, sizeof(ch));
                    sprintf(ch, "%02X", readBuf[iStart + j]);
                    m_Field82 += ch;
                }
                // 跳过已读取过的部分
                i += len;
            }
        }
    }

    if (m_Field9F26.length() > 0 && m_Field9F10.length() > 0  && 
        m_Field9F36.length() > 0 && m_Field82.length() > 0)
    {
        bGetRes = true;
    }
    return bGetRes;
}

// 在选读记录命令的响应报文里查找9F74 tag标签的值
bool CJMYPassPay::Get9F74Value(const uint8 *readBuf, const int readLen)
{
    bool bGetRes = false;
    // 查找 9F74 tag标签
    m_Field9F74 = "";
    for (int i = 0; i < readLen; i++)
    {
        if (0X9F == readBuf[i] && 0X74 == readBuf[i + 1])
        {
            int iStart = i + 2;
            int len = readBuf[iStart]; // 9F37tag后面的内容长度
            char ch[4] = {0};
            for(int j = 0; j <= len; j++) // 连同长度一起取出来
            {
                memset(ch, 0x00, sizeof(ch));
                sprintf(ch, "%02X", readBuf[iStart + j]);
                m_Field9F74 += ch;
            }
            bGetRes = true;
            break;
        }
    }

    return bGetRes;
}

// 获取卡片的生效日期
bool CJMYPassPay::GetValidate(const uint8 *readBuf, const int readLen)
{
    bool bGetRes = false;
    m_Validate = "";
    //卡片上应用生效日期（标签5F25）
    for (int i = 0; i < readLen; ++i)
    {
        if (0X5F == readBuf[i] && 0X25 == readBuf[i+1])
        {
            char tmpbuf[16] = {0};
            int start = i + 3;
            for (int j = 0; j < 3; ++j)
            {
                sprintf(&tmpbuf[j*2], "%02X", readBuf[start++]);
            }
            m_Validate = tmpbuf;
            bGetRes = true;
            break;
        }
    }
    return bGetRes;
}

// 获取卡片的失效日期
bool CJMYPassPay::GetInvalidate(const uint8 *readBuf, const int readLen)
{
    bool bGetRes = false;
    m_Invalidate = "";
    //应用失效日期（标签5F24）是卡片上必须存在的数据
    for (int i = 0; i < readLen; ++i)
    {
        if (0X5F == readBuf[i] && 0X24 == readBuf[i+1])
        {
            // 失效日期的长度
            char tmpbuf[16] = {0};
            int start = i + 3;
            for (int j = 0; j < 3; ++j)
            {
                sprintf(&tmpbuf[j*2], "%02X", readBuf[start++]);
            }
            PrintLog("卡片失效日期:%s", tmpbuf);
            m_Invalidate = tmpbuf;
            bGetRes = true;
            break;
        }
    }
    return bGetRes;
}

// 读取接口设备序列号
bool CJMYPassPay::Get9F1EValue(string &tag9F1E)
{
    bool bGetRes = false;
    uint8 recvBuf[BUF_SIZE] = {0};
    string cmd = "10";
	
    if (false == WriteData(cmd))
	{
		PrintLog("Send cmd:%s failed!", cmd.c_str());
		return bGetRes;
	}
    
    GetReadRes();
    int len = g_readLen;
	if (len < 12)
	{
		PrintLog("Get device serials failed");
		return false;
	}

	tag9F1E = "08";
	for (int i = 0; i < len; i++)
	{
		if (0X10 == g_readBuff[i])
		{
			int iStart =  i  + 1;
			int iEnd = iStart + 8;
			for (int j = iStart; j < iEnd; j++)
			{
				char buf[4] = {0};
				sprintf(buf, "%02X", g_readBuff[j]);
				tag9F1E += buf;
			}
			bGetRes = true;
		}
	}
        
    return bGetRes;
}

// 组装银联55域的报文
bool CJMYPassPay::Get55Filed(string &filed55)
{
	if (0 >= m_Field9F1E.length())
	{
		if (false  == Get9F1EValue(m_Field9F1E))
		{
			PrintLog("Query tag9F1E value failed");
			return false;
		}
	}
    
    filed55 = "9F26" + m_Field9F26  ;
    filed55 += "9F27" + m_Field9F27  ;
    filed55 += "9F10" + m_Field9F10  ;
    filed55 += "9F37" + m_Field9F37  ;
    filed55 += "9F36" + m_Field9F36  ;
    filed55 += "95" + m_Field95    ;
    filed55 += "9A" + m_Field9A    ;
    filed55 += "9C" + m_Field9C    ;
    filed55 += "9F02" + m_Field9F02  ;
    filed55 += "5F2A" + m_Field5F2A  ;
    filed55 += "82" + m_Field82    ;
    filed55 += "9F1A" + m_Field9F1A  ;
    filed55 += "9F03" + m_Field9F03  ;
    filed55 += "9F33" + m_Field9F33  ;
    filed55 += "9F35" + m_Field9F35  ;
    filed55 += "9F1E" + m_Field9F1E  ;
    filed55 += "84" + m_Field84    ;
    filed55 += "9F09" + m_Field9F09  ;
    filed55 += "9F41" + m_Field9F41  ;
    filed55 += "9F74" + m_Field9F74  ;
    filed55 += "8A" + m_Field8A    ;

    return true;
}

bool CJMYPassPay::IsOpenPort()
{
    return m_fd != -1;
}


void CJMYPassPay::StrToHex(uint8 *pbDest, const char *pbSrc, int nLen)
{
    char h1,h2;
    uint8 s1,s2;

    for (int i = 0; i < nLen/2; i++)
    {
        h1 = pbSrc[2*i];
        h2 = pbSrc[2*i + 1];

        s1 = toupper(h1) - 0x30;
        if (s1 > 9)
            s1 -= 7;

        s2 = toupper(h2) - 0x30;
        if (s2 > 9)
            s2 -= 7;

        pbDest[i] = s1 * 16 + s2;
    }
}

void CJMYPassPay::HexToStr(uint8 *pbDest, uint8 *pbSrc, int nLen)
{
    char ddl,ddh;
    for (int i = 0; i < nLen; i++)
    {
        ddh = 48 + pbSrc[i] / 16;
        ddl = 48 + pbSrc[i] % 16;
        if (ddh > 57) ddh = ddh + 7;
        if (ddl > 57) ddl = ddl + 7;
        pbDest[i*2] = ddh;
        pbDest[i*2+1] = ddl;
    }

    pbDest[nLen*2] = '\0';
}

int CJMYPassPay::TrimSpace(char *pDest, const char *src, int nLen)
{
    int j = 0;
    for (int i = 0; i < nLen; i++)
    {
        if(src[i] == 0x20)
        {
            continue;
        }
        pDest[j++] = src[i];
    }
    pDest[j] = 0;
    return j;
}


uint8 CJMYPassPay::CaculateBCC(uint8 *cmd, int len, uint8 &bcc)
{
    for (int i = 0; i < len; i++)
    {
        bcc ^= cmd[i];
    }

    return bcc;
}

// 读取终端维护的应用列表
void CJMYPassPay::GetAPIDList()
{
    m_vecAppId.clear();
    ifstream aidFile("./AID.txt", ios::in);
    if (aidFile.is_open())
    {
        while(!aidFile.eof())
        {
            char aid[1024] = {0};
            aidFile.getline(aid, sizeof(aid));
            if (strlen(aid) > 0)
            {
                m_vecAppId.push_back(aid);
            } 
        }
        aidFile.close();
    }

    string defAID = "08 A0 00 00 03 33 01 01 01";
    int i = 0;
    for (; i < m_vecAppId.size(); ++i)
    {
        //PrintLog("[%s],[%s]", defAID.c_str(), m_vecAppId[i].c_str());
        if (strstr(m_vecAppId[i].c_str(),defAID.c_str()))
        {
            break;
        }
    }

    // 添加默认的AID
    if (i >= m_vecAppId.size())
    {
        //PrintLog("***********Add Default AID");
        m_vecAppId.push_back(defAID);
    }
    
    return;
}

// 保存终端维护的应用列表
void CJMYPassPay::SaveAPIDList()
{
    ofstream aidFile("./AID.txt", ios::out|ios::trunc);
    if (aidFile.is_open())
    {
        for (int i = 0; i < m_vecAppId.size(); ++i)
        {
            string aid = m_vecAppId[i];
            aidFile<<aid<<endl;
            PrintLog("aid:%s", aid.c_str());
        }
        aidFile.close();
        m_vecAppId.clear();
    }
    return;
}

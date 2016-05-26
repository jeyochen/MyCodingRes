#include "SixLine.h"

#include "../MainApp.h"


CSixLine::CSixLine(void)
    :m_fd(-1)
{
}


CSixLine::~CSixLine(void)
{
    if(-1 != m_fd)
    {
        close(m_fd);
        m_fd = 0;
    }
}

bool CSixLine::OpenSerialPort(uint8 port)
{
    char port_path[128] = {0};
#ifdef _NORMAL_ENV
    snprintf(port_path, sizeof(port_path), "/dev/ttyS%d", port);
#else
    snprintf(port_path, sizeof(port_path), "/dev/ttyUSB%d", port);
#endif
    MainApp.m_log.WriteLog("只能照明控制器类打开串口:%s", port_path);

    m_fd = open(port_path, O_RDWR|O_NOCTTY|O_NDELAY);
    if (-1 == m_fd)
    {
        MainApp.m_log.WriteLog("%s:%d 打开文件失败", __FILE__, __LINE__);
        return false;
    }

    int flags = fcntl(m_fd, F_GETFL, 0);
    fcntl(m_fd, F_SETFL, flags | O_NONBLOCK | O_NDELAY);
    return true;
}

bool CSixLine::SetBaudrate(int buad)
{
    struct termios opt;
    if (tcgetattr(m_fd, &opt) != 0 )
    {
        MainApp.m_log.WriteLog("%s:%d 设置波特率失败", __FILE__, __LINE__);
        return false;
    }

    int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300, B38400, B19200, B9600, B4800, B2400, B1200, B300};
    int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200, 300, 38400, 19200, 9600, 4800, 2400, 1200, 300};


    // TCSANOW：立即执行而不等待数据发送或者接受完成。
    // TCSADRAIN：等待所有数据传递完成后执行。
    // TCSAFLUSH：Flush input and output buffers and make the change
    int i = 0;
    int arr_len = sizeof(name_arr)/sizeof(int);
    for (i = 0; i < arr_len; i++)
    {
        if (buad == name_arr[i])
        {
            tcflush(m_fd, TCSANOW);
            cfsetispeed(&opt, speed_arr[i]);
            cfsetospeed(&opt, speed_arr[i]);

            if (tcsetattr(m_fd, TCSADRAIN, &opt) != 0)
            {
                MainApp.m_log.WriteLog("%s:%d 设置波特率失败", __FILE__, __LINE__);
                return false;
            }
            break;
        }
    }
    if (i >= arr_len)
    {
        MainApp.m_log.WriteLog("%s:%d 设置波特率失败:%d", __FILE__, __LINE__, buad);
        return false;
    }

    return true;
}

bool CSixLine::SetParity(uint8 databits, uint8 stopbits, uint8 parity)
{
    struct termios opt;

    if(tcgetattr(m_fd, &opt) != 0)
    {
        MainApp.m_log.WriteLog("%s:%d 设置就校验失败", __FILE__, __LINE__);
        return false;
    }

    opt.c_cflag |= (CLOCAL | CREAD);

    switch (databits)
    {
    case 7:
        opt.c_cflag &= ~CSIZE; // 清除原来的字符位数
        opt.c_cflag |= CS7;
        break;
    case 8:
        opt.c_cflag &= ~CSIZE;
        opt.c_cflag |= CS8;
        break;
    default:
        break;
        return false;
    }

    switch (parity)
    {
    case 'n':
    case 'N': // 无奇偶校验
        opt.c_cflag &= ~PARENB; // 清除校验位
        opt.c_iflag &= ~INPCK; // enable parity checking
        break;
    case 'o':
    case 'O': // 奇校验
        opt.c_cflag |= PARENB; // 设置校验位
        opt.c_cflag |= PARODD; // 奇偶验
        opt.c_iflag |= INPCK; // disable parity checking
        break;
    case 'e':
    case 'E': // 偶校验
        opt.c_cflag |= PARENB; // 设置校验位
        opt.c_cflag &= ~PARODD; // 设置为偶验
        opt.c_iflag |= INPCK; // disable parity checking
        break;
    case 's':
    case 'S':
        opt.c_cflag &= ~PARENB; // 清除设置校验位验位
        opt.c_cflag &= ~CSTOPB; // 停止位为 1位
        opt.c_iflag |= INPCK; // disable parity checking
        break;
    default:
        MainApp.m_log.WriteLog("%s:%d 错误的奇偶校验设置项");
        return false;
    }

    switch(stopbits)        //设置停止位
    {
    case 1:
        opt.c_cflag &= ~CSTOPB; // 设置1位停止位
        break;
    case 2:
        opt.c_cflag |= CSTOPB;  // 设置两位停止位
        break;
    default:
        MainApp.m_log.WriteLog("%s:%d 不支持的停止位选项", __FILE__, __LINE__);
        return false;
    }

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
    opt.c_cc[VTIME] = 150; // 设置超时15 seconds , 单位为百毫秒
    opt.c_cc[VMIN] = 0;

    if(tcsetattr(m_fd, TCSANOW, &opt) != 0)
    {
        MainApp.m_log.WriteLog("%s:%d 设置奇偶校验失败", __FILE__, __LINE__);
        return false;
    }

    return true;
}

bool CSixLine::WriteData(const char *pbuf, const int len, const uint8 timeout)
{
	static uint8 send_buf[1024] = {0}; // 存储待发送的十六进制命令
	static char  temp_buf[1024] = {0}; // 临时存放去除空格的命令字符串
	
    int left_len = 0; // 待发送的命令长度
	
    int nfds = 0; // 可写句柄数量
    fd_set write_fds; // 句柄集合
    struct timeval tv;  // select 等待时间
	
	if (len > sizeof(send_buf))
	{
		MainApp.m_log.WriteLog("%s:%d 发送命令的长度必须小于 1024", __FILE__, __LINE__);
		return -1;
	}

	memset(temp_buf, 0x00, sizeof(temp_buf));
	int temp_len = TrimSpace(temp_buf, pbuf, len);
	if (temp_len % 2 != 0)
	{
		MainApp.m_log.WriteLog("%s:%d 命令的长度必须是2的整数倍", __FILE__, __LINE__);
		return false;;
	}
	//fprintf(stdout, "len=%d, cmd=%s\n", temp_len, temp_buf);
	
	memset(send_buf, 0x00, sizeof(send_buf));
	StrToHex(send_buf, temp_buf, temp_len);
	
	uint8 CRC[2];
	memset(CRC, 0, sizeof(CRC));
	CRC16_Modbus(send_buf, (uint16)(temp_len/2), CRC);
	//fprintf(stdout, "CRC[0]=%02X, CRC[1]=%02X\n", CRC[0], CRC[1]);

	// 待发送缓冲区加上两个字节的CRC码
	send_buf[temp_len/2] = CRC[0];
	send_buf[temp_len/2 + 1] = CRC[1];
	left_len = temp_len/2 + 2;
	
	uint8 *p = send_buf;
    while (left_len > 0)
    {
        tv.tv_sec = 0;
        tv.tv_usec = 500000;
        FD_ZERO(&write_fds);
        FD_SET(m_fd, &write_fds);

        nfds = select(m_fd + 1, NULL, &write_fds, NULL, &tv);
        if (nfds < 0)
        {
            break;
        }
        else if(0 == nfds)
        {
            //printf("no fd use write!\n");
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

int CSixLine::ReadData(unsigned char *pbuf, const int len, const uint8 timeout)
{
    int left_len = len;
    int nfds = 0;
    fd_set read_fds;
    struct timeval tv;
    
    while(left_len > 0)
    {
        tv.tv_sec = 0;
        tv.tv_usec = 500000;

        FD_ZERO(&read_fds);
        FD_SET(m_fd, &read_fds);

        nfds = select(m_fd + 1, &read_fds, NULL, NULL, &tv);
        if (nfds < 0)
        {
            return -1;
        }
        else if (0 == nfds)
        {
            //printf("no fd use read!\n");
            break;
        }
        
        int nread = read(m_fd, pbuf, left_len);
        if (nread > 0)
        {
            left_len -= nread;
            pbuf += nread;
        }
        else
        {
            break;
        }
    }

    return len - left_len;
}

bool CSixLine::ClosePort()
{
    close(m_fd);
    m_fd = -1;
    return true;
}

void CSixLine::CRC16_Modbus(const uint8 *ptr, uint8 len, uint8 crc[2])
{
  unsigned long wcrc=0XFFFF;//预置16位crc寄存器，初值全部为1
  unsigned char temp;//定义中间变量
  int i=0,j=0;//定义计数
  for(i=0;i<len;i++)//循环计算每个数据
  {
    temp=ptr[i]&0X00FF;//将八位数据与crc寄存器亦或
    //ptr++;//指针地址增加，指向下个数据
    wcrc^=temp;//将数据存入crc寄存器
    for(j=0;j<8;j++)//循环计算数据的
    {
      if(wcrc&0X0001)//判断右移出的是不是1，如果是1则与多项式进行异或。
      {
        wcrc>>=1;//先将数据右移一位
        wcrc^=0XA001;//与上面的多项式进行异或
      }
      else//如果不是1，则直接移出
      {
        wcrc>>=1;//直接移出
      }
    }
  }
  temp=wcrc;//crc的值
  crc[0]=wcrc;//crc的低八位
  crc[1]=wcrc>>8;//crc的高八位
}

void CSixLine::StrToHex(uint8 *pbDest, const char *pbSrc, int nLen)
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

void CSixLine::HexToStr(uint8 *pbDest, uint8 *pbSrc, int nLen)
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

int CSixLine::TrimSpace(char* pDest, const char *src, int nLen)
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


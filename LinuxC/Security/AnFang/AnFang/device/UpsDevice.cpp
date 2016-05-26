#include "UpsDevice.h"

#include "../MainApp.h"


CUpsDevice::CUpsDevice(void)
    :m_fd(-1)
{
}


CUpsDevice::~CUpsDevice(void)
{
}


//打开串口
bool CUpsDevice::OpenSerialPort(uint8 port)
{
    char port_path[128] = {0};
#ifdef _NORMAL_ENV
    snprintf(port_path, sizeof(port_path), "/dev/ttyS%d", port);
#else
    snprintf(port_path, sizeof(port_path), "/dev/ttyUSB%d", port);
#endif
    MainApp.m_log.WriteLog("温湿度控制器类打开串口:%s", port_path);

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

//设置串口波特率, 数据传输速率9600bps
bool CUpsDevice::SetBaudrate(uint16 buad)
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

//设置奇偶校验 为异步方式，起始位1位，数据位8位，停止位1位，无校验⃞
bool CUpsDevice::SetParity(uint8 databits, uint8 stopbits, uint8 parity)
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

//写数据

//除SOI和EOI是以十六进制解释（SOI=7EH，EOI=0DH），十六进制传
//输外，其余各项都是以十六进制解释，以十六进制—ASCII码的方式传输，每个字节用两个ASCII
//码表示，即高四位用一个ASCII码表示，低四位用一个ASCII码表示
bool CUpsDevice::WriteData(uint8 addr, uint8 cid2, const char *pbuf, uint16 len, const uint8 timeout)
{
    static uint8 send_buf[1024] = {0}; // 存储待发送的十六进制命令
    static uint8 tmp_body[1024] = {0}; // 存放报文的主体部分
    static char  temp_buf[1024] = {0}; // 临时存放去除空格的命令字符串

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
  
    // 协议格式为
    //SOI(1) VER(1) ADDR(1) CID1(1) CID2(1) LENGTH(2) INFO(0~4096) CHKSUM(2) EOI(1) 
    // 0X7E   0X21    00     0X2A                                             0X0D

    // 计算LENGTH(2)
    uint16 length = CalcLength(temp_len);

    // 组装报文的主体部分，不包括 SOI, CHKSUM, EOI
    //VER
    memset(tmp_body, 0x00, sizeof(tmp_body));
    tmp_body[0] = '2';
    tmp_body[1] = '1';

    //ADDR
    char chBuf[8];
    memset(chBuf, 0x00, sizeof(chBuf));
    snprintf(chBuf, sizeof(chBuf), "%02X", addr);
    tmp_body[2] = chBuf[0];
    tmp_body[3] = chBuf[1];

    //CID1
    tmp_body[4] = '2';
    tmp_body[5] = 'A';

    //CID2
    memset(chBuf, 0x00, sizeof(chBuf));
    snprintf(chBuf, sizeof(chBuf), "%02X", cid2);
    tmp_body[6] = chBuf[0];
    tmp_body[7] = chBuf[1];

    //LENGTH
    memset(chBuf, 0x00, sizeof(chBuf));
    snprintf(chBuf, sizeof(chBuf), "%04X", cid2);
    tmp_body[8] = chBuf[0];
    tmp_body[9] = chBuf[1];
    tmp_body[10] = chBuf[2];
    tmp_body[11] = chBuf[3];

    //INFO
    for (int i = 0; i < temp_len; i++)
    {
        tmp_body[12+i] = temp_buf[i];
    }

    // 计算CHKSUM
    int contextLen = temp_len + 12;
    uint16 chksum = CalcChksum(tmp_body, contextLen);
    
    // 组装待发送的内容
    memset(send_buf, 0x00, sizeof(send_buf));
    // 开始标记
    send_buf[0] = 0X7E;
    //报文主体
    for (int i = 0; i < contextLen; i++)
    {
        send_buf[i+1] = tmp_body[i];
    }
    // chksum
    memset(chBuf, 0x00, sizeof(chBuf));
    snprintf(chBuf, sizeof(chBuf), "%04X", chksum);
    send_buf[contextLen+1] = chBuf[0];
    send_buf[contextLen+2] = chBuf[1];
    send_buf[contextLen+3] = chBuf[2];
    send_buf[contextLen+4] = chBuf[3];

    // EOI
    send_buf[contextLen+5] = 0X0D;
    
    int left_len = contextLen + 6;
    uint8 *p = send_buf;

    // 输出待发送的命令
    char tmp_cmd[1024] = {0};
    for (int i = 0; i < left_len; i++)
    {
        sprintf(&tmp_cmd[i*3], "%02X ", send_buf[i]);
    }
    MainApp.m_log.WriteLog("发送到UPS的命令:%s", tmp_cmd);

    int nfds = 0; // 可写句柄数量
    fd_set write_fds; // 句柄集合
    struct timeval tv;  // select 等待时间
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

//读数据
int CUpsDevice::ReadData(unsigned char *pbuf, const int len, const uint8 timeout)
{
    int left_len = len;
    int nfds = 0;
    fd_set read_fds;
    struct timeval tv;
    int cnt = 0;
    while(left_len > 0)
    {
        tv.tv_sec = 0;
        tv.tv_usec = 500000;

        FD_ZERO(&read_fds);
        FD_SET(m_fd, &read_fds);

        nfds = select(m_fd + 1, &read_fds, NULL, NULL, &tv);
        if (nfds < 0)
        {
            MainApp.m_log.WriteLog("%s:%d,fds=%d", __FILE__, __LINE__, nfds);
            return -1;
        }
        else if (0 == nfds)
        {
            //printf("no fd use read!\n");
            if (cnt++ > 2)
            {
                MainApp.m_log.WriteLog("%s:%d,timeout return,nfds=%d", __FILE__, __LINE__, nfds);
                break;
            }
            continue;
        }
        cnt = 0;

        int nread = read(m_fd, pbuf, left_len);
        if (nread > 0)
        {
            left_len -= nread;
            pbuf += nread;
        }
        else
        {
            MainApp.m_log.WriteLog("%s:%d,read_len=%d", __FILE__, __LINE__, nread);
            break;
        }
    }

    return len - left_len;
}

//关闭串口
bool CUpsDevice::ClosePort()
{
    close(m_fd);
    m_fd = -1;

    return true;
}

//串口是否打开
bool CUpsDevice::IsOpen()
{
    return -1 != m_fd;
}

// 字符串转16进制
void CUpsDevice::StrToHex(uint8 *pbDest, const char *pbSrc, int nLen)
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

    return;
}

// 十六机制转字符串
void CUpsDevice::HexToStr(uint8 *pbDest, uint8 *pbSrc, int nLen)
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

    return;
}

// 删除空格
int CUpsDevice::TrimSpace(char* pDest, const char *src, int nLen)
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

// 计算LENGHT，4位LCHKSUM， 12位LENID
// 校验码的计算：D11D10D9D8+D7D6D5D4+D3D2D1D0，求和后模16的余数取反加1
uint16 CUpsDevice::CalcLength(uint16 len)
{
    // 保留低12位
    len = len & 0X00000FFF;
    uint8 first4bit = (len >> 8) & 0X0000000F; // 9~12 位
    uint8 second4bit = (len >> 4) & 0X0000000F; // 5~8 位
    uint8 third4bit = len & 0X0000000F; // 1~4 位

    uint16 chck = (first4bit + second4bit + third4bit) % 16;
    chck = (~chck) + 1;

    chck = chck & 0X000F;

    uint16 infoLen = len | (chck << 12);

    return infoLen;
}

// 计算CHKSUM CHKSUM的计算是除SOI⃝ EOI和CHKSUM外， 其他字符ASCII码值累加求和， 
//所得结果模65535 余数取反加1
uint16 CUpsDevice::CalcChksum(const uint8 *context, int len)
{
    int i = 0;
    uint16 chkValue = 0;
    for (; i < len; i++)
    {
        chkValue += context[i];
    }

    chkValue %= 65536;
    chkValue = ~chkValue;
    chkValue += 1;

    return chkValue;
}

// 解析告警状
//格式   SOI  VER  ADR  2AH  RTN  LENGTH  DATA INFO  CHKSUM  EOI
//字节数  1    1    1    1    1   2         LENID/2    2      1
//注 ：LENID=30H ⃞DATAINFO 由DATAFLAG （1 字节 ） 与WARNSTATE （23 字节 ） 组成
bool CUpsDevice::ParseAlarmStatus(const uint8 *pbuf, int len)
{
    // 检查SOI
    if (0X7E != pbuf[0])
    {
        MainApp.m_log.WriteLog("返回的响应信息不正确，SOI 错误!");
        return false;
    }

    // 检查VER
    if (0X21 != pbuf[1])
    {
        MainApp.m_log.WriteLog("返回的响应信息不正确，VER 错误!");
        return false;
    }

    // 检查RTN
    if (0X01 == pbuf[4])
    {
        MainApp.m_log.WriteLog("返回信息有误：VER错");
        return false;
    }
    else if(0X02 == pbuf[4])
    {
        MainApp.m_log.WriteLog("返回信息有误：CHKSUM错");
        return false;
    }
    else if (0X03 == pbuf[4])
    {
        MainApp.m_log.WriteLog("返回信息有误：LCHKSUM错");
        return false;
    }
    else if(0X04 == pbuf[4])
    {
        MainApp.m_log.WriteLog("返回信息有误：CID2无效");
        return false;
    }
    else if(0X05 == pbuf[4])
    {
        MainApp.m_log.WriteLog("返回信息有误：命令格式错");
        return false;
    }
    else if(0X06 == pbuf[4])
    {
        MainApp.m_log.WriteLog("返回信息有误：无效数据");
        return false;
    }
    else if(0XE0 == pbuf[4])
    {
        MainApp.m_log.WriteLog("返回信息有误：无效权限");
        return false;
    }
    else if(0XE1 == pbuf[4])
    {
        MainApp.m_log.WriteLog("返回信息有误：操作失败");
        return false;
    }
    else
    {
        if (0X00 != pbuf[4])
        {
            MainApp.m_log.WriteLog("返回信息有误：未知的错误");
            return false;
        }
    }

    // LENGTH占两个字节
    short int infoLen = 0;
    infoLen = pbuf[5];
    infoLen <<= 8;
    infoLen |= pbuf[6];

    // LENGTH的前四位是LCHKSUM
    infoLen = infoLen & 0X00000FFF;
    if (0X30 != infoLen)
    {
        MainApp.m_log.WriteLog("LENGTHID 不正确:[0X%X]", infoLen);
        return false;
    }

    // 截取DATAINFO内容
    int start = 7;
    char sInfo[64] = {0};
    memset(sInfo, 0X00, sizeof(sInfo));
    for (int i = 0; i < infoLen; i++)
    {
        sInfo[i] = pbuf[i+start];
    }

    // 把字符转为16进制数
    uint8 byteInfo[54] = {0};
    memset(byteInfo, 0X00, sizeof(byteInfo));
    StrToHex(byteInfo, sInfo, infoLen);

    // 去状态值，参加 iTrust Adapt1-3k UPS 电总协议（客户版）V110.pdf
    // 表6.3 功率模块告警内容及传送
    std::string strUpsAlarmStatus;
    if (0X00 == byteInfo[1])
    {
        strUpsAlarmStatus += "同步状态：正常#";
    }
    else if (0X03 == byteInfo[1])
    {
        strUpsAlarmStatus += "同步状态：不同步#";
    }

    if (0X00 == byteInfo[2])
    {
        strUpsAlarmStatus += "市电：正常#";
    }
    else if (0XF0 == byteInfo[2])
    {
        strUpsAlarmStatus += "市电：异常#";
    }

    if (0X00 == byteInfo[3])
    {
        strUpsAlarmStatus += "整流器：正常#";
    }
    else if (0XF0 == byteInfo[3])
    {
        strUpsAlarmStatus += "整流器：故障#";
    }

    if (0X00 == byteInfo[4])
    {
        strUpsAlarmStatus += "逆变器：正常#";
    }
    else if(0XF0 == byteInfo[4])
    {
        strUpsAlarmStatus += "逆变器：故障#";
    }

    if (0X00 == byteInfo[5])
    {
        strUpsAlarmStatus += "旁路：正常#";
    }
    else if (0XF0 == byteInfo[5])
    {
        strUpsAlarmStatus += "旁路：异常#";
    }

    if (0X00 == byteInfo[6])
    {
        strUpsAlarmStatus += "蓄电池电压异常：正常#";
    }
    else if (0X01 == byteInfo[6])
    {
        strUpsAlarmStatus += "蓄电池电压异常：低于下限#";
    }
    else if (0X02 == byteInfo[6])
    {
        strUpsAlarmStatus += "蓄电池电压异常：高于上限#";
    }
    else if (0XF0 == byteInfo[6])
    {
        strUpsAlarmStatus += "蓄电池电压异常：故障#";
    }

    /* 7 标示电池数量m 填写20H
    strUpsAlarmStatus += "标示电池数量m：20H";
     8 用户自定义告警数量p 填写0FH
    strUpsAlarmStatus += "标示电池数量m：0FH";*/

    if (0X00 == byteInfo[9])
    {
        strUpsAlarmStatus += "散热器：正常#";
    }
    else if (0XF0 == byteInfo[9])
    {
        strUpsAlarmStatus += "散热器：过温#";
    }

    if (0X00 == byteInfo[10])
    {
        strUpsAlarmStatus += "风扇：正常#";
    }
    else if (0XF0 == byteInfo[10])
    {
        strUpsAlarmStatus += "风扇：故障#";
    }

    if (0X00 == byteInfo[12])
    {
        strUpsAlarmStatus += "母线电压：正常#";
    }
    else if (0XF0 == byteInfo[12])
    {
        strUpsAlarmStatus += "母线电压：异常#";
    }

    if (0X00 == byteInfo[13])
    {
        strUpsAlarmStatus += "充电器：正常#";
    }
    else if (0XF0 == byteInfo[13])
    {
        strUpsAlarmStatus += "充电器：故障#";
    }

    if (0X00 == byteInfo[14])
    {
        strUpsAlarmStatus += "电池EOD：正常#";
    }
    else if (0XF0 == byteInfo[14])
    {
        strUpsAlarmStatus += "电池EOD：电池EOD#";
    }

    if (0X00 == byteInfo[15])
    {
        strUpsAlarmStatus += "辅助电源：正常#";
    }
    else if (0XF0 == byteInfo[15])
    {
        strUpsAlarmStatus += "辅助电源：故障#";
    }

    if (0X00 == byteInfo[16])
    {
        strUpsAlarmStatus += "单机输出：正常#";
    }
    else if(0XF0 == byteInfo[16])
    {
        strUpsAlarmStatus += "单机输出：过载#";
    }

    if (0X00 == byteInfo[18])
    {
        strUpsAlarmStatus += "输出短路：正常#";
    }
    else if (0XF0 == byteInfo[18])
    {
        strUpsAlarmStatus += "输出短路：发生短路#";
    }

    if (0X00 == byteInfo[19])
    {
        strUpsAlarmStatus += "单机过载：正常";
    }
    else if (0XF0 == byteInfo[19])
    {
        strUpsAlarmStatus += "单机过载：过载超时";
    }
    MainApp.m_log.WriteLog("UPS告警信息：%s", strUpsAlarmStatus.c_str());
    return true;
}
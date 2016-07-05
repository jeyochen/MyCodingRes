#include "StdAfx.h"
#include "MySerialPort.h"
#include "LZSerialPortTool.h"
#include "LZSerialPortToolDlg.h"

#include "Log.h"

#define  BUF_SIZE 1024

// 显示提示信息
#define  SHOWTIP ((CLZSerialPortToolDlg*)(theApp.m_pMainWnd))->ShowTipMessage

CMySerialPort::CMySerialPort(void)
    :m_strComName("")
    ,m_baud(BAUD_9600)
    ,m_check(NOPARITY )
    ,m_databits(8)
    ,m_stopbits(1)
    ,m_portHandle(INVALID_HANDLE_VALUE)
    ,m_peerHandle(INVALID_HANDLE_VALUE)
    ,m_bClosePort(false)
{
}


CMySerialPort::~CMySerialPort(void)
{
    ClosePort();
}

// 初始化化串口参数
bool CMySerialPort::InitSerialParam(CString& com, DWORD baud, 
    int check, int databits, int stopbits)
{
    if (0 >= com.GetLength())
    {
        LOG.err_log("串口名称不能为空");
        CString strTip = "串口名称不能为空";
        SHOWTIP(strTip);
        return false;
    }

    if (0 > com.Find("COM"))
    {
        LOG.err_log("串口名称不正确");
        CString strTip = "串口名称不正确";
        SHOWTIP(strTip);
        return false;
    }

    m_strComName = com;
    m_baud = baud;
    m_check = check;
    m_databits = databits;
    m_stopbits = stopbits;

    CString strParity;
    switch(m_check)
    {
    case NOPARITY:
        strParity = "无校验";
        break;
    case ODDPARITY: 
        strParity = "奇校验";
        break;
    case EVENPARITY:
        strParity = "偶校验";
        break;
    case MARKPARITY:
        strParity = "MARKPARITY";
        break;
    case SPACEPARITY:
        strParity = "SPACEPARITY";
        break;
    default:
        strParity = "不正确的校验位";
        LOG.err_log("不正确的校验位");
        return false;
    }

    CString strStopbits;
    switch(m_stopbits)
    {
    case ONESTOPBIT:
        strStopbits = "1";
        break;
    case ONE5STOPBITS:
        strStopbits = "1.5";
        break;
    case TWOSTOPBITS:
        strStopbits = "2";
        break;
    default:
        strStopbits = "不正确的停止位";
        LOG.err_log("不正确的停止位");
        return false;
    }
    LOG.debug_log("串口初始化参数:[名称:%s][波特率:%d][校验类型:%s][数据位:%d][停止位:%s]"
        , (LPCSTR)m_strComName, m_baud, (LPCSTR)strParity, databits, (LPCSTR)strStopbits);
    return true;
}

// 打开串口
HANDLE CMySerialPort::OpenPort()
{
    if (IsOpen())
    {
        return m_portHandle;
    }

    if (0 >= m_strComName.GetLength())
    {
        LOG.err_log("请先设置串口参数");
        CString strTip = "请先设置串口参数";
        SHOWTIP(strTip);
        return INVALID_HANDLE_VALUE;
    }

    // 以重叠方式打开串口
    m_strComName = "\\\\.\\" + m_strComName;
    m_portHandle = CreateFile(m_strComName,
        GENERIC_READ|GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0, //FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
        NULL);

    if (INVALID_HANDLE_VALUE == m_portHandle)
    {
        CString strTip;
        LOG.err_log("打开串口[%s]失败", (LPCSTR)m_strComName);
        strTip.Format("打开串口[%s]失败", m_strComName);
        SHOWTIP(strTip);
        return INVALID_HANDLE_VALUE;
    }

    // 设置串口参数
    DCB dcb;
    BOOL ret = GetCommState(m_portHandle, &dcb);
    if (FALSE == ret)
    {
        LOG.err_log("获取串口默认配置参数失败");
        CString strTip("获取串口默认配置参数失败");
        SHOWTIP(strTip);
        return INVALID_HANDLE_VALUE;
    }
    dcb.BaudRate = m_baud;
    dcb.StopBits = m_stopbits;
    dcb.Parity = m_check;
    dcb.ByteSize = m_databits;
    //dcb.fParity = FALSE; // 禁止奇偶检查
    //dcb.fBinary = TRUE;//是否允许传二进制 
    if (FALSE == SetCommState(m_portHandle,&dcb))
    {
        LOG.err_log("配置串口参数失败!");
        CString strTip("配置串口参数失败!");
        SHOWTIP(strTip);
        return INVALID_HANDLE_VALUE;
    }
    
    // 设置缓冲区大小
    if (FALSE == SetupComm(m_portHandle, 1024, 1024))
    {
        LOG.err_log("缓冲区设置失败!");
        CString strTip("缓冲区设置失败!");
        SHOWTIP(strTip);
        return INVALID_HANDLE_VALUE;
    }

    // 设置串口超时时间
    COMMTIMEOUTS CommTimeOuts;
    CommTimeOuts.ReadIntervalTimeout = 100;
    CommTimeOuts.ReadTotalTimeoutMultiplier = 100;
    CommTimeOuts.ReadTotalTimeoutConstant = 100;

    CommTimeOuts.WriteTotalTimeoutMultiplier = 500;
    CommTimeOuts.WriteTotalTimeoutConstant = 2000;
    if(false == SetCommTimeouts(m_portHandle,&CommTimeOuts))
    {
        LOG.err_log("设置读写超时时间失败!");
        CString strTip("设置读写超时时间失败!");
        SHOWTIP(strTip);
        return INVALID_HANDLE_VALUE;
    }

    // 清空发送和接收缓冲区
    PurgeComm(m_portHandle, PURGE_TXCLEAR|PURGE_RXCLEAR);

    m_bClosePort = false;

    return m_portHandle;
}

// 串口是否已打开
bool CMySerialPort::IsOpen()
{
    return m_portHandle != INVALID_HANDLE_VALUE;
}

// 获取串口名称
CString CMySerialPort::GetPortName()
{
    return m_strComName;
}

// 读数据
DWORD CMySerialPort::ReadData(unsigned char* sbuf, DWORD readlen)
{
    // 读缓冲
    unsigned char lpInBuffer[BUF_SIZE] = {0};
    // 本次需要读取的字节数
    DWORD dwBytesRead = readlen;
    COMSTAT comStat;
    DWORD dwErrorFlags;

    // 检查是否要关闭串口
    if (m_bClosePort)
    {
        ClosePort();
        return 0;
    }

    if (false == IsOpen())
    {
        LOG.err_log("串口未打开");
        return -1;
    }

    if (0 >= readlen)
    {
        LOG.warn_log("要读取的字节数必须大于0");
        return 0;
    }
    
    BOOL bReadStatus = FALSE; // 读取状态
    DWORD dwReaded = 0; // 实际读取的字节数
    DWORD dwTotalReaded = 0; // 累计已读取的字节数
    static int cnt = 0; // 连续3次都不到数据返回。
    while(dwBytesRead > 0)
    {
        // 清除错误
        ClearCommError(m_portHandle, &dwErrorFlags, &comStat);
        // 本次可读取的字节数
        DWORD dwAbleRead = min(sizeof(lpInBuffer), (DWORD)comStat.cbInQue);
        if (0 >= dwAbleRead)
        {
            if(++cnt > 3)
            {
                //LOG.warn_log("串口[%s]已无数据可读!", (LPCSTR)m_strComName);
                return readlen - dwBytesRead;
            }
            Sleep(100);
            continue;
        }
        cnt = 0;
        memset(lpInBuffer, 0, sizeof(lpInBuffer));
        bReadStatus = ReadFile(m_portHandle, lpInBuffer,
            dwAbleRead, &dwReaded, NULL); // &m_osRead);

        if(!bReadStatus) //如果ReadFile函数返回FALSE
        {
            LOG.warn_log("读串口[%s]失败!", (LPCSTR)m_strComName);
            PurgeComm(m_portHandle, PURGE_TXABORT|
                PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

            return  readlen - dwBytesRead;
        }
        memcpy(sbuf+dwTotalReaded, lpInBuffer, dwReaded);
        
        dwBytesRead -= dwReaded;
        dwTotalReaded += dwReaded;
    }
    
    //PurgeComm(m_portHandle, PURGE_TXABORT|
    //    PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);

    return readlen - dwBytesRead;
}

// 写数据
DWORD CMySerialPort::WriteData(unsigned char* sbuf, DWORD sendlen)
{
    DWORD dwBytesWritten = sendlen;
    DWORD dwErrorFlags;
    COMSTAT comStat;

    // 检查是否要关闭串口
    if (m_bClosePort)
    {
        ClosePort();
        return 0;
    }

    if (false == PeerIsValid())
    {
        LOG.err_log("请设置有效的目的串口!");
        return -1;
    }

    if (0 >= sendlen)
    {
        LOG.err_log("要发送的字节数必须大于0");
    }

    BOOL bWriteStat = FALSE; // 发送状态
    DWORD dwWritedLen = 0; // 实际发送的字节数
    DWORD dwTotalWrited = 0; // 累计已发送的字节总数

    while(dwBytesWritten > 0)
    {
        ClearCommError(m_peerHandle, &dwErrorFlags,&comStat);

        bWriteStat = WriteFile(m_peerHandle, sbuf + dwTotalWrited, dwBytesWritten,
            &dwWritedLen, NULL);

        if(!bWriteStat)
        {
            LOG.warn_log("写串口[%s]失败!", (LPCSTR)m_strComName);
            return sendlen - dwBytesWritten;
        }

        dwBytesWritten -= dwWritedLen;
        dwWritedLen += dwWritedLen;
    }
    //PurgeComm(m_portHandle, PURGE_TXABORT|
    //    PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
    return sendlen - dwBytesWritten;
}

// 设置关闭串口标记
bool CMySerialPort::SetClosePort()
{
    m_bClosePort = true;
    return m_bClosePort;
}

// 关闭端口
bool CMySerialPort::ClosePort()
{
    if (IsOpen())
    {
        CloseHandle(m_portHandle);
        m_portHandle = INVALID_HANDLE_VALUE;
        m_peerHandle = INVALID_HANDLE_VALUE;
    }
    return true;
}

// 设置发送目标串口
bool CMySerialPort::SetPeerPort(HANDLE handle)
{
    if (INVALID_HANDLE_VALUE == handle || NULL == handle)
    {
        LOG.err_log("SetPeerPort 失败,handle不是有效的值");
        SHOWTIP("SetPeerPort 失败,handle不是有效的值");
        return false;
    }
    m_peerHandle = handle;

    return true;
}

// 判读对端串口是否有效
bool CMySerialPort::PeerIsValid()
{
    return m_peerHandle != INVALID_HANDLE_VALUE;
}

// 设置串口handle
bool CMySerialPort::SetPortHandle(HANDLE handle)
{
    m_portHandle = handle;
    return true;
}
#include "StdAfx.h"
#include "MySerialPort.h"
#include "LZSerialPortTool.h"
#include "LZSerialPortToolDlg.h"

#include "Log.h"

#define  BUF_SIZE 1024

// ��ʾ��ʾ��Ϣ
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

// ��ʼ�������ڲ���
bool CMySerialPort::InitSerialParam(CString& com, DWORD baud, 
    int check, int databits, int stopbits)
{
    if (0 >= com.GetLength())
    {
        LOG.err_log("�������Ʋ���Ϊ��");
        CString strTip = "�������Ʋ���Ϊ��";
        SHOWTIP(strTip);
        return false;
    }

    if (0 > com.Find("COM"))
    {
        LOG.err_log("�������Ʋ���ȷ");
        CString strTip = "�������Ʋ���ȷ";
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
        strParity = "��У��";
        break;
    case ODDPARITY: 
        strParity = "��У��";
        break;
    case EVENPARITY:
        strParity = "żУ��";
        break;
    case MARKPARITY:
        strParity = "MARKPARITY";
        break;
    case SPACEPARITY:
        strParity = "SPACEPARITY";
        break;
    default:
        strParity = "����ȷ��У��λ";
        LOG.err_log("����ȷ��У��λ");
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
        strStopbits = "����ȷ��ֹͣλ";
        LOG.err_log("����ȷ��ֹͣλ");
        return false;
    }
    LOG.debug_log("���ڳ�ʼ������:[����:%s][������:%d][У������:%s][����λ:%d][ֹͣλ:%s]"
        , (LPCSTR)m_strComName, m_baud, (LPCSTR)strParity, databits, (LPCSTR)strStopbits);
    return true;
}

// �򿪴���
HANDLE CMySerialPort::OpenPort()
{
    if (IsOpen())
    {
        return m_portHandle;
    }

    if (0 >= m_strComName.GetLength())
    {
        LOG.err_log("�������ô��ڲ���");
        CString strTip = "�������ô��ڲ���";
        SHOWTIP(strTip);
        return INVALID_HANDLE_VALUE;
    }

    // ���ص���ʽ�򿪴���
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
        LOG.err_log("�򿪴���[%s]ʧ��", (LPCSTR)m_strComName);
        strTip.Format("�򿪴���[%s]ʧ��", m_strComName);
        SHOWTIP(strTip);
        return INVALID_HANDLE_VALUE;
    }

    // ���ô��ڲ���
    DCB dcb;
    BOOL ret = GetCommState(m_portHandle, &dcb);
    if (FALSE == ret)
    {
        LOG.err_log("��ȡ����Ĭ�����ò���ʧ��");
        CString strTip("��ȡ����Ĭ�����ò���ʧ��");
        SHOWTIP(strTip);
        return INVALID_HANDLE_VALUE;
    }
    dcb.BaudRate = m_baud;
    dcb.StopBits = m_stopbits;
    dcb.Parity = m_check;
    dcb.ByteSize = m_databits;
    //dcb.fParity = FALSE; // ��ֹ��ż���
    //dcb.fBinary = TRUE;//�Ƿ����������� 
    if (FALSE == SetCommState(m_portHandle,&dcb))
    {
        LOG.err_log("���ô��ڲ���ʧ��!");
        CString strTip("���ô��ڲ���ʧ��!");
        SHOWTIP(strTip);
        return INVALID_HANDLE_VALUE;
    }
    
    // ���û�������С
    if (FALSE == SetupComm(m_portHandle, 1024, 1024))
    {
        LOG.err_log("����������ʧ��!");
        CString strTip("����������ʧ��!");
        SHOWTIP(strTip);
        return INVALID_HANDLE_VALUE;
    }

    // ���ô��ڳ�ʱʱ��
    COMMTIMEOUTS CommTimeOuts;
    CommTimeOuts.ReadIntervalTimeout = 100;
    CommTimeOuts.ReadTotalTimeoutMultiplier = 100;
    CommTimeOuts.ReadTotalTimeoutConstant = 100;

    CommTimeOuts.WriteTotalTimeoutMultiplier = 500;
    CommTimeOuts.WriteTotalTimeoutConstant = 2000;
    if(false == SetCommTimeouts(m_portHandle,&CommTimeOuts))
    {
        LOG.err_log("���ö�д��ʱʱ��ʧ��!");
        CString strTip("���ö�д��ʱʱ��ʧ��!");
        SHOWTIP(strTip);
        return INVALID_HANDLE_VALUE;
    }

    // ��շ��ͺͽ��ջ�����
    PurgeComm(m_portHandle, PURGE_TXCLEAR|PURGE_RXCLEAR);

    m_bClosePort = false;

    return m_portHandle;
}

// �����Ƿ��Ѵ�
bool CMySerialPort::IsOpen()
{
    return m_portHandle != INVALID_HANDLE_VALUE;
}

// ��ȡ��������
CString CMySerialPort::GetPortName()
{
    return m_strComName;
}

// ������
DWORD CMySerialPort::ReadData(unsigned char* sbuf, DWORD readlen)
{
    // ������
    unsigned char lpInBuffer[BUF_SIZE] = {0};
    // ������Ҫ��ȡ���ֽ���
    DWORD dwBytesRead = readlen;
    COMSTAT comStat;
    DWORD dwErrorFlags;

    // ����Ƿ�Ҫ�رմ���
    if (m_bClosePort)
    {
        ClosePort();
        return 0;
    }

    if (false == IsOpen())
    {
        LOG.err_log("����δ��");
        return -1;
    }

    if (0 >= readlen)
    {
        LOG.warn_log("Ҫ��ȡ���ֽ����������0");
        return 0;
    }
    
    BOOL bReadStatus = FALSE; // ��ȡ״̬
    DWORD dwReaded = 0; // ʵ�ʶ�ȡ���ֽ���
    DWORD dwTotalReaded = 0; // �ۼ��Ѷ�ȡ���ֽ���
    static int cnt = 0; // ����3�ζ��������ݷ��ء�
    while(dwBytesRead > 0)
    {
        // �������
        ClearCommError(m_portHandle, &dwErrorFlags, &comStat);
        // ���οɶ�ȡ���ֽ���
        DWORD dwAbleRead = min(sizeof(lpInBuffer), (DWORD)comStat.cbInQue);
        if (0 >= dwAbleRead)
        {
            if(++cnt > 3)
            {
                //LOG.warn_log("����[%s]�������ݿɶ�!", (LPCSTR)m_strComName);
                return readlen - dwBytesRead;
            }
            Sleep(100);
            continue;
        }
        cnt = 0;
        memset(lpInBuffer, 0, sizeof(lpInBuffer));
        bReadStatus = ReadFile(m_portHandle, lpInBuffer,
            dwAbleRead, &dwReaded, NULL); // &m_osRead);

        if(!bReadStatus) //���ReadFile��������FALSE
        {
            LOG.warn_log("������[%s]ʧ��!", (LPCSTR)m_strComName);
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

// д����
DWORD CMySerialPort::WriteData(unsigned char* sbuf, DWORD sendlen)
{
    DWORD dwBytesWritten = sendlen;
    DWORD dwErrorFlags;
    COMSTAT comStat;

    // ����Ƿ�Ҫ�رմ���
    if (m_bClosePort)
    {
        ClosePort();
        return 0;
    }

    if (false == PeerIsValid())
    {
        LOG.err_log("��������Ч��Ŀ�Ĵ���!");
        return -1;
    }

    if (0 >= sendlen)
    {
        LOG.err_log("Ҫ���͵��ֽ����������0");
    }

    BOOL bWriteStat = FALSE; // ����״̬
    DWORD dwWritedLen = 0; // ʵ�ʷ��͵��ֽ���
    DWORD dwTotalWrited = 0; // �ۼ��ѷ��͵��ֽ�����

    while(dwBytesWritten > 0)
    {
        ClearCommError(m_peerHandle, &dwErrorFlags,&comStat);

        bWriteStat = WriteFile(m_peerHandle, sbuf + dwTotalWrited, dwBytesWritten,
            &dwWritedLen, NULL);

        if(!bWriteStat)
        {
            LOG.warn_log("д����[%s]ʧ��!", (LPCSTR)m_strComName);
            return sendlen - dwBytesWritten;
        }

        dwBytesWritten -= dwWritedLen;
        dwWritedLen += dwWritedLen;
    }
    //PurgeComm(m_portHandle, PURGE_TXABORT|
    //    PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
    return sendlen - dwBytesWritten;
}

// ���ùرմ��ڱ��
bool CMySerialPort::SetClosePort()
{
    m_bClosePort = true;
    return m_bClosePort;
}

// �رն˿�
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

// ���÷���Ŀ�괮��
bool CMySerialPort::SetPeerPort(HANDLE handle)
{
    if (INVALID_HANDLE_VALUE == handle || NULL == handle)
    {
        LOG.err_log("SetPeerPort ʧ��,handle������Ч��ֵ");
        SHOWTIP("SetPeerPort ʧ��,handle������Ч��ֵ");
        return false;
    }
    m_peerHandle = handle;

    return true;
}

// �ж��Զ˴����Ƿ���Ч
bool CMySerialPort::PeerIsValid()
{
    return m_peerHandle != INVALID_HANDLE_VALUE;
}

// ���ô���handle
bool CMySerialPort::SetPortHandle(HANDLE handle)
{
    m_portHandle = handle;
    return true;
}
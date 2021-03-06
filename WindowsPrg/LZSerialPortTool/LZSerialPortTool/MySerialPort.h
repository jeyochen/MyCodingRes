#pragma once
class CMySerialPort
{
public:
    CMySerialPort(void);
    ~CMySerialPort(void);

    // 初始化化串口参数
    bool InitSerialParam(CString& com, DWORD baud, 
        int check, int databits, int stopbits);

    // 打开串口
    HANDLE OpenPort();

    // 读数据
    DWORD ReadData(unsigned char* sbuf, DWORD readlen);

    // 写数据
    DWORD WriteData(unsigned char* sbuf, DWORD sendlen);

    // 设置关闭串口标记
    bool SetClosePort();

    // 关闭端口
    bool ClosePort();

    // 设置发送目标串口
    bool SetPeerPort(HANDLE handle);

    // 判读对端串口是否有效
    bool PeerIsValid();

    // 设置串口handle
    bool SetPortHandle(HANDLE handle);

    // 串口是否已打开
    bool IsOpen();

    // 获取串口名称
    CString GetPortName();

private:
    CString m_strComName; // 串口名称
    DWORD m_baud; // 波特率 
    int m_check; // 校验位
    int m_databits; // 数据位 
    int m_stopbits; // 停止位
    HANDLE m_portHandle; // 串口handle
    HANDLE m_peerHandle; // 另外一个串口，充当发送目的

    OVERLAPPED m_osRead;
    OVERLAPPED m_osWrite;
    bool m_bClosePort;
};


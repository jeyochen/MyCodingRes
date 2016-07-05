#pragma once
class CMySerialPort
{
public:
    CMySerialPort(void);
    ~CMySerialPort(void);

    // ��ʼ�������ڲ���
    bool InitSerialParam(CString& com, DWORD baud, 
        int check, int databits, int stopbits);

    // �򿪴���
    HANDLE OpenPort();

    // ������
    DWORD ReadData(unsigned char* sbuf, DWORD readlen);

    // д����
    DWORD WriteData(unsigned char* sbuf, DWORD sendlen);

    // ���ùرմ��ڱ��
    bool SetClosePort();

    // �رն˿�
    bool ClosePort();

    // ���÷���Ŀ�괮��
    bool SetPeerPort(HANDLE handle);

    // �ж��Զ˴����Ƿ���Ч
    bool PeerIsValid();

    // ���ô���handle
    bool SetPortHandle(HANDLE handle);

    // �����Ƿ��Ѵ�
    bool IsOpen();

    // ��ȡ��������
    CString GetPortName();

private:
    CString m_strComName; // ��������
    DWORD m_baud; // ������ 
    int m_check; // У��λ
    int m_databits; // ����λ 
    int m_stopbits; // ֹͣλ
    HANDLE m_portHandle; // ����handle
    HANDLE m_peerHandle; // ����һ�����ڣ��䵱����Ŀ��

    OVERLAPPED m_osRead;
    OVERLAPPED m_osWrite;
    bool m_bClosePort;
};


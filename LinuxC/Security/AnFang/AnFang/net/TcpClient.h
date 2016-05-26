#pragma once

#include <string>

class CTcpClient
{
public:
    CTcpClient(void);
    ~CTcpClient(void);

    //  线程执行函数
    void Run();

    bool SetFd(int fd)
    {
        m_fd = fd;
        return true;
    }

    int GetFd()
    {
        return m_fd;
    }

    bool SetPort(int port)
    {
        m_port = port;
        return true;
    }
    int GetPort()
    {
        return m_port;
    }

    bool SetAddr(const char *addr)
    {
        m_addr = addr;
        return true;
    }
    std::string GetAddr()
    {
        return m_addr;
    }
   
private:
    int m_fd; // 服务器主动连接过来的socket句柄
    int m_port;
    std::string m_addr;
};


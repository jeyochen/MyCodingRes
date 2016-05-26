#pragma once
#include <pthread.h>

#include <string>
#include <vector>

#include "net/TcpClient.h"
class CListenThread
{
public:
    CListenThread(void);
    ~CListenThread(void);

    // 初始化
    bool Initialize();

    // 反初始化
    bool Uninitialize();

    // 线程运行函数
    void Run();

    // 关闭监听句柄
    void CloseListen();

private:
    int m_listenFD; // 监听句柄
    int m_iSvrPort; // 监听端口
    std::string m_strSvrIp; // 监听ip
    std::vector<pthread_t> m_vecSockThreadID; // socket处理线程ID
    std::vector<CTcpClient> m_vecTcpClient; // socket处理对象
};


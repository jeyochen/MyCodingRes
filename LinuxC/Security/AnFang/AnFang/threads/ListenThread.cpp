#include "ListenThread.h"

#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>

#include "../MainApp.h"
#include "../log/CLog.h"
#include "../net/TcpClient.h"
#include "../config/inifile.h"

// 启动一个socket处理线程
void* StartSocktThread(void *arg)
{
    pthread_t tid =  pthread_self();
    pthread_detach(tid);
    CTcpClient *client = (CTcpClient*)arg;
    client->Run();
    delete client;
    client = NULL;
    return 0;
}

CListenThread::CListenThread(void)
    :m_listenFD(-1)
    ,m_iSvrPort(54535)
    ,m_strSvrIp("0.0.0.0")
{
}


CListenThread::~CListenThread(void)
{
}

// 初始化
bool CListenThread::Initialize()
{
    // 从配置文件读取监督听的ip地址和端口
    m_iSvrPort = MainApp.m_config.ReadInt("listen", "port", 54535);
    m_strSvrIp = MainApp.m_config.ReadString("listen", "ip", "0.0.0.0");
    MainApp.m_log.WriteLog("本机监听地址:[%s][%d]", m_strSvrIp.c_str(), m_iSvrPort);
    return true;
}

// 反初始化
bool CListenThread::Uninitialize()
{
    if (m_listenFD != -1)
    {
        close(m_listenFD);
    }
    return true;
}

// 线程运行函数
void CListenThread::Run()
{
    if (!Initialize())
    {
        MainApp.m_log.WriteLog("监听线程初始化失败!");
        return;
    }
                                 
    m_listenFD = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == m_listenFD)
    {
        MainApp.m_log.WriteLog("创建m_listenFD失败!");
        return;
    }

    //设置地址复用
    int flag = 1;
    if(0 > setsockopt(m_listenFD, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)))
    {
        MainApp.m_log.WriteLog("设置地址复用失败");
        return;
    }

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    memset(&server_addr, 0x00, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(m_iSvrPort);
    if ("0.0.0.0" == m_strSvrIp)
    {
        server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    }
    else
    {
        MainApp.m_log.WriteLog("设置本机监听地址:%s：%d", m_strSvrIp.c_str(), m_iSvrPort);
        server_addr.sin_addr.s_addr = inet_addr(m_strSvrIp.c_str());
    }
    if (0 > bind(m_listenFD, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        MainApp.m_log.WriteLog("绑定地址和端口失败:[%s][%d]", m_strSvrIp.c_str(), m_iSvrPort);
        return;
    }
    if (0 > listen(m_listenFD, 32))
    {
        MainApp.m_log.WriteLog("调用listen函数失败");
        return;
    }

    while(false == MainApp.GetExitNotify())
    {
        //MainApp.m_log.WriteLog("监听线程正在运行...");
        socklen_t sock_len = sizeof(client_addr);
        memset(&client_addr, 0, sock_len);
        int client_fd = accept(m_listenFD, (struct sockaddr*)&client_addr, &sock_len);
        if (0 > client_fd)
        {
            MainApp.m_log.WriteLog("一个错误的连接请求");
            continue;
        }
        MainApp.m_log.WriteLog("接收到一个来自客户端的连接[%s][%d]",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // 创建处理线程
        pthread_t  clientThreadId;
        CTcpClient *tcpClientObj = new CTcpClient();
        tcpClientObj->SetFd(client_fd);
        tcpClientObj->SetPort(ntohs(client_addr.sin_port));
        tcpClientObj->SetAddr(inet_ntoa(client_addr.sin_addr));
        if (0 != pthread_create(&clientThreadId, NULL, StartSocktThread, (void *)tcpClientObj))
        {
            MainApp.m_log.WriteLog("创建客户端处理线程失败:%s", strerror(errno));
        }
        tcpClientObj = NULL;
    }
    MainApp.m_log.WriteLog("监听线程主程序退出");
    Uninitialize();
    return;
}

// 关闭监听句柄
void CListenThread::CloseListen()
{
    close(m_listenFD);
    return;
}
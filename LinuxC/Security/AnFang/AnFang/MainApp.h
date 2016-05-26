#pragma once

#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "config/inifile.h"
#include "log/CLog.h"
#include "net/TcpClient.h"
#include "proto/DataMessage.pb.h"
#include "common/MsgStruct.h"

#include "threads/ListenThread.h"
#include "threads/CollectThread.h"
#include "threads/ExecCmdThread.h"
#include "threads/SendThread.h"

#include <vector>
#include <list>

using namespace std;
class CMainApp
{
public:
    
    ~CMainApp(void);

protected:
    CMainApp(void);

public:
    static CMainApp* GetAppInstance();


public:
    // 读配置文件
    CIniFile m_config;
    // 写日志
    CLog m_log;
    // 监听线程对象
    CListenThread m_listenObj;
    // 采集线程对象
    CCollectThread m_collectObj;
    // 执行命令线程对象
    CExecCmdThread m_execCmdObj;
    //发送线程对象
    CListenThread m_sendObj;

    // 应用程序初始化
    bool Initialize();

    // 应用程序反初始化
    bool Uninitialze();

    // 启动监听线程
    bool StartListenThread();

    // 启动数据采集线程
    bool StartCollectThread();

    // 启动命令执行线程，执行服务端请求的命令
    bool StartCmdExecThread();

    // 启动数据发送线程，把定时采集的数据和服务端请求执行采集的数据发送给服务端
    bool StartSendThread();

    // 向发送队列放入一个待发送的数据
    bool PutRequest(AFPROC::MRequest& request);
    // 从发送队列取除一个要发送的数据
    bool GetReuest(AFPROC::MRequest& request);

    // 向待执行的命令队列放入一个执行请求
    bool PutCommand(AFPROC::MCommand& cmd);
    // 从待执行的命令队列取出一个执行请求
    bool GetCommand(AFPROC::MCommand& cmd);

    // 通知应用程序退出
    void NotifyAppExit();
    // 获取退出通知
    bool GetExitNotify();

    // 获取和服务器的连接句柄
    int GetConnectFd()
    {
        return m_svrLink;
    }

    // 主循环，检查个线程运行状态
    bool Run();

    // 重新连接服务器
    int ReConnectServer();

    // 刷新配置信息
    void FreshConfig();

    // 获取设备采集命令
    vector<AFPROC::MCommand>& GetDeviceCmd();
protected:

    //  初始化设备采集命令
    bool InitDeviceCmd();

    // 读取服务地址信息
    bool ReadSvrInfo();

    // 连接到服务器
    bool ConnectServer();

private:
    // 类的为静态实例
    static CMainApp* m_pInstance;

private:
    int m_svrLink; // 和服务端的连接句柄
    bool m_bExitRun;  // 程序是否退出运行
    int m_svrPort; // 服务器端口
    std::string m_svrAddr; // 服务器地址
    pthread_t m_thIdListen;  // 监听线程ID
    pthread_t m_thIdCollect; // 数据采集线程ID
    pthread_t m_thIdCmdExec; // 命令执行线程ID
    pthread_t m_thIdSend;    // 数据发送线程ID

    // 存放定时采集的指令
    vector<AFPROC::MCommand> m_vecDeviceCmd;

    // 临时存放需要发送给服务器的消息列表
    list<AFPROC::MRequest> m_listRequest;

    // 临时存放服务器请求执行的命令
    list<AFPROC::MCommand> m_listCommand;
};

#define MainApp (*(CMainApp::GetAppInstance())) 


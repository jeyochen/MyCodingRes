#include "MainApp.h"

#include <iostream>
#include <string>

#include "./common/common.h"

using namespace std;

static sem_t g_cmd_sem;
static sem_t g_request_sem;

CMainApp* CMainApp::m_pInstance = NULL;
CMainApp::CMainApp(void)
    :m_svrLink(-1)
    ,m_bExitRun(false)
    ,m_svrPort(0)
{
}

// 监听线程
void* ListenThread(void* arg)
{
    CListenThread *listenObj = (CListenThread*)arg;
    listenObj->Run();
    return NULL;
}

// 采集线程
void* CollectThread(void* arg)
{
    CCollectThread *collectObj = (CCollectThread*)arg;
    collectObj->Run();
    return NULL;
}

// 执行命令线程
void* ExecCmdThread(void *arg)
{
    CExecCmdThread *execCmdObj = (CExecCmdThread*)arg;
    execCmdObj->Run();
    return NULL;
}

// 数据发送线程
void* SendThread(void *arg)
{
    CSendThread *sendObj = (CSendThread*)arg;
    sendObj->Run();
    return NULL;
}

CMainApp::~CMainApp(void)
{
    if (NULL != m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

CMainApp* CMainApp::GetAppInstance()
{
    if (NULL == m_pInstance)
    {
        m_pInstance = new CMainApp();
    }
    return m_pInstance;
}

// 应用程序初始化
bool CMainApp::Initialize()
{
    m_svrLink = -1;
    if (!m_config.OpenIniFile("./config.ini"))
    {
        m_log.WriteLog("打开配置文件失败!");
        return false;
    }

    if (!ReadSvrInfo())
    {
        return false;
    }

    if (!StartListenThread())
    {
        return false;
    }

    if (!StartCollectThread())
    {
        return false;
    }

    if (!StartCmdExecThread())
    {
        return false;
    }

    if (!StartSendThread())
    {
        return false;
    }

    InitDeviceCmd();
    return true;
}

//  获取化设备采集命令
vector<AFPROC::MCommand>& CMainApp::GetDeviceCmd()
{
    return m_vecDeviceCmd;
}

bool CMainApp::InitDeviceCmd()
{
    // 初始化采集设备列表
    AFPROC::MCommand cmd1;
    cmd1.set_d_type(SIX_LINE_DEVICE);
    cmd1.set_d_address(0XFF);
    cmd1.set_execommand("0300000006");
    cmd1.set_dcode("000001");
    cmd1.set_cmd_class(TIMING_EXEC);
    cmd1.set_cmd_func(COLLECTOR_SIX_LINE_DC);
    //m_vecDeviceCmd.push_back(cmd1);

    AFPROC::MCommand cmd2;
    cmd2.set_d_type(SIX_LINE_DEVICE);
    cmd2.set_d_address(0XFF);
    cmd2.set_execommand("0300060006");
    cmd2.set_dcode("000001");
    cmd2.set_cmd_class(TIMING_EXEC);
    cmd2.set_cmd_func(COLLECTOR_SIX_LINE_STATUS);
    //m_vecDeviceCmd.push_back(cmd2);

    AFPROC::MCommand cmd3;
    cmd3.set_d_type(TEMPER_HUMDI_CTRL_DEVICE);
    cmd3.set_d_address(0X01);
    cmd3.set_execommand("0400000002");
    cmd3.set_dcode("0000002");
    cmd3.set_cmd_class(TIMING_EXEC);
    cmd3.set_cmd_func(COLLECTOR_TEMPE_HUMIDI);
    m_vecDeviceCmd.push_back(cmd3);
 
    AFPROC::MCommand cmd4;
    cmd4.set_d_type(LINUX_HOST_DEVICE);
    cmd4.set_cmd_class(TIMING_EXEC);
    cmd4.set_cmd_func(COLLECTOR_LINUX_CPU_USAGE);
    m_vecDeviceCmd.push_back(cmd4);

    AFPROC::MCommand cmd5;
    cmd5.set_d_type(LINUX_HOST_DEVICE);
    cmd5.set_cmd_class(TIMING_EXEC);
    cmd5.set_cmd_func(COLLECTOR_LINUX_MEM_USAGE);
    m_vecDeviceCmd.push_back(cmd5);

    AFPROC::MCommand cmd6;
    cmd6.set_d_type(LINUX_HOST_DEVICE);
    cmd6.set_cmd_class(TIMING_EXEC);
    cmd6.set_cmd_func(COLLECTOR_LINUX_DISK_SPACE);
    m_vecDeviceCmd.push_back(cmd6);

    return true;

}

// 应用程序反初始化
bool CMainApp::Uninitialze()
{
    if (-1 != m_svrLink)
    {
        close(m_svrLink);
    }
    m_config.CloseIniFile();
    return true;
}

// 启动监听线程
bool CMainApp::StartListenThread()
{
    if (0 != pthread_create(&m_thIdListen, NULL, ListenThread, (void*)(&m_listenObj)))
    {
        MainApp.m_log.WriteLog("创建监听线程失败:%s", strerror(errno));
        return false;
    }
    return true;
}

// 启动数据采集线程
bool CMainApp::StartCollectThread()
{
    if (0 != pthread_create(&m_thIdCollect, NULL, CollectThread, (void*)(&m_collectObj)))
    {
        MainApp.m_log.WriteLog("创建采集线程失败:%s", strerror(errno));
        return false;
    }
    return true;
}

// 启动命令执行线程，执行服务端请求的命令
bool CMainApp::StartCmdExecThread()
{
    if (0 != pthread_create(&m_thIdCmdExec, NULL, ExecCmdThread, (void*)(&m_execCmdObj)))
    {
        MainApp.m_log.WriteLog("创建执行命令线程失败:%s", strerror(errno));
        return false;
    }
    return true;
}

// 启动数据发送线程，把定时采集的数据和服务端请求执行采集的数据发送给服务端
bool CMainApp::StartSendThread()
{
    if (0 != pthread_create(&m_thIdSend, NULL, SendThread, (void*)(&m_sendObj)))
    {
        MainApp.m_log.WriteLog("创建发送数据线程失败:%s", strerror(errno));
        return false;
    }
    return true;
}

// 向发送队列放入一个待发送的数据
bool CMainApp::PutRequest(AFPROC::MRequest& request)
{
    m_listRequest.push_back(request);
    sem_post(&g_request_sem);
    return true;
}

// 从发送队列取除一个要发送的数据
bool CMainApp::GetReuest(AFPROC::MRequest& request)
{
    sem_wait(&g_request_sem);    
    request = m_listRequest.front();
    m_listRequest.pop_front();
    return true;
}

// 向待执行的命令队列放入一个执行请求
bool CMainApp::PutCommand(AFPROC::MCommand& cmd)
{
    m_listCommand.push_back(cmd);
    sem_post(&g_cmd_sem);
    return true;
}

// 从待执行的命令队列取出一个执行请求
bool CMainApp::GetCommand(AFPROC::MCommand& cmd)
{
    sem_wait(&g_cmd_sem);
    cmd =  m_listCommand.front();
    m_listCommand.pop_front();
    return true;
}

// 通知应用程序退出
void CMainApp::NotifyAppExit()
{
    MainApp.m_log.WriteLog("接收到退出信号，通知线程退出");
    m_bExitRun = true;
}

// 获取退出通知
bool CMainApp::GetExitNotify()
{
    return m_bExitRun;
}

// 和服务端建立连接
// 连接到服务器
bool CMainApp::ConnectServer()
{
    struct sockaddr_in svr_addr;  // 服务器地址
    struct timeval timeo = {3, 0}; // 连接超时时间

    m_svrLink = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(m_svrLink, SOL_SOCKET, SO_SNDTIMEO, &timeo, sizeof(timeo));

    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_port = htons(m_svrPort);
    svr_addr.sin_addr.s_addr = inet_addr(m_svrAddr.c_str());

    if (0 > connect(m_svrLink, (struct sockaddr *)&svr_addr, sizeof(svr_addr)))
    {
        m_log.WriteLog("连接服务器失败:[%s:%d],error[%s]",
            m_svrAddr.c_str(), m_svrPort, strerror(errno));

        return false;
    }

    return true;
}

// 重新连接服务器
int CMainApp::ReConnectServer()
{
    close(m_svrLink);
    if(!ConnectServer())
    {
        m_log.WriteLog("重新连接服务失败,请检查服务器是否正常![%s:%d]",
            m_svrAddr.c_str(), m_svrPort);
        return -1;
    }
    return m_svrLink;
}

// 主循环，检查各个线程运行状态
bool CMainApp::Run()
{
    m_log.WriteLog("应用程序主线程启动...");
    if(!Initialize())
    {
        m_log.WriteLog("主线程初始化失败!");
        return false;
    }
    while(false == m_bExitRun)
    {
        // 检查线程状态
        if (ESRCH == pthread_kill(m_thIdListen, 0))
        {
            m_log.WriteLog("监听线程已退出,重启监听线程");
            pthread_join(m_thIdListen, NULL); // 清理线程资源
            StartListenThread();
        }

        if (ESRCH == pthread_kill(m_thIdCollect, 0))
        {
            m_log.WriteLog("采集线程已退出,重启采集线程");
            pthread_join(m_thIdCollect, NULL); // 清理线程资源
            StartCollectThread();
        }

        if (ESRCH == pthread_kill(m_thIdCmdExec, 0))
        {
            m_log.WriteLog("执行命令线程已退出,重启执行命令线程");
            pthread_join(m_thIdCmdExec, NULL); // 清理线程资源
            StartCmdExecThread();
        }

        if (ESRCH == pthread_kill(m_thIdSend, 0))
        {
            m_log.WriteLog("发送数据线程已退出,重启发送数据线程");
            pthread_join(m_thIdSend, NULL); // 清理线程资源
            StartSendThread();
        }
        sleep(1);
    }

    m_log.WriteLog("开始结束所有线程并退出");
    //m_listenObj.CloseListen();
    pthread_cancel(m_thIdListen);
    pthread_cancel(m_thIdCollect);
    pthread_cancel(m_thIdCmdExec);
    pthread_cancel(m_thIdSend);
    
    // 等待各个工作线程退出
    pthread_join(m_thIdListen, NULL);
    pthread_join(m_thIdCollect, NULL);
    pthread_join(m_thIdCmdExec, NULL);
    pthread_join(m_thIdSend, NULL);
    m_log.WriteLog("已结束所有线程并退出");
    Uninitialze();

    return true;
}

bool CMainApp::ReadSvrInfo()
{
    m_svrPort = m_config.ReadInt("server", "port", 0);
    m_svrAddr = m_config.ReadString("server", "ip", "");
    if (0 >= m_svrPort || 0 >= m_svrAddr.length())
    {
        MainApp.m_log.WriteLog("获取服务器端的IP地址和端口出错[%s][%d]", 
            m_svrAddr.c_str(), m_svrPort);
        return false;
    }
    m_log.WriteLog("服务器端的IP地址和端口为[%s][%d]", 
        m_svrAddr.c_str(), m_svrPort);

    return true;
}

// 刷新配置信息
void CMainApp::FreshConfig()
{
    m_config.CloseIniFile();
    m_config.OpenIniFile("./config.ini");
}
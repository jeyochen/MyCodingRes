#include <stdio.h>
#include <string.h>
#include <signal.h>

#include <iostream>

#include "MainApp.h"

//10) SIGUSR1
//12) SIGUSR2

void SigHandler(int signo)
{
    // 接收到 SIGUSR1 信号后，程序退出
    if (SIGUSR1 == signo)
    {
        MainApp.m_log.WriteLog("接收到一个SIGUSR1信号，程序即将退出！");
        MainApp.NotifyAppExit();
    }
    else if (SIGHUP == signo)
    {
        // 重新读取采集间隔时间
        MainApp.m_log.WriteLog("刷新采集间隔时间");
        MainApp.FreshConfig();
        MainApp.m_collectObj.ReadCollectorInterval();
    }
    return;
}

int main()
{
    signal(SIGCHLD, SIG_IGN); // 防止子进程退出时，程序退出
    signal(SIGINT, SIG_IGN);  // 防止按下CTRL+C键后程序退出
    signal(SIGQUIT, SIG_IGN);  // 防止按下CTRL+\键后程序退出并产生core文件
    signal(SIGHUP, SigHandler); // 重新读取采集间隔时间
    signal(SIGUSR1, SigHandler); // 处理自定义的信号

    // 启动主线程
    MainApp.Run();

    fprintf(stdout, "安防进程已退出!\n");
    return 0;
}
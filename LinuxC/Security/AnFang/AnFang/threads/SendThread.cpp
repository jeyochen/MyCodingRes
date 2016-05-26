#include "SendThread.h"

#include <sys/types.h>
#include <sys/socket.h>

#include "../MainApp.h"
#include "../proto/DataMessage.pb.h"

CSendThread::CSendThread(void)
{
}


CSendThread::~CSendThread(void)
{
}

// 初始化
bool CSendThread::Initialize()
{
    return true;
}

// 反初始化
bool CSendThread::Uninitialize()
{
    return true;
}

// 线程运行函数
void CSendThread::Run()
{
    if (false == Initialize())
    {
        return;
    }

    while(false == MainApp.GetExitNotify())
    {
        MainApp.m_log.WriteLog("发送线程正在运行...");
        sleep(5);
        // 从待发送的消息队列取出消息，并发送给服务器
        AFPROC::MRequest request;
        MainApp.GetReuest(request);
        if (false == SendRequst(request))
        {
            // 发送失败，如果是定时采集的，可能是和服务器的连接断开了
            // 重连服务器在发送 1：定时采集来的， 2：服务端实时发命令要求采集的
            if (2 == request.from_type())
            {
                continue;
            }
            int svrLink = MainApp.GetConnectFd();
            if (-1 == svrLink)
            {
                MainApp.m_log.WriteLog("重连服务器失败!不再重发");
                continue;
            }

            request.set_fd(svrLink);
            SendRequst(request);
        }
    }
    Uninitialize();
}

bool CSendThread::SendRequst(AFPROC::MRequest& request)
{
    std::string strSend;
    int fd = request.fd();
    strSend = request.SerializeAsString();

    int left_len = strSend.length();
    int nfds = 0; // 可写句柄数量
    fd_set write_fds; // 句柄集合
    struct timeval tv;  // select 等待时间
   
    const char *p = strSend.c_str();
    while(left_len > 0)
    {
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO(&write_fds);
        FD_SET(fd, &write_fds);

        nfds = select(fd + 1, NULL, &write_fds, NULL, &tv);
        if (nfds < 0)
        {
            // 如果是被其它信号中断的，则继续select
            if(EINTR==errno)
            {
                continue;
            }
            break;
        }
        else if(0 == nfds)
        {
            break;
        }
        if (0 == FD_ISSET(fd, &write_fds))
        {
            continue;
        }
        int nwrite = write(fd, p, left_len);
        if (nwrite > 0)
        {
            left_len -= nwrite;
            p += nwrite;
        }
        else
        {
            MainApp.m_log.WriteLog("数据发送失败[%d]", fd);
            break;
        }
    }
    return left_len == 0 ? true:false;
}

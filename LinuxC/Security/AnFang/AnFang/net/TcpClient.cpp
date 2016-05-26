#include "TcpClient.h"

#include <ctype.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "../MainApp.h"
#include "../proto/DataMessage.pb.h"
#include "../common/MsgStruct.h"
#include "../common/common.h"

CTcpClient::CTcpClient(void)
    :m_fd(-1)
    ,m_port(0)
{
}


CTcpClient::~CTcpClient(void)
{
}

void CTcpClient::Run()
{
    // 监听fd上是否有数据到达，如果有数据过来怎处理来自服务器端的执行请求
    // 并把命令执行后的采集结果放到MainApp的发送队列，由发送线程发送给服务端
    int cnt = 0;
    fd_set read_fds;
    struct timeval tv;
    char readBuf[1024] = {0};
    while(false == MainApp.GetExitNotify())
    {
        tv.tv_sec = 10;// 监听周期为3秒
        tv.tv_usec = 0; 
        FD_ZERO(&read_fds);
        FD_SET(m_fd, &read_fds);
        int nfds = select(m_fd+1, &read_fds, NULL, NULL, &tv);
        if (nfds < 0)
        {
            MainApp.m_log.WriteLog("select 监听出错,停止对客户端的数据处理[%s:%d][%d]",
                m_addr.c_str(), m_port, m_fd);
            close(m_fd);
            return;
        }
        if (0 == nfds)
        {
            // 如果连续3次超时都返回无数据可读则认为客户端无数据要发送了，关闭并退出线程
            MainApp.m_log.WriteLog("无数据可读");
            if (++cnt >= 3)
            {
                close(m_fd);
                MainApp.m_log.WriteLog("退出客户端处理线程");
                return;
            }
            continue;
        }
        cnt  = 0;

        // 读取客户端发送过来的数据，并处理
        memset(readBuf, 0x00, sizeof(readBuf));
        int nread = read(m_fd, readBuf, sizeof(readBuf));
        if (nread > 0)
        {
            // 反序列化buffer
            MainApp.m_log.WriteLog("接收了一个数据包:%d, %s", nread, readBuf);
            AFPROC::MCommand protoCmd;
            if (false == protoCmd.ParseFromArray(readBuf, strlen(readBuf)))
            {
                write(m_fd, readBuf, sizeof(readBuf));
                continue;
            }

            // 构建一个待执行的命令放入执行队列
            //struCommand cmd;
            //MCmd2StruCmd(protoCmd, cmd);
            //cmd.fd = m_fd;
            protoCmd.set_fd(m_fd);
            MainApp.PutCommand(protoCmd);
        }
        else
        {
            // 接收到0字节说明client已关闭
            close(m_fd);
            MainApp.m_log.WriteLog("没有读取到客户端发送过来的请求数据,退出客户端处理线程");
            return;
        }
    }
    return;
}

#pragma  once

#include <string>
#include "../proto/DataMessage.pb.h"


// 进程内部使用的消息结构

// 待执行的命令机构
struct struCommand{
    struCommand()
    {
        fd = -1;
        type = 0;
        addr = 0;
        cmd_class = 0;
        cmd_func = 0;
    }
    int fd; // 存放和服务器端的连接句柄
    int type; // 设备类型
    int addr; // 设备地址
    int cmd_class; // 命令分类
    int cmd_func; // 命令功能
    std::string cmd; // 命令
    std::string dcode; // 唯一设备编码，业务系统生成并告知客户端
};

// 命令执行后需要发送给服务器端的数据结构
struct struRequest{
    struRequest()
    {
        fd = -1;
        frome_type = 0;
        d_type = 0;
        d_addr = 0;
        cmd_class = 0;
        cmd_func = 0;
    }
    int fd; // 存放和服务器端的连接句柄
    int frome_type; // 数据来源。 1：定时采集来的， 2：服务端实时发命令要求采集的
    int d_type; // 设备类型
    int d_addr; // 设备地址
    int cmd_class; // 命令分类
    int cmd_func; // 命令功能
    std::string  cmd; //  命令
    std::string dcode; // 唯一设备编码，业务系统生成并告知客户端
    std::string body; // 消息体
    std::string token; // 客户端token
    std::string seq; // 发送序列号
};

// 内部类型和协议互转
extern void StruCmd2MCmd(struCommand& strucmd, AFPROC::MCommand& mcmd);

extern void MCmd2StruCmd(AFPROC::MCommand& mcmd, struCommand& strucmd);

extern void StruRequest2MRequest(struRequest& struReq, AFPROC::MRequest mReq);

extern void MRequest2StruRequest(AFPROC::MRequest mReq, struRequest& struReq);
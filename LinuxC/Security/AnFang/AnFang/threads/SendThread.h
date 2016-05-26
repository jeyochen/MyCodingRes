#pragma once

#include <string>

#include "../proto/DataMessage.pb.h"
class CSendThread
{
public:
    CSendThread(void);
    ~CSendThread(void);

    // 初始化
    bool Initialize();

    // 反初始化
    bool Uninitialize();

    // 线程运行函数
    void Run();

protected:
    bool SendRequst(AFPROC::MRequest& request);    
};


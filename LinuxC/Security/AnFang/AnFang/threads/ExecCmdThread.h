#pragma once

#include "CollectThread.h"
class CExecCmdThread
{
public:
    CExecCmdThread(void);
    ~CExecCmdThread(void);

    // 初始化
    bool Initialize();

    // 反初始化
    bool Uninitialize();

    // 线程运行函数
    void Run();

private:
    CCollectThread m_collectObj;
};


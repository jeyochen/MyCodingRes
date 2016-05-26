#pragma once

/*******************************************************
* 时刻告警主机
* create by Chenjiyou On 2015-05-16
*********************************************************/
#include <string>

#include "SK811_UDP_DLL.h"
#include "../proto/DataMessage.pb.h"
#include "../proto/CollectDatatMessage.pb.h"

class CShikeAlarmDevice
{
public:
    CShikeAlarmDevice(void);
    ~CShikeAlarmDevice(void);

public:
    //  初始化网络
    bool InitNetEnv();

    // 采集时刻告警主机各个防区状态
   bool CollectRegionStatus(COLLECT_DATA::MSKHostInfos& alarmhostinfo);

    // 布防
    bool EnableAlarm(const AFPROC::MCommand& mcmd);

    // 撤防
    bool DisableAlarm(const AFPROC::MCommand& mcmd);

    // 卸载网络
    bool UninitNetEnv();

private:
    int m_port;
    std::string m_ip;
};


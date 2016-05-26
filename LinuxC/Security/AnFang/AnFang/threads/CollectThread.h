#pragma once

#include "../device/SerialDevice.h"
#include "../device/LinuxHostDevice.h"

class CCollectThread
{
public:
    CCollectThread(void);
    ~CCollectThread(void);

    // 初始化
    bool Initialize();

    // 反初始化
    bool Uninitialize();

    // 线程运行函数
    void Run();

    // 读取采集间隔时间
    void ReadCollectorInterval();

public:
    // 采集六路开关设备的电流
    bool CollectorSixLineDC(const AFPROC::MCommand& cmd);
    //  采集六路开关设备的状态
    bool CollectorSixLineStatus(const AFPROC::MCommand& cmd);

    // 采集温湿度
    bool CollectorTemperHumidi(const AFPROC::MCommand& cmd);
    // 执行空调遥控器命令
    bool ExecAirConditionerCmd(const AFPROC::MCommand& cmd);
    
    // 采集Linux主机Cpu使用率
    bool CollectorHostCpuLoad();
    // 采集Linux 主机内存使用率
    bool CollectorHostMemUsage();
    // 采集Linux 主机磁盘信息
    bool CollectorHostDiskInfo();

    // 采集所有时刻告警主机布防状态
    //bool CollectorShikeAlarm(const AFPROC::MCommand& cmd);
    //// 整机布防
    //bool DefenseAlarm();
    //// 整机撤防
    //bool DisAlarm();

private:
    int m_collectInterval; //采集间隔，以秒为单位
    CSerialDevice m_serialDeviceObj; // 串口设备
    CLinuxHostDevice m_linuxHostDeviceObj; // Linux主机
};


#pragma once

/*******************************************************
* 采集Linux主机内存、cpu、磁盘使用率
* create by Chenjiyou On 2015-05-16
*********************************************************/

#include "../proto/CollectDatatMessage.pb.h"

class CLinuxHostDevice
{
public:
    CLinuxHostDevice(void);
    ~CLinuxHostDevice(void);

public:
    // 获取CPU使用率
    bool GetCpuOverLoad(COLLECT_DATA::MCpuLoad& cpuload);

    // 获取内存使用率
    bool GetMemUsage(COLLECT_DATA::MMemInfo& meminfo);

    // 获取磁盘空间
    bool GetDiskInfo(COLLECT_DATA::MDiskInfo& diskinfo);

protected:
    // 获取新的cpu负载
    int get_cpu_load_new();
    // 获取cpu实时使用率
    int get_cpu_real( float *cpu_load );
};


#pragma once

/*******************************************************
* �ɼ�Linux�����ڴ桢cpu������ʹ����
* create by Chenjiyou On 2015-05-16
*********************************************************/

#include "../proto/CollectDatatMessage.pb.h"

class CLinuxHostDevice
{
public:
    CLinuxHostDevice(void);
    ~CLinuxHostDevice(void);

public:
    // ��ȡCPUʹ����
    bool GetCpuOverLoad(COLLECT_DATA::MCpuLoad& cpuload);

    // ��ȡ�ڴ�ʹ����
    bool GetMemUsage(COLLECT_DATA::MMemInfo& meminfo);

    // ��ȡ���̿ռ�
    bool GetDiskInfo(COLLECT_DATA::MDiskInfo& diskinfo);

protected:
    // ��ȡ�µ�cpu����
    int get_cpu_load_new();
    // ��ȡcpuʵʱʹ����
    int get_cpu_real( float *cpu_load );
};


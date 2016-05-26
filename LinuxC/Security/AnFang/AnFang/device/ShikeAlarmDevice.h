#pragma once

/*******************************************************
* ʱ�̸澯����
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
    //  ��ʼ������
    bool InitNetEnv();

    // �ɼ�ʱ�̸澯������������״̬
   bool CollectRegionStatus(COLLECT_DATA::MSKHostInfos& alarmhostinfo);

    // ����
    bool EnableAlarm(const AFPROC::MCommand& mcmd);

    // ����
    bool DisableAlarm(const AFPROC::MCommand& mcmd);

    // ж������
    bool UninitNetEnv();

private:
    int m_port;
    std::string m_ip;
};


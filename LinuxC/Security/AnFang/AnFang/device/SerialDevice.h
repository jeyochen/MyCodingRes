#pragma once

/*******************************************************
* 控制各类串口设备
* create by Chenjiyou On 2015-05-16
*********************************************************/

#include "SixLine.h"
#include "TemperHumiCtrl.h"

#include "../proto/DataMessage.pb.h"
#include "../proto/CollectDatatMessage.pb.h"

class CSerialDevice
{
public:
    CSerialDevice(void);
    ~CSerialDevice(void);

    /*********************************************** 六路开关***************************************************/
    // 执行六路开关控制命令
    bool ExecSixLineCtrlCmd(int port, const AFPROC::MCommand& mcmd);
    // 获取六路开关的电流量
    bool GetSixlineDC(int port, const AFPROC::MCommand& mcmd, COLLECT_DATA::MSixLineDC& dc);
    // 获取六路开关的状态
    bool GetSixlineStatus(int port, const AFPROC::MCommand& mcmd, COLLECT_DATA::MSixLineStatus& status);

    /*************************************************温湿度及空调控制器*****************************************/
    // 采集温湿度
    bool GetTemperatureHumidity(int port, const AFPROC::MCommand& mcmd, COLLECT_DATA::MTemperHumidityValue& mvalue);
    // 执行空调遥控器命令
    bool ExcAirConditonCmd(int port, const AFPROC::MCommand& mcmd);    

private:
    CSixLine m_sixLineObj; // 智能照明控制器
    CTemperHumiCtrl m_temperHumiCtrlObj; // 温湿度控制器
};


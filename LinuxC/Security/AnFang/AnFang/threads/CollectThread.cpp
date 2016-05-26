#include "CollectThread.h"

#include <sys/time.h>
#include "../MainApp.h"
#include "../config/inifile.h"
#include "../proto/DataMessage.pb.h"
#include "../proto/CollectDatatMessage.pb.h"
#include "../common/common.h"

CCollectThread::CCollectThread(void)
    :m_collectInterval(60)
{
}


CCollectThread::~CCollectThread(void)
{
}

// 初始化
bool CCollectThread::Initialize()
{
    ReadCollectorInterval();
    return true;
}

// 读取采集间隔时间
void CCollectThread::ReadCollectorInterval()
{
    m_collectInterval = MainApp.m_config.ReadInt("collector", "interval", 60);
    MainApp.m_log.WriteLog("m_collectInterval = %d", m_collectInterval);
}

// 反初始化
bool CCollectThread::Uninitialize()
{
    return true;
}

// 线程运行函数
void CCollectThread::Run()
{
    long pre_secs; // 采集前的时间
    long cur_secs; // 采集后的时间

    Initialize();
    struct timeval tm;

    gettimeofday(&tm, NULL);
    cur_secs = tm.tv_sec;

    // 规整到正分钟开始采集
    int sleepSecs = cur_secs % 60;
    if (sleepSecs > 0)
    {
        MainApp.m_log.WriteLog("规整时间为:%d秒", 60 - sleepSecs);
        sleep(60 - sleepSecs);
    }
    gettimeofday(&tm, NULL);
    cur_secs = tm.tv_sec;
    while(false == MainApp.GetExitNotify())
    {
        MainApp.m_log.WriteLog("采集线程正在运行...");
        // 定时执行采集设备数据，并把采集的结果放入MainApp的发送队列
        // 由发送线程发送给服务端
        // 执行数据采集
        
        gettimeofday(&tm, NULL);
        pre_secs = tm.tv_sec;
        vector<AFPROC::MCommand> vecDeviceCmd =  MainApp.GetDeviceCmd();
        int cmdSize = vecDeviceCmd.size();
        for (int i = 0; i < cmdSize; i++)
        {
            AFPROC::MCommand cmd = vecDeviceCmd[i];
            MainApp.m_log.WriteLog("采集命令数:%d, 当前采集第：%d,d_type:%d, cmd_type:%d", 
                cmdSize, i + 1, cmd.d_type(), cmd.cmd_func());
            if (SIX_LINE_DEVICE == cmd.d_type())
            {
                if (COLLECTOR_SIX_LINE_DC == cmd.cmd_func() && TIMING_EXEC == cmd.cmd_class())
                {
                    CollectorSixLineDC(cmd);
                }
                else if (COLLECTOR_SIX_LINE_STATUS == cmd.cmd_func() && TIMING_EXEC == cmd.cmd_class())
                {
                    CollectorSixLineStatus(cmd);
                }
            }
            else if (TEMPER_HUMDI_CTRL_DEVICE == cmd.d_type())
            {
                if (COLLECTOR_TEMPE_HUMIDI == cmd.cmd_func() && TIMING_EXEC == cmd.cmd_class())
                {
                    CollectorTemperHumidi(cmd);
                }
            }
            else if(LINUX_HOST_DEVICE == cmd.d_type())
            {
                if (COLLECTOR_LINUX_CPU_USAGE == cmd.cmd_func() && TIMING_EXEC == cmd.cmd_class())
                {
                    CollectorHostCpuLoad();
                }
                else if (COLLECTOR_LINUX_MEM_USAGE == cmd.cmd_func() && TIMING_EXEC == cmd.cmd_class())
                {
                    CollectorHostMemUsage();
                }
                else if (COLLECTOR_LINUX_DISK_SPACE == cmd.cmd_func() && TIMING_EXEC == cmd.cmd_class())
                {
                    CollectorHostDiskInfo();
                }
            }
            else if (ALARM_HOST_DEVICE == cmd.d_type())
            {
                if (COLLECTOR_ALARM_STATUS == cmd.cmd_func() && TIMING_EXEC == cmd.cmd_class())
                {
                    //CollectorShikeAlarm(cmd);
                }
            }
        }
        // 本次采集时间 
        gettimeofday(&tm, NULL);
        cur_secs = tm.tv_sec;

        // 本次采集时间减去上次采集时间为采集消耗时间
        int elase_secs = cur_secs - pre_secs;
        
        // 采集间隔时间减去采集消耗时间为下次采集时间
        int left_secs = m_collectInterval - elase_secs;
        MainApp.m_log.WriteLog("left_secs=%d, elapse_secs=%d, m_collectInterval=%d", 
            left_secs, elase_secs, m_collectInterval);
        if (left_secs > 0)
        {
            sleep(left_secs);
        }
       
        // 执行下次采集
    }
    Uninitialize();
    return;
}

// 采集六路开关设备的电流
bool CCollectThread::CollectorSixLineDC(const AFPROC::MCommand& cmd)
{
    COLLECT_DATA::MSixLineDC lineDC;

    int iport = MainApp.m_config.ReadInt("serial_port","six_line_port", 0);
    MainApp.m_log.WriteLog("智能照明控制器串口号:%d", iport);
    m_serialDeviceObj.GetSixlineDC(iport, cmd, lineDC);
    int lines = lineDC.line_dc_size();
    for (int i = 0; i < lines; i++)
    {
        MainApp.m_log.WriteLog("line%d DC is:%f", i+1, lineDC.line_dc(i));
    }

    return true;
}

//  采集六路开关设备的状态
bool CCollectThread::CollectorSixLineStatus(const AFPROC::MCommand& cmd)
{
    COLLECT_DATA::MSixLineStatus lineStatus;

    int iport = MainApp.m_config.ReadInt("serial_port","six_line_port", 0);
    MainApp.m_log.WriteLog("智能照明控制器串口号:%d", iport);

    m_serialDeviceObj.GetSixlineStatus(iport, cmd, lineStatus);
    int lines = lineStatus.line_status_size();
    for (int i = 0; i < lines; i++)
    {
        MainApp.m_log.WriteLog("line%d status is:%d", i+1, lineStatus.line_status(i));
    }

    return true;
}

// 采集温湿度
bool CCollectThread::CollectorTemperHumidi(const AFPROC::MCommand& cmd)
{
    COLLECT_DATA::MTemperHumidityValue temperValue;
    int iport = MainApp.m_config.ReadInt("serial_port","temper_humidi_port", 0);
    MainApp.m_log.WriteLog("温湿度控制器串口号:%d", iport);
    m_serialDeviceObj.GetTemperatureHumidity(iport, cmd, temperValue);
    MainApp.m_log.WriteLog("温度:%.2f, 湿度:%.2f", temperValue.temperature(), temperValue.humidity());

    return true;
}

// 执行空调遥控器命令
bool CCollectThread::ExecAirConditionerCmd(const AFPROC::MCommand& cmd)
{
    COLLECT_DATA::MTemperHumidityValue temperValue;
    int iport = MainApp.m_config.ReadInt("serial_port","temper_humidi_port", 0);
    MainApp.m_log.WriteLog("智能遥控器串口号:%d", iport);
    bool bres = m_serialDeviceObj.ExcAirConditonCmd(iport, cmd);
    if (bres)
    {
        MainApp.m_log.WriteLog("向智能遥控器发送命令成功!");
    }
    else
    {
        MainApp.m_log.WriteLog("向智能遥控器发送命令失败!");
    }

    return true;
}

// 采集Linux主机Cpu使用率
bool CCollectThread::CollectorHostCpuLoad()
{
    COLLECT_DATA::MCpuLoad cpuload;
    m_linuxHostDeviceObj.GetCpuOverLoad(cpuload);
    MainApp.m_log.WriteLog("Linux主机CPU使用率:%d", cpuload.load());
    return true;
}

// 采集Linux 主机内存使用率
bool CCollectThread::CollectorHostMemUsage()
{
    COLLECT_DATA::MMemInfo meminfo;
    m_linuxHostDeviceObj.GetMemUsage(meminfo);
    MainApp.m_log.WriteLog("物理内存%ld   空闲内存:%ld   使用率:%.2f",
        meminfo.total(), meminfo.free(), meminfo.usage());
    return true;
}

// 采集Linux 主机磁盘信息
bool CCollectThread::CollectorHostDiskInfo()
{
    COLLECT_DATA::MDiskInfo diskinfo;
    m_linuxHostDeviceObj.GetDiskInfo(diskinfo);
    int isize = diskinfo.diskinfo_size();
    for (int i = 0; i < isize; i++)
    {
        COLLECT_DATA::MDiskInfoItem item = diskinfo.diskinfo(i);
        MainApp.m_log.WriteLog("磁盘空间=%.2f  已用=%.2f  可用=%.2f 使用率=%.2f  挂载点=%s",
            item.space(), item.used(), item.available(), item.usage(), item.mount_point().c_str());
    }
    return true;
}

// 采集所有时刻告警主机布防状态
//bool CCollectThread::CollectorShikeAlarm(const AFPROC::MCommand& cmd)
//{
//    COLLECT_DATA::MSKHostInfos skhostinfo;
//    if (false == m_shikeAlarmDeviceObj.CollectRegionStatus(skhostinfo))
//    {
//        return false;
//    }
//
//    int hostSize = skhostinfo.sk_alarm_size();
//    for (int i = 0; i < hostSize; i++)
//    {
//        COLLECT_DATA::MSKHostInfoItem infoItem = skhostinfo.sk_alarm(i);
//        std::string hostIP = infoItem.ip();
//        int hostStatus = infoItem.host_status();
//        std::string szHostStatus = hostStatus == 1 ? "整机布防":"整机撤防";
//
//        int fangquType = infoItem.fangqu_type();
//        std::string szFangquType;
//        if (8 == fangquType)
//        {
//            szFangquType = "8路防区";
//            COLLECT_DATA::MFangquStatusItem fqStatus;
//            int arrFqStatus[8];
//            std::string strTemp;
//            for (int i = 0; i < 8; i++)
//            {
//                char st[16] = {0};
//                fqStatus = infoItem.fangqu_status(i);
//                arrFqStatus[i] = fqStatus.status();
//                snprintf(st, sizeof(st) - 1, "%d ", arrFqStatus[i]);
//                strTemp += st;
//            }
//
//            MainApp.m_log.WriteLog("时刻主机地址:%s, 主机状态:%s, 防区类型:%s 防区状态:%s",
//                hostIP.c_str(), szHostStatus.c_str(), szFangquType.c_str(), strTemp.c_str());
//        }
//        else if(16 == fangquType)
//        {
//            szFangquType = "16路防区";
//            COLLECT_DATA::MFangquStatusItem fqStatus;
//            int arrFqStatus[16];
//            std::string strTemp;
//            for (int i = 0; i < 16; i++)
//            {
//                char st[16] = {0};
//                fqStatus = infoItem.fangqu_status(i);
//                arrFqStatus[i] = fqStatus.status();
//                snprintf(st, sizeof(st) - 1, "%d ", arrFqStatus[i]);
//                strTemp += st;
//            }
//
//            MainApp.m_log.WriteLog("时刻主机地址:%s, 主机状态:%s, 防区类型:%s 防区状态:%s",
//                hostIP.c_str(), szHostStatus.c_str(), szFangquType.c_str(), strTemp.c_str());
//        }
//        else if(32 == fangquType)
//        {
//            szFangquType = "32路防区";
//            COLLECT_DATA::MFangquStatusItem fqStatus;
//            int arrFqStatus[32];
//            std::string strTemp;
//            for (int i = 0; i < 32; i++)
//            {
//                char st[16] = {0};
//                fqStatus = infoItem.fangqu_status(i);
//                arrFqStatus[i] = fqStatus.status();
//                snprintf(st, sizeof(st) - 1, "%d ", arrFqStatus[i]);
//                strTemp += st;
//            }
//
//            MainApp.m_log.WriteLog("时刻主机地址:%s, 主机状态:%s, 防区类型:%s 防区状态:%s",
//                hostIP.c_str(), szHostStatus.c_str(), szFangquType.c_str(), strTemp.c_str());
//        }
//        else
//            szFangquType = "位置的防区类型";
//
//    }
//    return true;
//}
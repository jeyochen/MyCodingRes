#include "ExecCmdThread.h"
#include "../MainApp.h"
#include "../config/inifile.h"
#include "../proto/DataMessage.pb.h"
#include "../proto/CollectDatatMessage.pb.h"
#include "../common/common.h"

CExecCmdThread::CExecCmdThread(void)
{
}


CExecCmdThread::~CExecCmdThread(void)
{
}

// 初始化
bool CExecCmdThread::Initialize()
{
    return true;
}

// 反初始化
bool CExecCmdThread::Uninitialize()
{
    return true;
}

// 线程运行函数
void CExecCmdThread::Run()
{
    while(false == MainApp.GetExitNotify())
    {
        // 从待执行命令队列取出命令并执行，并把执行结果放入待发送的队列
        sleep(1);

        AFPROC::MCommand cmd;
        MainApp.GetCommand(cmd);

        MainApp.m_log.WriteLog("执行实时命令:d_type:%d, cmd_type:%d", cmd.d_type(), cmd.cmd_func());

        if (SIX_LINE_DEVICE == cmd.d_type())
        {
            if (COLLECTOR_SIX_LINE_DC == cmd.cmd_func() && REAL_TIME_EXEC == cmd.cmd_class())
            {
                m_collectObj.CollectorSixLineDC(cmd);
            }
            else if (COLLECTOR_SIX_LINE_STATUS == cmd.cmd_func() && REAL_TIME_EXEC == cmd.cmd_class())
            {
                m_collectObj.CollectorSixLineStatus(cmd);
            }
        }
        else if (TEMPER_HUMDI_CTRL_DEVICE == cmd.d_type())
        {
            if (COLLECTOR_TEMPE_HUMIDI == cmd.cmd_func() && REAL_TIME_EXEC == cmd.cmd_class())
            {
                m_collectObj.CollectorTemperHumidi(cmd);
            }
            else if(EXEC_AIR_CODITION_COMD == cmd.cmd_func() && REAL_TIME_EXEC == cmd.cmd_class())
            {
                m_collectObj.ExecAirConditionerCmd(cmd);
            }
        }
        else if(LINUX_HOST_DEVICE == cmd.d_type())
        {
            if (COLLECTOR_LINUX_CPU_USAGE == cmd.cmd_func() && REAL_TIME_EXEC == cmd.cmd_class())
            {
                m_collectObj.CollectorHostCpuLoad();
            }
            else if (COLLECTOR_LINUX_MEM_USAGE == cmd.cmd_func() && REAL_TIME_EXEC == cmd.cmd_class())
            {
                m_collectObj.CollectorHostMemUsage();
            }
            else if (COLLECTOR_LINUX_DISK_SPACE == cmd.cmd_func() && REAL_TIME_EXEC == cmd.cmd_class())
            {
                m_collectObj.CollectorHostDiskInfo();
            }
        }
        else if (ALARM_HOST_DEVICE == cmd.d_type())
        {
            if (COLLECTOR_ALARM_STATUS == cmd.cmd_func() && REAL_TIME_EXEC == cmd.cmd_class())
            {
                //m_collectObj.CollectorShikeAlarm(cmd);
            }
        }

    }
    return;
}
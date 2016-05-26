#include "ShikeAlarmDevice.h"

#include "../MainApp.h"

// 时刻报警系统报警回调函数
void RecevieAlarmCallBack(void* pvoid,int nType)
{
    SK_AlarmCodeInfo *pAlarm = (SK_AlarmCodeInfo*)pvoid;

    char szFangqu[128] = {0};
    char szAreaNo[16] = {0};
    switch(pAlarm->moreinfo.m_alarmstatefor485.m_485alarmstatefor8.CodeTypefor8)
    {
        //0表示有线，1表示无线,(2无线紧急报警，3表示键盘紧急，4表示键盘防拆(针对只有8防区的主机)无防区的概念),
        //5表示有线防区延时报警，6表示无线防区延时报警
    case 0:
        snprintf(szFangqu, sizeof(szFangqu) - 1, "%s", "有线防区");
        snprintf(szAreaNo, sizeof(szAreaNo) - 1, "%d", pAlarm->moreinfo.m_alarmstatefor485.m_485alarmstatefor8.CodeNo);
        break;;
    case 1:
        snprintf(szFangqu, sizeof(szFangqu) - 1, "%s", "无线线防区");
        snprintf(szAreaNo, sizeof(szAreaNo) - 1, "%d", pAlarm->moreinfo.m_alarmstatefor485.m_485alarmstatefor8.CodeNo);
    case 2:
        snprintf(szFangqu, sizeof(szFangqu) - 1, "%s", "无线紧急报警");
        snprintf(szAreaNo, sizeof(szAreaNo) - 1, "%s", "遥控器");
    case 3:
        snprintf(szFangqu, sizeof(szFangqu) - 1, "%s", "键盘紧急");
        snprintf(szAreaNo, sizeof(szFangqu) - 1, "%s", "键盘");
    case 4:
        snprintf(szFangqu, sizeof(szFangqu) - 1, "%s", "键盘防拆");
        snprintf(szAreaNo, sizeof(szFangqu) - 1, "%s", "键盘");
        break;
    case 5:
        snprintf(szFangqu, sizeof(szFangqu) - 1, "%s", "有线延时报警");
        snprintf(szAreaNo, sizeof(szFangqu) - 1, "%d", pAlarm->moreinfo.m_alarmstatefor485.m_485alarmstatefor8.CodeNo);
        break;
    case 6:
        snprintf(szFangqu, sizeof(szFangqu) - 1, "%s", "无线延时报警");
        snprintf(szAreaNo, sizeof(szFangqu) - 1, "%d", pAlarm->moreinfo.m_alarmstatefor485.m_485alarmstatefor8.CodeNo);
        break;
    }
}

CShikeAlarmDevice::CShikeAlarmDevice(void)
    :m_port(1159)
{
    InitNetEnv();
}


CShikeAlarmDevice::~CShikeAlarmDevice(void)
{
    UninitNetEnv();
}

//  初始化网络
bool CShikeAlarmDevice::InitNetEnv()
{
    m_port = MainApp.m_config.ReadInt("shike_alarm", "port", 1159);
    m_ip = MainApp.m_config.ReadString("shike_alarm", "ip", "0.0.0.0");

    char szIP[256] = {0};
    memset(szIP, 0x00, sizeof(szIP));
    snprintf(szIP, sizeof(szIP), m_ip.c_str());

    MainApp.m_log.WriteLog("与时sk-811进行通信的地址为:[%s][%d]", m_ip.c_str(), szIP);
    InitEnviroment(m_port, RecevieAlarmCallBack, szIP);
    return true;
}

// 采集时刻告警主机各个防区状态
bool CShikeAlarmDevice::CollectRegionStatus(COLLECT_DATA::MSKHostInfos& alarmhostinfo)
{
    // 获取所有在线设备的个数
    int nDevSize = SK_GetCurrOnLineDev(NULL);
    if (0 >= nDevSize)
    {
        return false;
    }

    SK_AlarmDevBasicInfo arrDev[1024];
    memset(arrDev, 0x00, sizeof(SK_AlarmDevBasicInfo) * sizeof(arrDev));
    SK_GetCurrOnLineDev(arrDev);
    for (int i = 0; i < nDevSize; i++)
    {
        // 获取每一个设备的防区状态
        SK_AlarmDevHostNo devHostNo;
        devHostNo.SKHostNoH = arrDev[i].skdevhostno.SKHostNoH; // 主机编号高位字节
        devHostNo.SKHostNoL = arrDev[i].skdevhostno.SKHostNoL; // 主机编号地位字节
        devHostNo.SKHostNoGroup = arrDev[i].skdevhostno.SKHostNoGroup; // 主机组号

        SK_AlarmDevBasicInfo devBasicInfo;
        // 获取主机基本信息
        SK_GetDevBasicInfo(&devBasicInfo, devHostNo);
        // 清除主机告警信息
        //SK_SetDevBasicInfo(&devBasicInfo,devBasicInfo.skdevhostno,SET_SK_CLEARALARM);

        int nFangquNum = devBasicInfo.devspecialstate.sk485state.fangQuNums;
        switch(nFangquNum)
        {
        case 8: // 八防区
            {
                COLLECT_DATA::MSKHostInfoItem *host8 = alarmhostinfo.add_sk_alarm();
                host8->set_ip(devBasicInfo.devIpAddress);
                host8->set_host_status(devBasicInfo.alarmdevArmState);
                host8->set_fangqu_type(8);
                int byteAlarm = devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for8fangqu._1To8FangQuArm;

                // 有线1~8防区各个防区的布防状态， 1布防，0 撤防
                for (int i = 0; i < 8; i++)
                {
                    COLLECT_DATA::MFangquStatusItem *status = host8->add_fangqu_status();
                    int n = byteAlarm >> (7-i);
                    n = byteAlarm & 0x1;
                    status->set_status(n);
                }

                // 无线1~8防区
                byteAlarm = devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for8fangqu.wire_1To8FangQuArm;
                for (int i = 0; i < 8; i++)
                {
                    COLLECT_DATA::MFangquStatusItem *status = host8->add_fangqu_status();
                    int n = byteAlarm >> (7-i);
                    n = byteAlarm & 0x1;
                    status->set_status(n);
                }
            }
            break;
        case 16:// 十六防区
            {
                COLLECT_DATA::MSKHostInfoItem *host16 = alarmhostinfo.add_sk_alarm();
                host16->set_ip(devBasicInfo.devIpAddress);
                host16->set_host_status(devBasicInfo.alarmdevArmState);
                host16->set_fangqu_type(16);
                int byteAlarm = devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for16fangqu._1To8FangQuArm;
                // 有线 1~8防区撤布防状态
                for (int i = 0; i < 8; i++)
                {
                    COLLECT_DATA::MFangquStatusItem *status = host16->add_fangqu_status();
                    int n = byteAlarm >> (7-i);
                    n = byteAlarm & 0x1;
                    status->set_status(n);
                }

                // 有线9~16防区撤布防状态
                byteAlarm = devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for16fangqu._9To16FangQuArm;
                for (int i = 0; i < 8; i++)
                {
                    COLLECT_DATA::MFangquStatusItem *status = host16->add_fangqu_status();
                    int n = byteAlarm >> (7-i);
                    n = byteAlarm & 0x1;
                    status->set_status(n);
                }
            }
            break;
        case 32: // 三十二防区
            {
                COLLECT_DATA::MSKHostInfoItem *host32 = alarmhostinfo.add_sk_alarm();
                host32->set_ip(devBasicInfo.devIpAddress);
                host32->set_host_status(devBasicInfo.alarmdevArmState);
                host32->set_fangqu_type(32);

                // 有线1~8
                int byteAlarm = devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for32fangqu._1To8FangQuArm;
                for (int i = 0; i < 8; i++)
                {
                    COLLECT_DATA::MFangquStatusItem *status = host32->add_fangqu_status();
                    int n = byteAlarm >> (7-i);
                    n = byteAlarm & 0x1;
                    status->set_status(n);
                }

                // 有线9~16
                byteAlarm = devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for32fangqu._9To16FangQuArm;
                for (int i = 0; i < 8; i++)
                {
                    COLLECT_DATA::MFangquStatusItem *status = host32->add_fangqu_status();
                    int n = byteAlarm >> (7-i);
                    n = byteAlarm & 0x1;
                    status->set_status(n);
                }

                // 有线17~24
                byteAlarm = devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for32fangqu._17To24FangquArm;
                for (int i = 0; i < 8; i++)
                {
                    COLLECT_DATA::MFangquStatusItem *status = host32->add_fangqu_status();
                    int n = byteAlarm >> (7-i);
                    n = byteAlarm & 0x1;
                    status->set_status(n);
                }

                // 有线25~32
                byteAlarm = devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for32fangqu._25To31FangQuArm;
                for (int i = 0; i < 8; i++)
                {
                    COLLECT_DATA::MFangquStatusItem *status = host32->add_fangqu_status();
                    int n = byteAlarm >> (7-i);
                    n = byteAlarm & 0x1;
                    status->set_status(n);
                }
            }
            break;;
        default:
            break;
        }

    }

    return true;
}

// 主机布防
bool CShikeAlarmDevice::EnableAlarm(const AFPROC::MCommand& mcmd)
{
    SK_AlarmDevBasicInfo devBasicInfo;

    int fangqu = mcmd.sk_fangqu_no();
    SK_AlarmDevHostNo hostno;
    hostno.SKHostNoH = mcmd.sk_hostno_h();
    hostno.SKHostNoL = mcmd.sk_hostno_l();
    hostno.SKHostNoGroup = mcmd.sk_host_group();

    SK_GetDevBasicInfo(&devBasicInfo, hostno);
    switch(fangqu)
    {
    case 8:
        devBasicInfo.devspecialstate.sk485state.fangQuNums = fangqu;
        devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for8fangqu._1To8FangQuArm=0xff;
        devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for8fangqu.wire_1To8FangQuArm=0xff;
        break;
    case 16:
        devBasicInfo.devspecialstate.sk485state.fangQuNums = fangqu;
        devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for16fangqu._1To8FangQuArm=0xff;
        devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for16fangqu._9To16FangQuArm=0xff;
    case 32:
        devBasicInfo.devspecialstate.sk485state.fangQuNums = fangqu;
        devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for32fangqu._1To8FangQuArm=0xff;
        devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for32fangqu._9To16FangQuArm=0xff;
        devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for32fangqu._17To24FangquArm=0xff;
        devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for32fangqu._25To31FangQuArm=0xff;
    }
    SK_SetDevBasicInfo(&devBasicInfo, hostno, SET_SK_DEVARM);
    return true;
}

// 撤防
bool CShikeAlarmDevice::DisableAlarm(const AFPROC::MCommand& mcmd)
{
    SK_AlarmDevHostNo hostno;
    hostno.SKHostNoH = mcmd.sk_hostno_h();
    hostno.SKHostNoL = mcmd.sk_hostno_l();
    hostno.SKHostNoGroup = mcmd.sk_host_group();

    SK_AlarmDevBasicInfo devBasicInfo;
    SK_GetDevBasicInfo(&devBasicInfo, hostno);
    SK_SetDevBasicInfo(&devBasicInfo, hostno, SET_SK_DEVDISARM);
    return true;
}

// 卸载网络
bool CShikeAlarmDevice::UninitNetEnv()
{
    UnInitEnviroment();
    return true;
}
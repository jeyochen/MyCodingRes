#include "ShikeAlarmDevice.h"

#include "../MainApp.h"

// ʱ�̱���ϵͳ�����ص�����
void RecevieAlarmCallBack(void* pvoid,int nType)
{
    SK_AlarmCodeInfo *pAlarm = (SK_AlarmCodeInfo*)pvoid;

    char szFangqu[128] = {0};
    char szAreaNo[16] = {0};
    switch(pAlarm->moreinfo.m_alarmstatefor485.m_485alarmstatefor8.CodeTypefor8)
    {
        //0��ʾ���ߣ�1��ʾ����,(2���߽���������3��ʾ���̽�����4��ʾ���̷���(���ֻ��8����������)�޷����ĸ���),
        //5��ʾ���߷�����ʱ������6��ʾ���߷�����ʱ����
    case 0:
        snprintf(szFangqu, sizeof(szFangqu) - 1, "%s", "���߷���");
        snprintf(szAreaNo, sizeof(szAreaNo) - 1, "%d", pAlarm->moreinfo.m_alarmstatefor485.m_485alarmstatefor8.CodeNo);
        break;;
    case 1:
        snprintf(szFangqu, sizeof(szFangqu) - 1, "%s", "�����߷���");
        snprintf(szAreaNo, sizeof(szAreaNo) - 1, "%d", pAlarm->moreinfo.m_alarmstatefor485.m_485alarmstatefor8.CodeNo);
    case 2:
        snprintf(szFangqu, sizeof(szFangqu) - 1, "%s", "���߽�������");
        snprintf(szAreaNo, sizeof(szAreaNo) - 1, "%s", "ң����");
    case 3:
        snprintf(szFangqu, sizeof(szFangqu) - 1, "%s", "���̽���");
        snprintf(szAreaNo, sizeof(szFangqu) - 1, "%s", "����");
    case 4:
        snprintf(szFangqu, sizeof(szFangqu) - 1, "%s", "���̷���");
        snprintf(szAreaNo, sizeof(szFangqu) - 1, "%s", "����");
        break;
    case 5:
        snprintf(szFangqu, sizeof(szFangqu) - 1, "%s", "������ʱ����");
        snprintf(szAreaNo, sizeof(szFangqu) - 1, "%d", pAlarm->moreinfo.m_alarmstatefor485.m_485alarmstatefor8.CodeNo);
        break;
    case 6:
        snprintf(szFangqu, sizeof(szFangqu) - 1, "%s", "������ʱ����");
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

//  ��ʼ������
bool CShikeAlarmDevice::InitNetEnv()
{
    m_port = MainApp.m_config.ReadInt("shike_alarm", "port", 1159);
    m_ip = MainApp.m_config.ReadString("shike_alarm", "ip", "0.0.0.0");

    char szIP[256] = {0};
    memset(szIP, 0x00, sizeof(szIP));
    snprintf(szIP, sizeof(szIP), m_ip.c_str());

    MainApp.m_log.WriteLog("��ʱsk-811����ͨ�ŵĵ�ַΪ:[%s][%d]", m_ip.c_str(), szIP);
    InitEnviroment(m_port, RecevieAlarmCallBack, szIP);
    return true;
}

// �ɼ�ʱ�̸澯������������״̬
bool CShikeAlarmDevice::CollectRegionStatus(COLLECT_DATA::MSKHostInfos& alarmhostinfo)
{
    // ��ȡ���������豸�ĸ���
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
        // ��ȡÿһ���豸�ķ���״̬
        SK_AlarmDevHostNo devHostNo;
        devHostNo.SKHostNoH = arrDev[i].skdevhostno.SKHostNoH; // ������Ÿ�λ�ֽ�
        devHostNo.SKHostNoL = arrDev[i].skdevhostno.SKHostNoL; // ������ŵ�λ�ֽ�
        devHostNo.SKHostNoGroup = arrDev[i].skdevhostno.SKHostNoGroup; // �������

        SK_AlarmDevBasicInfo devBasicInfo;
        // ��ȡ����������Ϣ
        SK_GetDevBasicInfo(&devBasicInfo, devHostNo);
        // ��������澯��Ϣ
        //SK_SetDevBasicInfo(&devBasicInfo,devBasicInfo.skdevhostno,SET_SK_CLEARALARM);

        int nFangquNum = devBasicInfo.devspecialstate.sk485state.fangQuNums;
        switch(nFangquNum)
        {
        case 8: // �˷���
            {
                COLLECT_DATA::MSKHostInfoItem *host8 = alarmhostinfo.add_sk_alarm();
                host8->set_ip(devBasicInfo.devIpAddress);
                host8->set_host_status(devBasicInfo.alarmdevArmState);
                host8->set_fangqu_type(8);
                int byteAlarm = devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for8fangqu._1To8FangQuArm;

                // ����1~8�������������Ĳ���״̬�� 1������0 ����
                for (int i = 0; i < 8; i++)
                {
                    COLLECT_DATA::MFangquStatusItem *status = host8->add_fangqu_status();
                    int n = byteAlarm >> (7-i);
                    n = byteAlarm & 0x1;
                    status->set_status(n);
                }

                // ����1~8����
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
        case 16:// ʮ������
            {
                COLLECT_DATA::MSKHostInfoItem *host16 = alarmhostinfo.add_sk_alarm();
                host16->set_ip(devBasicInfo.devIpAddress);
                host16->set_host_status(devBasicInfo.alarmdevArmState);
                host16->set_fangqu_type(16);
                int byteAlarm = devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for16fangqu._1To8FangQuArm;
                // ���� 1~8����������״̬
                for (int i = 0; i < 8; i++)
                {
                    COLLECT_DATA::MFangquStatusItem *status = host16->add_fangqu_status();
                    int n = byteAlarm >> (7-i);
                    n = byteAlarm & 0x1;
                    status->set_status(n);
                }

                // ����9~16����������״̬
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
        case 32: // ��ʮ������
            {
                COLLECT_DATA::MSKHostInfoItem *host32 = alarmhostinfo.add_sk_alarm();
                host32->set_ip(devBasicInfo.devIpAddress);
                host32->set_host_status(devBasicInfo.alarmdevArmState);
                host32->set_fangqu_type(32);

                // ����1~8
                int byteAlarm = devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for32fangqu._1To8FangQuArm;
                for (int i = 0; i < 8; i++)
                {
                    COLLECT_DATA::MFangquStatusItem *status = host32->add_fangqu_status();
                    int n = byteAlarm >> (7-i);
                    n = byteAlarm & 0x1;
                    status->set_status(n);
                }

                // ����9~16
                byteAlarm = devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for32fangqu._9To16FangQuArm;
                for (int i = 0; i < 8; i++)
                {
                    COLLECT_DATA::MFangquStatusItem *status = host32->add_fangqu_status();
                    int n = byteAlarm >> (7-i);
                    n = byteAlarm & 0x1;
                    status->set_status(n);
                }

                // ����17~24
                byteAlarm = devBasicInfo.devspecialstate.sk485state.m_485divfangqustate.m_485for32fangqu._17To24FangquArm;
                for (int i = 0; i < 8; i++)
                {
                    COLLECT_DATA::MFangquStatusItem *status = host32->add_fangqu_status();
                    int n = byteAlarm >> (7-i);
                    n = byteAlarm & 0x1;
                    status->set_status(n);
                }

                // ����25~32
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

// ��������
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

// ����
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

// ж������
bool CShikeAlarmDevice::UninitNetEnv()
{
    UnInitEnviroment();
    return true;
}
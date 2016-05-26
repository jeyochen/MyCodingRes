#ifndef INTERFACE_H_
#define INTERFACE_H_
// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� SK811_UDP_DLL_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// SK811_UDP_DLL_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���

#define  MAXARRAYSIZE 512
#define  UDPNETMSG WM_USER+100
#define  CALLBACKINFOTOUSER WM_USER+101
#define  QUITTHREAD WM_USER+102 //�����߳�
//#define NULL 0
typedef unsigned char byte;

/*�����������*/
typedef struct SK_AlarmDevHostNo
{
	byte SKHostNoH;//��Ÿ��ֽ�
	byte SKHostNoL;//��ŵ��ֽ�
	byte SKHostNoGroup;//���
}_SK_AlarmDevHostNo,*PSK_AlarmDevHostNo;
/*��������������*/
enum  SK_AlarmDevType
{
	SK_SK_CK=0,
	SK_SK485FORGeneral,
	SK_SK485FOR236K,
	VISTA_4110,
	VISTA_20_120,
	CC408,
	DSC_PC585,
	SK_SK136,
	SK_SK812_KR816,/* SK812 FOR ��¡816*/
	SK_SP4000
};
/*��������*/
enum DEVControlCmd
{
	SET_SK_ALARMHOSTNO=1,/*�޸��������*/
	SET_SK_ALARMNETINFO,/*�޸�������Ϣ*/
	SET_SK_ALARMRELAY,/*�޸������̵���״̬*/
	SET_SK_DEVARM,//���豸���в�������,���485�豸
	SET_SK_DEVDISARM,//���豸���г������������485�豸
	SET_SK_DIVFANGQUARM,//�ַ��������������485�豸
	SET_SK_DIVFANGQUPangLu,//�ַ�����·�����485�豸
	SET_SK_CLEARALARM,//����豸�ı��������485�豸
	SET_SK_JINGHAOSTATEFOR485,//����485�豸�ľ��ſ���״̬
	SET_SK_REFRESH485DEVSTATE,//ˢ��485�������豸״̬
	SET_SK_RET485DEV,//������λ
	SET_SK_SET485DEVRELAY,//�޸�485���������̵���״̬
	SET_SK_SNDKEYCMD,//����ģ���������
	/*�ص�����������*/
	SK_ALARMHOSTALARM_CALLBACK,/*������������*/
	SK_ALARMHOSTINFO_MODIFIY_CALLBACK,/*����������������ţ����߲��������޸ĳɹ�*/
	SK_ALARMHOSTNETINFO_MODIFY_CALLBACK,/*����������������Ϣ�޸ĳɹ�*/
	SK_ALARMHOSTSTATE_CALLBACK,/*����״̬�ص�*/
	SK_ALARMDEVOFFLINE,/*�豸����*/
	SK_ALARMDEVCONNECT/*�豸����*/
	
};
typedef struct SK_AlarmCodeInfo
{
	SK_AlarmDevType skalarmtype;//��������
	SK_AlarmDevHostNo skdevhostno;//�������
	char alarmTime[25];//����ʱ�� yyyy-MM-dd HH:mm:ss
	union AlarmMoreInfo
	{
		struct SK_SK136_AlarmState
		{
			byte eventdevhostnohigh;
			byte eventdevhostnolow;//�������
			byte eventtype;//�¼�������
			byte eventserial;//�¼������
			byte eventCode1;//�¼��ı���1
			byte eventCode2;//�¼��ı���2
			int CodeNo;//�¼��������
		}m_alarmstatefor136;
		struct SK_SK485_AlarmState
		{
				struct SK_485FOR8
				{
					//8��������
					int CodeTypefor8;//0��ʾ���ߣ�1��ʾ����,(2���߽���������3��ʾ���̽�����4��ʾ���̷���(���ֻ��8����������)�޷����ĸ���),5��ʾ���߷�����ʱ������6��ʾ���߷�����ʱ����
					int CodeNo;//�������

				}m_485alarmstatefor8;
				struct SK_485FOR16
				{
					//16��������
					int CodeTypefor16;//���16����������,ͬ��
					int CodeNo;
				}m_485alarmstate16;	
				struct SK_485FOR32
				{
					//32�������� ���߷���
					int CodeTypefor32;//���32������ͬ��
					int CodeNo;//�������
				}m_485alarmstate32;
				struct SK_485FORnewbus
				{	
					byte bytaddr;//������ַ
					int CodeTypeforbus;//0��ʾ���߷�����1��ʾ���÷�����2��ʾ���߽���������3��ʾ���̽���������4��ʾ���̷������
					int CodeNo;
				}m_485fornewbus;
				byte devhostalmstate[4];
		}m_alarmstatefor485;
		struct SK_SKCK_AlarmState
		{
			int CodeNo;//�������
			int CodeType;//0��ʾ���ߣ�1��ʾ����
		}m_alarmstateforskck;
		struct SK_SK812ForKR816_AlarmState
		{
			int AlarmType;//0��ʾ���߷�����1��ʾ���𱨾�
			int CodeNo;//�������
		}m_alarmstatforKR812;
		struct SK_VISTA4110_AlarmState
		{
			int CodeNo;//�������
			int CodeType;//0��ʾ���ߣ�1��ʾ���ߣ�2��ʾ��
		}m_alarmstatefor4110;
		struct SK_VISTA20_120_AlarmState
		{
			int CodeNo;//�������
			int CodeType;//0��ʾ���ߣ�1��ʾ���ߣ�2��ʾ��
		}m_alarmstatefor20_120;
	}moreinfo;
}_AlarmCodeInfo,*PAlarmCodeInfo;
/*
*/
//ͨ�����ù������ñ���������������Ϣ
typedef struct SK_ConfigNetInfo
{
	char localconfigIpaddress[20];//���������������IP��ַ
	char localconfigGateWayIpaddress[20];//����ip��ַ
	char localconfigCenterIpaddress[20];//���ĵ�IP��ַ
	char localconfigMaskIpaddress[20];//��������
	int localconfigPort;//���������Ķ˿ں�
	int localconfigRemotePort;//���ĵĶ˿ں�
	byte localconfigMac[6];//MAC��ַ
}_SK_ConfigNetInfo,*PSK_ConfigNetInfo;
/*���������Ļ�����Ϣ*/
typedef struct SK_AlarmDevBasicInfo
{
	SK_AlarmDevHostNo skdevhostno;//��ȡ������Ϣ��ʱ���Ų���Ϊ��
	char devIpAddress[20];//�豸��IP��ַ
	SK_ConfigNetInfo localconfigNetInfo;//ͨ�����ù������ñ���������������Ϣ
	byte devhandlePWD[2];//��������
	byte alarmdevArmState;//����������״̬1Ϊ������0Ϊ����
	SK_AlarmDevType AlarmDevType;//������������
	byte AlarmDevSoftVersion[3];//����汾
	byte PhoneLineState;//�绰��״̬0��ʾ�绰�߹��ϣ�1��ʾ�绰������
	byte RelayState;//�̵���״̬0��ʾ�̵����رգ�1��ʾ�̵�����
	union SK_DevSpecialStateByAlarmType
	{
		struct SK_SK485State
		{
			byte fangQuNums;//���߷���������8���ߣ�16���ߣ�32����,��ͨ485������ֵ����8,216ϵ������16������ϵ������32,����������ϵ������64
			byte cenhandletype;
			struct SK_SKDIV485Type
			{
				struct SK_485NorMal
				{
					byte _1To8FangQuArm;//!~8��������״̬
					byte wire_1To8FangQuArm;//����1~8����������״̬
					byte _1To8FangQuPangLu;//����1~8������·״̬
					byte wire_1To8FangQuPangLu;//����1~8������·״̬
					byte _1To8FangQuPWL;//����1~8������ƽ״̬
					byte wire_1ToFangQuPWL;//����1~8������ƽ״̬
				}m_485for8fangqu;
				struct SK_485for216Serial
				{
					byte _1To8FangQuArm;//����1~8����������״̬
					byte _9To16FangQuArm;//����9~16����������״̬
					byte wire_1To8tFangQuArm;//����1~8����������״̬���ݲ�֧��
					byte wire_9To16FangQuArm;//����9~16����������״̬���ݲ�֧��
					byte _1To8FangQuPangLu;//����1~8������·
					byte _9To16FangQuPangLu;//����9~16������·
					byte wire_1To8FangQuPangLu;//����1~8������·���ݲ�֧��
					byte wire_9To16FangQuPangLu;//����9~16������·���ݲ�֧��
					byte _1To8FangQuPWL;//����1~8������ƽ״̬
					byte _9To16FangQuPWL;//����9~16������ƽ״̬
				}m_485for16fangqu;
				struct SK_485for239Bus
				{
					//����ϵ��
					byte _1To8FangQuArm;//����1~8����������״̬
					byte _9To16FangQuArm;//����9~16����������״̬
					byte _17To24FangquArm;//����17~24����������״̬
					byte _25To31FangQuArm;//����25~32����������״̬
					byte _1To8FangQuPangLu;//����1~8������·
					byte _9To16FangQuPangLu;//����9~16������·
					byte _17To24FangQuPangLu;//����17~24������·
					byte _25To32FangQuPangLu;//����25~32������·
					byte _1To8FangQuPWL;//����1~8������ƽ״̬
				}m_485for32fangqu;
				struct SK_485forNewBusAlm
				{
					byte FangQuArm[32];//256����������״̬
					byte FangQuPangLu[32];//256����������״̬
					byte bytaddrgroup;//���Ƶڼ���
					byte _1To8FangQuPWL;//����1~8������ƽ״̬
					byte wire_1ToFangQuPWL;//����1~8������ƽ״̬
				}m_485fornewbusalm;

			}m_485divfangqustate;
			byte devhoststate;//����״̬
			byte devAcStateFor485;//���������Ľ�����״̬
			byte devDCStateFor485;//���������ı��õ��״̬
			byte devDialStateFor485;//�Ƿ����ڲ���
			byte PhoneStateFor485;//�绰��·�Ƿ�����
			byte TelPhoneStateFor485;//�ֻ�ģ���Ƿ�����
			byte JingHaoStateFor485;//485�����Ƿ�����
			byte LianDongSwitchFor485;//�����̵�������
		}sk485state;
		struct SK_CKSKState
		{
			byte _1To8FangQuPWL;//1~8������ƽ״̬
			byte _9To16FangQuPWL;//9~16������ƽ״̬
			byte _1To8FangQuPanglu;//1~8������·��Ϣ,���CK����
			byte _9To16FangQuPanglu;//9~16������·��Ϣ�����CK����
			byte keyAddress;//���̵�ַ
			byte Connectbetween811andAlarmDev;//��������������ģ�������״̬
			byte isDialing;//����ռ��
			//���������ֶ�����������ģ���������
			byte byt_KeyData[25];
			byte byt_len;
		}skckskstate;
		struct SK_CKKERONGState
		{
			//��������״̬
//			byte _1To8FangQuAlarmState;//1~8��������״̬ 
	//		byte _9To16FangQuAlarmState;//9~16��������״̬
			byte _1To8FangQuPWLState;//1~8������ƽ״̬
			byte _9To16FangquPWLState;//9~16������ƽ״̬
			bool IsBatteryfault;//����Ƿ��ѹ����
			bool IsAAreaFangQuArm;//A�������Ƿ񲼷�
			bool IsBAreaFangQuArm;//B�������Ƿ񲼷�
			bool IsAAreaFangQuDelayArm;//A�������Ƿ���ʱ����
			bool IsBAreaFangQuDelayArm;//B�������Ƿ���ʱ����
			
		}skKRstate;
		struct SK_VISTA4110_20_120
		{
			byte ACState;//������״̬
			byte DCState;//���õ��״̬
			byte bKuaiSuState;//����
			byte bWaiChuArm;//�������
			byte bLiuSuo;//���ز���
			byte bPangLu;//��·״̬
			byte keyAddress;//���̵�ַ			
			byte Connectbetween811andAlarmDev;//��������������ģ�������״̬
			//���������ֶ�����������ģ���������
			byte byt_KeyData[25];
			byte byt_len;
		}skvistastate;
		struct SK_SK136State
		{
			byte TiaoXian1;//����1
			byte TiaoXian2;//����2
			byte DianHuaXianOut;//��ǰ�绰����� 1��ʾGSM��0��ʾPSTN
			byte GSMSignalValue;//GSM�źŵ�ƽ0~7
			byte GSMState;//GSMģ��״̬0��ʾ���1��ʾ������2��ʾ���ţ�3��ʾ���壬4��ʾ���壬5��ʾ�ȴ���10��ʾ�һ���11��ʾδ֪״̬
			byte GSMIMEI[8];//GSM��IMEI ���ַ�
		}sk136state;
	} devspecialstate;
}_SK_AlarmDevBasicInfo,*PSK_AlarmDevBasicInfo;
/*������ţ����߲������룬���޸���*/
struct SK_AlarmDevBasicInfo_ModifyHostInfo
{
	struct SK_AlarmDevBasicInfo m_alarmdevinfo;
	struct SK_AlarmDevHostNo m_originalHostNo;
};
//����ص�����
typedef void (*pFAllCallback)(void* pvoid,int nType);
void  InitEnviroment(int nport,pFAllCallback callback1=NULL,char* ListenIPAddr=NULL);
void  UnInitEnviroment();//ж�����绷��
//��ȡ�豸�Ļ�����Ϣ
int  SK_GetDevBasicInfo(SK_AlarmDevBasicInfo* skalarmbasicinfo,SK_AlarmDevHostNo currentDevNo);
//�޸��豸������
int   SK_SetDevBasicInfo(SK_AlarmDevBasicInfo* skalarmbasicinfo,SK_AlarmDevHostNo currentDevNo,DEVControlCmd devcmd);
//��ȡ���е������豸
int  SK_GetCurrOnLineDev(SK_AlarmDevBasicInfo* onlineDevlst);
//��ȡ�������
int  SK_GetLastError();
#endif

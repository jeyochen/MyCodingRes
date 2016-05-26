#ifndef INTERFACE_H_
#define INTERFACE_H_
// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 SK811_UDP_DLL_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// SK811_UDP_DLL_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的

#define  MAXARRAYSIZE 512
#define  UDPNETMSG WM_USER+100
#define  CALLBACKINFOTOUSER WM_USER+101
#define  QUITTHREAD WM_USER+102 //结束线程
//#define NULL 0
typedef unsigned char byte;

/*报警主机编号*/
typedef struct SK_AlarmDevHostNo
{
	byte SKHostNoH;//编号高字节
	byte SKHostNoL;//编号低字节
	byte SKHostNoGroup;//组号
}_SK_AlarmDevHostNo,*PSK_AlarmDevHostNo;
/*报警主机的类型*/
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
	SK_SK812_KR816,/* SK812 FOR 科隆816*/
	SK_SP4000
};
/*控制命令*/
enum DEVControlCmd
{
	SET_SK_ALARMHOSTNO=1,/*修改主机编号*/
	SET_SK_ALARMNETINFO,/*修改网络信息*/
	SET_SK_ALARMRELAY,/*修改联动继电器状态*/
	SET_SK_DEVARM,//对设备进行布防操作,针对485设备
	SET_SK_DEVDISARM,//对设备进行撤防操作，针对485设备
	SET_SK_DIVFANGQUARM,//分防区布撤防，针对485设备
	SET_SK_DIVFANGQUPangLu,//分防区旁路，针对485设备
	SET_SK_CLEARALARM,//清除设备的报警，针对485设备
	SET_SK_JINGHAOSTATEFOR485,//控制485设备的警号开关状态
	SET_SK_REFRESH485DEVSTATE,//刷新485主机的设备状态
	SET_SK_RET485DEV,//主机复位
	SET_SK_SET485DEVRELAY,//修改485主机联动继电器状态
	SET_SK_SNDKEYCMD,//发送模拟键盘数据
	/*回调函数的命令*/
	SK_ALARMHOSTALARM_CALLBACK,/*报警主机报警*/
	SK_ALARMHOSTINFO_MODIFIY_CALLBACK,/*报警主机的主机编号，或者操作密码修改成功*/
	SK_ALARMHOSTNETINFO_MODIFY_CALLBACK,/*报警主机的网络信息修改成功*/
	SK_ALARMHOSTSTATE_CALLBACK,/*主机状态回调*/
	SK_ALARMDEVOFFLINE,/*设备掉线*/
	SK_ALARMDEVCONNECT/*设备上线*/
	
};
typedef struct SK_AlarmCodeInfo
{
	SK_AlarmDevType skalarmtype;//主机类型
	SK_AlarmDevHostNo skdevhostno;//主机编号
	char alarmTime[25];//报警时间 yyyy-MM-dd HH:mm:ss
	union AlarmMoreInfo
	{
		struct SK_SK136_AlarmState
		{
			byte eventdevhostnohigh;
			byte eventdevhostnolow;//主机编号
			byte eventtype;//事件的类型
			byte eventserial;//事件的序号
			byte eventCode1;//事件的编码1
			byte eventCode2;//事件的编码2
			int CodeNo;//事件防区编号
		}m_alarmstatefor136;
		struct SK_SK485_AlarmState
		{
				struct SK_485FOR8
				{
					//8防区主机
					int CodeTypefor8;//0表示有线，1表示无线,(2无线紧急报警，3表示键盘紧急，4表示键盘防拆(针对只有8防区的主机)无防区的概念),5表示有线防区延时报警，6表示无线防区延时报警
					int CodeNo;//防区编号

				}m_485alarmstatefor8;
				struct SK_485FOR16
				{
					//16防区主机
					int CodeTypefor16;//针对16防区的主机,同上
					int CodeNo;
				}m_485alarmstate16;	
				struct SK_485FOR32
				{
					//32防区主机 总线防区
					int CodeTypefor32;//针对32防区，同上
					int CodeNo;//防区编号
				}m_485alarmstate32;
				struct SK_485FORnewbus
				{	
					byte bytaddr;//辅助地址
					int CodeTypeforbus;//0表示总线防区，1表示内置防区，2表示无线紧急防区，3表示键盘紧急防区，4表示键盘防拆防区
					int CodeNo;
				}m_485fornewbus;
				byte devhostalmstate[4];
		}m_alarmstatefor485;
		struct SK_SKCK_AlarmState
		{
			int CodeNo;//防区编号
			int CodeType;//0表示有线，1表示无线
		}m_alarmstateforskck;
		struct SK_SK812ForKR816_AlarmState
		{
			int AlarmType;//0表示有线防区，1表示防拆报警
			int CodeNo;//防区编号
		}m_alarmstatforKR812;
		struct SK_VISTA4110_AlarmState
		{
			int CodeNo;//防区编号
			int CodeType;//0表示有线，1表示无线，2表示火警
		}m_alarmstatefor4110;
		struct SK_VISTA20_120_AlarmState
		{
			int CodeNo;//防区编号
			int CodeType;//0表示有线，1表示无线，2表示火警
		}m_alarmstatefor20_120;
	}moreinfo;
}_AlarmCodeInfo,*PAlarmCodeInfo;
/*
*/
//通过配置工具配置报警主机的网络信息
typedef struct SK_ConfigNetInfo
{
	char localconfigIpaddress[20];//分配给报警主机的IP地址
	char localconfigGateWayIpaddress[20];//网关ip地址
	char localconfigCenterIpaddress[20];//中心的IP地址
	char localconfigMaskIpaddress[20];//子网掩码
	int localconfigPort;//报警主机的端口号
	int localconfigRemotePort;//中心的端口号
	byte localconfigMac[6];//MAC地址
}_SK_ConfigNetInfo,*PSK_ConfigNetInfo;
/*报警主机的基本信息*/
typedef struct SK_AlarmDevBasicInfo
{
	SK_AlarmDevHostNo skdevhostno;//获取主机信息的时候编号不能为空
	char devIpAddress[20];//设备的IP地址
	SK_ConfigNetInfo localconfigNetInfo;//通过配置工具配置报警主机的网络信息
	byte devhandlePWD[2];//操作密码
	byte alarmdevArmState;//整机布撤防状态1为布防，0为撤防
	SK_AlarmDevType AlarmDevType;//报警主机类型
	byte AlarmDevSoftVersion[3];//软件版本
	byte PhoneLineState;//电话线状态0表示电话线故障，1表示电话线正常
	byte RelayState;//继电器状态0表示继电器关闭，1表示继电器打开
	union SK_DevSpecialStateByAlarmType
	{
		struct SK_SK485State
		{
			byte fangQuNums;//有线防区数量，8有线，16有线，32有线,普通485主机该值设置8,216系列设置16，总线系列设置32,新总线主机系列设置64
			byte cenhandletype;
			struct SK_SKDIV485Type
			{
				struct SK_485NorMal
				{
					byte _1To8FangQuArm;//!~8防区布撤状态
					byte wire_1To8FangQuArm;//无线1~8防区布撤防状态
					byte _1To8FangQuPangLu;//有线1~8防区旁路状态
					byte wire_1To8FangQuPangLu;//无线1~8防区旁路状态
					byte _1To8FangQuPWL;//有线1~8防区电平状态
					byte wire_1ToFangQuPWL;//无线1~8防区电平状态
				}m_485for8fangqu;
				struct SK_485for216Serial
				{
					byte _1To8FangQuArm;//有线1~8防区布撤防状态
					byte _9To16FangQuArm;//有线9~16防区布撤防状态
					byte wire_1To8tFangQuArm;//无线1~8防区布撤防状态，暂不支持
					byte wire_9To16FangQuArm;//无线9~16防区布撤防状态，暂不支持
					byte _1To8FangQuPangLu;//有线1~8防区旁路
					byte _9To16FangQuPangLu;//有线9~16防区旁路
					byte wire_1To8FangQuPangLu;//无线1~8防区旁路，暂不支持
					byte wire_9To16FangQuPangLu;//无线9~16防区旁路，暂不支持
					byte _1To8FangQuPWL;//有线1~8防区电平状态
					byte _9To16FangQuPWL;//有线9~16防区电平状态
				}m_485for16fangqu;
				struct SK_485for239Bus
				{
					//总线系列
					byte _1To8FangQuArm;//有线1~8防区布撤防状态
					byte _9To16FangQuArm;//有线9~16防区布撤防状态
					byte _17To24FangquArm;//有线17~24防区布撤防状态
					byte _25To31FangQuArm;//有线25~32防区布撤防状态
					byte _1To8FangQuPangLu;//有线1~8防区旁路
					byte _9To16FangQuPangLu;//有线9~16防区旁路
					byte _17To24FangQuPangLu;//有线17~24防区旁路
					byte _25To32FangQuPangLu;//有线25~32防区旁路
					byte _1To8FangQuPWL;//有线1~8防区电平状态
				}m_485for32fangqu;
				struct SK_485forNewBusAlm
				{
					byte FangQuArm[32];//256防区布撤防状态
					byte FangQuPangLu[32];//256防区布撤防状态
					byte bytaddrgroup;//控制第几个
					byte _1To8FangQuPWL;//有线1~8防区电平状态
					byte wire_1ToFangQuPWL;//无线1~8防区电平状态
				}m_485fornewbusalm;

			}m_485divfangqustate;
			byte devhoststate;//主机状态
			byte devAcStateFor485;//报警主机的交流电状态
			byte devDCStateFor485;//报警主机的备用电池状态
			byte devDialStateFor485;//是否正在拨号
			byte PhoneStateFor485;//电话线路是否正常
			byte TelPhoneStateFor485;//手机模块是否正常
			byte JingHaoStateFor485;//485警号是否正常
			byte LianDongSwitchFor485;//联动继电器开关
		}sk485state;
		struct SK_CKSKState
		{
			byte _1To8FangQuPWL;//1~8防区电平状态
			byte _9To16FangQuPWL;//9~16防区电平状态
			byte _1To8FangQuPanglu;//1~8防区旁路信息,针对CK主机
			byte _9To16FangQuPanglu;//9~16防区旁路信息，针对CK主机
			byte keyAddress;//键盘地址
			byte Connectbetween811andAlarmDev;//报警主机与网络模块的连接状态
			byte isDialing;//拨号占线
			//以下两个字段是用来发送模拟键盘数据
			byte byt_KeyData[25];
			byte byt_len;
		}skckskstate;
		struct SK_CKKERONGState
		{
			//科荣主机状态
//			byte _1To8FangQuAlarmState;//1~8防区报警状态 
	//		byte _9To16FangQuAlarmState;//9~16防区报警状态
			byte _1To8FangQuPWLState;//1~8防区电平状态
			byte _9To16FangquPWLState;//9~16防区电平状态
			bool IsBatteryfault;//电池是否低压报警
			bool IsAAreaFangQuArm;//A区防区是否布防
			bool IsBAreaFangQuArm;//B区防区是否布防
			bool IsAAreaFangQuDelayArm;//A区防区是否延时报警
			bool IsBAreaFangQuDelayArm;//B区防区是否延时报警
			
		}skKRstate;
		struct SK_VISTA4110_20_120
		{
			byte ACState;//交流电状态
			byte DCState;//备用电池状态
			byte bKuaiSuState;//快速
			byte bWaiChuArm;//外出布防
			byte bLiuSuo;//留守布防
			byte bPangLu;//旁路状态
			byte keyAddress;//键盘地址			
			byte Connectbetween811andAlarmDev;//报警主机与网络模块的连接状态
			//以下两个字段是用来发送模拟键盘数据
			byte byt_KeyData[25];
			byte byt_len;
		}skvistastate;
		struct SK_SK136State
		{
			byte TiaoXian1;//跳线1
			byte TiaoXian2;//跳线2
			byte DianHuaXianOut;//当前电话线输出 1表示GSM，0表示PSTN
			byte GSMSignalValue;//GSM信号电平0~7
			byte GSMState;//GSM模块状态0表示活动，1表示保留，2表示拨号，3表示回铃，4表示振铃，5表示等待，10表示挂机，11表示未知状态
			byte GSMIMEI[8];//GSM的IMEI 码字符
		}sk136state;
	} devspecialstate;
}_SK_AlarmDevBasicInfo,*PSK_AlarmDevBasicInfo;
/*主机编号，或者操作密码，被修改了*/
struct SK_AlarmDevBasicInfo_ModifyHostInfo
{
	struct SK_AlarmDevBasicInfo m_alarmdevinfo;
	struct SK_AlarmDevHostNo m_originalHostNo;
};
//定义回调函数
typedef void (*pFAllCallback)(void* pvoid,int nType);
void  InitEnviroment(int nport,pFAllCallback callback1=NULL,char* ListenIPAddr=NULL);
void  UnInitEnviroment();//卸载网络环境
//获取设备的基本信息
int  SK_GetDevBasicInfo(SK_AlarmDevBasicInfo* skalarmbasicinfo,SK_AlarmDevHostNo currentDevNo);
//修改设备的属性
int   SK_SetDevBasicInfo(SK_AlarmDevBasicInfo* skalarmbasicinfo,SK_AlarmDevHostNo currentDevNo,DEVControlCmd devcmd);
//获取所有的在线设备
int  SK_GetCurrOnLineDev(SK_AlarmDevBasicInfo* onlineDevlst);
//获取错误代码
int  SK_GetLastError();
#endif

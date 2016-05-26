#pragma  once

// 设备类型编码
enum DeviceTypeEnum{
    SIX_LINE_DEVICE = 1000, // 六路智能照明控制器
    TEMPER_HUMDI_CTRL_DEVICE, // 温湿度控制器
    ALARM_HOST_DEVICE, // 告警主机
    UPS_DEVICE, //UPS电压设备
    LINUX_HOST_DEVICE, // 工业控制主机机

    MAX_DEVICE_TYPE, // 设备类型编码上限值
};

// 命令分类
enum CommandClassCodeEnum{
    TIMING_EXEC = 100,  // 定时执行
    REAL_TIME_EXEC,     // 实时执行


    MAX_CMD_CLASS, // 命令分类编码上限值
};

// 命令功能
enum CmdFuncCodeEnum{
    /*智能照明控制器命令*/
    COLLECTOR_SIX_LINE_DC = 10000, // 采集智能照明控制器的各路电流
    COLLECTOR_SIX_LINE_STATUS, // 采集智能照明控制器的各路开关状态
    EXEC_SIX_LINE_CTRL_CMD, // 执行智能照明控制器命令

    /* 温湿度及只智能空调遥控器命令*/
    COLLECTOR_TEMPE_HUMIDI, // 采集温湿度
    EXEC_AIR_CODITION_COMD, // 执行空调遥控命令  


    /* 时刻报警系统命令 */
    COLLECTOR_ALARM_STATUS, // 采集告警主机状态
    ALARM_DEFENSE, // 告警主机布防
    ALARM_DISARM, // 告警主机撤防

    /* UPS 命令 */
    COLLECTOR_UPS_VOLT, // 采集UPS电压
    COLLECTOR_UPS_DC, // 采集UPS电流

    /* 工业控制主机命令 */
    COLLECTOR_LINUX_CPU_USAGE, // CPU使用率
    COLLECTOR_LINUX_MEM_USAGE, // 内存使用率
    COLLECTOR_LINUX_DISK_SPACE, // 磁盘空间

    MAX_CMD_FUNC, // 命令功能编码上限值
};
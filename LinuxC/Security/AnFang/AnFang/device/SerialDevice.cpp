#include "SerialDevice.h"
#include "../MainApp.h"

#include <iomanip>

CSerialDevice::CSerialDevice(void)
{
}


CSerialDevice::~CSerialDevice(void)
{
}


// 获取六路开关的电流量
bool CSerialDevice::GetSixlineDC(int port, const AFPROC::MCommand& mcmd, COLLECT_DATA::MSixLineDC& dc)
{
    if (false == m_sixLineObj.OpenSerialPort (port))
    {
        MainApp.m_log.WriteLog("%s:%d Open serial port %d failed", __FILE__, __LINE__, port);
        return false;
    }
    if (false == m_sixLineObj.SetBaudrate (9600))
    {
        MainApp.m_log.WriteLog("%s:%d SetBaudrate fail: %d", __FILE__, __LINE__, 9600);
        m_sixLineObj.ClosePort();
        return false;
    }
    if (false == m_sixLineObj.SetParity (8, 1, 'N'))
    {
        MainApp.m_log.WriteLog("%s:%d SetParity fail", __FILE__, __LINE__);
        m_sixLineObj.ClosePort();
        return false;
    }

    std::string strCmd;
    char addr[4] = {0};
    snprintf(addr, sizeof(addr), "%02X", mcmd.d_address());
    strCmd = addr;
    strCmd += mcmd.execommand();

    MainApp.m_log.WriteLog("六路开关采集指令：%s", strCmd.c_str());
    if(false == m_sixLineObj.WriteData(strCmd.c_str(), strCmd.length()))
    {
        MainApp.m_log.WriteLog("%s:%d 发送命令失败", __FILE__, __LINE__);
        m_sixLineObj.ClosePort();
        return false;
    }
    
    unsigned char response[READ_BUF_SIZE] = {0};
    int len = m_sixLineObj.ReadData (response, sizeof(response) - 1);

    if (len > 0)
    {
        //从机反馈(设备)：地址(1个字节)+功能码(1个字节,0x03)+字节数(1个字节，寄存器数量x2) +有效数据+ crc校验(2个字节)
        int byteSize =  response[2];
        MainApp.m_log.WriteLog("寄存器数量:%d", byteSize/2);
        for (int i = 0; i < byteSize/2; i++)
        {
            float part1 = response[3 + 2*i];
            float part2 = response[3 + 2*i + 1]/100.0;
            dc.add_line_dc(part1 + part2);
            //MainApp.m_log.WriteLog("value=%.2f", part1 + part2);
        }
    }
    else
    {
        MainApp.m_log.WriteLog("%s:%d 接收六路开关返回数据失败", __FILE__, __LINE__);
    }
    m_sixLineObj.ClosePort();
}

// 获取六路开关的状态
bool CSerialDevice::GetSixlineStatus(int port, const AFPROC::MCommand& mcmd, COLLECT_DATA::MSixLineStatus& status)
{
    if (false == m_sixLineObj.OpenSerialPort (port))
    {
        MainApp.m_log.WriteLog("%s:%d Open serial port %d failed", __FILE__, __LINE__, port);
        return false;
    }
    if (false == m_sixLineObj.SetBaudrate (9600))
    {
        MainApp.m_log.WriteLog("%s:%d SetBaudrate fail: %d", __FILE__, __LINE__, 9600);
        m_sixLineObj.ClosePort();
        return false;
    }
    if (false == m_sixLineObj.SetParity (8, 1, 'N'))
    {
        MainApp.m_log.WriteLog("%s:%d SetParity fail", __FILE__, __LINE__);
        m_sixLineObj.ClosePort();
        return false;
    }

    std::string strCmd;
    char addr[4] = {0};
    snprintf(addr, sizeof(addr), "%02X", mcmd.d_address());
    strCmd = addr;
    strCmd += mcmd.execommand();

    MainApp.m_log.WriteLog("六路开关采集指令：%s", strCmd.c_str());
    if(false == m_sixLineObj.WriteData(strCmd.c_str(), strCmd.length()))
    {
        MainApp.m_log.WriteLog("%s:%d 发送命令失败", __FILE__, __LINE__);
        m_sixLineObj.ClosePort();
        return false;
    }

    unsigned char response[READ_BUF_SIZE] = {0};
    int len = m_sixLineObj.ReadData (response, sizeof(response) - 1);

    if (len > 0)
    {
        int byteSize =  response[2];
        for (int i = 0; i < byteSize/2; i++)
        {
            //0X00AA为开， 0X0055为关
            if ((0XAA == response[3 + 2*i +1]) || (0xaa == response[3 + 2*i +1]))
            {
                status.add_line_status(true);
            }
            else
            {
                status.add_line_status(false);
            }
        }

    }
    else
    {
        MainApp.m_log.WriteLog("%s:%d 接收六路开关返回数据失败", __FILE__, __LINE__);
    }
    m_sixLineObj.ClosePort();
}

// 执行六路开关控制命令
bool CSerialDevice::ExecSixLineCtrlCmd(int port, const AFPROC::MCommand& mcmd)
{
    bool  bRes = false;
    if (false == m_sixLineObj.OpenSerialPort (port))
    {
        MainApp.m_log.WriteLog("%s:%d Open serial port %d failed", __FILE__, __LINE__, port);
        return false;
    }
    if (false == m_sixLineObj.SetBaudrate (9600))
    {
        MainApp.m_log.WriteLog("%s:%d SetBaudrate fail: %d", __FILE__, __LINE__, 9600);
        m_sixLineObj.ClosePort();
        return false;
    }
    if (false == m_sixLineObj.SetParity (8, 1, 'N'))
    {
        MainApp.m_log.WriteLog("%s:%d SetParity fail", __FILE__, __LINE__);
        m_sixLineObj.ClosePort();
        return false;
    }

    std::string strCmd;
    char addr[4] = {0};
    snprintf(addr, sizeof(addr), "%02X", mcmd.d_address());
    strCmd = addr;
    strCmd += mcmd.execommand();

    MainApp.m_log.WriteLog("六路开关指令：%s", strCmd.c_str());
    if(false == m_sixLineObj.WriteData(strCmd.c_str(), strCmd.length()))
    {
        MainApp.m_log.WriteLog("%s:%d 发送命令失败", __FILE__, __LINE__);
        m_sixLineObj.ClosePort();
        return false;
    }

    unsigned char response[READ_BUF_SIZE] = {0};
    int len = m_sixLineObj.ReadData (response, sizeof(response) - 1);

    if (8 == len)
    {
        MainApp.m_log.WriteLog("%s:%d 六路开关控制命令执行成功", __FILE__, __LINE__);
        bRes = true;
    }
    else
    {
        bRes = false;
        MainApp.m_log.WriteLog("%s:%d 接收六路开关返回数据失败", __FILE__, __LINE__);
    }
    m_sixLineObj.ClosePort();

    return bRes;
}

// 采集温湿度
bool CSerialDevice::GetTemperatureHumidity(int port, const AFPROC::MCommand& mcmd, COLLECT_DATA::MTemperHumidityValue& mvalue)
{
    if (false == m_temperHumiCtrlObj.OpenSerialPort (port))
    {
        MainApp.m_log.WriteLog("%s:%d Open serial port %d failed", __FILE__, __LINE__, port);
        return false;
    }
    if (false == m_temperHumiCtrlObj.SetBaudrate (9600))
    {
        MainApp.m_log.WriteLog("%s:%d SetBaudrate fail: %d", __FILE__, __LINE__, 9600);
        m_temperHumiCtrlObj.ClosePort();
        return false;
    }
    if (false == m_temperHumiCtrlObj.SetParity (8, 1, 'N'))
    {
        MainApp.m_log.WriteLog("%s:%d SetParity fail", __FILE__, __LINE__);
        m_temperHumiCtrlObj.ClosePort();
        return false;
    }

    std::string strCmd;
    char addr[4] = {0};
    snprintf(addr, sizeof(addr), "%02X", mcmd.d_address());
    strCmd = addr;
    strCmd += mcmd.execommand();

    MainApp.m_log.WriteLog("温湿度控制器采集指令：%s", strCmd.c_str());
    if(false == m_temperHumiCtrlObj.WriteData(strCmd.c_str(), strCmd.length()))
    {
        MainApp.m_log.WriteLog("%s:%d 发送命令失败", __FILE__, __LINE__);
        m_temperHumiCtrlObj.ClosePort();
        return false;
    }

    unsigned char response[READ_BUF_SIZE] = {0};
    int len = m_temperHumiCtrlObj.ReadData (response, sizeof(response) - 1);

    if (len > 0)
    {
        for (int i = 0; i < len; i++)
        {
            if (0X4 == response[i] && 0X4 == response[i+1])
            {
                // 取温度值
                int start = i + 2;
                unsigned short int temper  = 0;
                temper = response[start++] << 8;
                temper |= response[start++];
                mvalue.set_temperature(temper/10.0);

                // 取湿度值
                unsigned short int humi = 0;
                humi = response[start++] << 8;
                humi |= response[start];
                mvalue.set_humidity(humi/10.0);
            }
        }
    }
    else
    {
        MainApp.m_log.WriteLog("%s:%d 接收温湿度控制器返回数据失败", __FILE__, __LINE__);
    }
    m_temperHumiCtrlObj.ClosePort();
    return true;
}

// 执行空调遥控器命令
bool CSerialDevice::ExcAirConditonCmd(int port, const AFPROC::MCommand& mcmd)
{
    if (false == m_temperHumiCtrlObj.OpenSerialPort (port))
    {
        MainApp.m_log.WriteLog("%s:%d Open serial port %d failed", __FILE__, __LINE__, port);
        return false;
    }
    if (false == m_temperHumiCtrlObj.SetBaudrate (9600))
    {
        MainApp.m_log.WriteLog("%s:%d SetBaudrate fail: %d", __FILE__, __LINE__, 9600);
        m_temperHumiCtrlObj.ClosePort();
        return false;
    }
    if (false == m_temperHumiCtrlObj.SetParity (8, 1, 'N'))
    {
        MainApp.m_log.WriteLog("%s:%d SetParity fail", __FILE__, __LINE__);
        m_temperHumiCtrlObj.ClosePort();
        return false;
    }

    std::string strCmd;
    char addr[4] = {0};
    snprintf(addr, sizeof(addr), "%02X", mcmd.d_address());
    strCmd = addr;
    strCmd += mcmd.execommand();

    MainApp.m_log.WriteLog("智能遥控器制指令：%s", strCmd.c_str());
    if(false == m_temperHumiCtrlObj.WriteData(strCmd.c_str(), strCmd.length()))
    {
        MainApp.m_log.WriteLog("%s:%d 发送命令失败", __FILE__, __LINE__);
        m_temperHumiCtrlObj.ClosePort();
        return false;
    }
    m_temperHumiCtrlObj.ClosePort();

    return true;
}

// LZSerialPortToolDlg.h : 头文件
//

#pragma once
#include "afxwin.h"

#include "MySerialPort.h"
#include "SerialPort.h"
#include <map>
#include <vector>

// CLZSerialPortToolDlg 对话框
class CLZSerialPortToolDlg : public CDialogEx
{
// 构造
public:
	CLZSerialPortToolDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_LZSERIALPORTTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedMfcbuttonSend();
	DECLARE_MESSAGE_MAP()

public:
    CComboBox m_cmbSendPort; // 发送端口号
    CComboBox m_cmbSendBaud; // 发送端波特率
    CComboBox m_cmbSendCheckbits; // 发送端校验位
    CComboBox m_cmbSendDatabits; // 发送端数据位
    CComboBox m_cmbSendStopbits; // 发送端停止位

    CComboBox m_cmbRecvPort; // 接收端口号
    CComboBox m_cmbRecvBaud; // 接收端波特率
    CComboBox m_cmbRecvDatabits; // 接收端数据位
    CComboBox m_cmbRecvCheckbits; // 接收端校验位
    CComboBox m_cmbRecvStopbits; // 接收端停止位

    CEdit m_editCmd; // 发送的命令
    CEdit m_editSendShow; // 发送端显示窗口
    CEdit m_editRecvShow; // 接收端显示窗口


public:
    // 以对话框的方式显示提示信息
    void ShowTipMessage(const CString& tip);
    // 显示发送端口窗口的信息
    void ShowSendMessage(CString msg, int srflag);
    // 显示接收接收窗口的信息
    void ShowRecvMessage(CString msg, int srflag);

protected:
    // 显示错误信息
    void GetErrorDesc();
    // 获取当前机器上的串口列表
    void GetSerialPortList();
    // 初始化界面控件
    void InitControle();

    // 启动发送线程
    bool StartSendThread();
    // 启动接收线程
    bool StartRecvThread();

private:
    CMySerialPort m_sendPort;
    CMySerialPort m_recvPort;
    CSerialPort m_testPort;
    HANDLE m_sendHandle;
    HANDLE m_recvHandle;
    CWinThread* m_sendThread;
    CWinThread* m_recvThread;
    CString m_strSendPortName;
    CString m_strRecvPortName;

    std::map<CString, DWORD>  m_mapBaud; // 波特率key-value
    std::map<CString, int> m_mapDataBits; // 数据位数key-value
    std::map<CString, int> m_mapCheck; // 校验位key-value
    std::map<CString, int> m_mapStopbits; // 停止位key-value
    std::map<CString, CString> m_mapCmdDesc; // 命令和命令描述
public:
    afx_msg void OnBnClickedMfcbuttonOpen();
    afx_msg void OnBnClickedMfcbuttonClose();
    afx_msg void OnBnClickedMfcbuttonClear();
    afx_msg void OnClose();
    afx_msg void OnBnClickedMfcbuttonFresh();
    afx_msg void OnBnClickedMfcbtnctlLog();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
};


// LZSerialPortToolDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"

#include "MySerialPort.h"
#include "SerialPort.h"
#include <map>
#include <vector>

// CLZSerialPortToolDlg �Ի���
class CLZSerialPortToolDlg : public CDialogEx
{
// ����
public:
	CLZSerialPortToolDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_LZSERIALPORTTOOL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedMfcbuttonSend();
	DECLARE_MESSAGE_MAP()

public:
    CComboBox m_cmbSendPort; // ���Ͷ˿ں�
    CComboBox m_cmbSendBaud; // ���Ͷ˲�����
    CComboBox m_cmbSendCheckbits; // ���Ͷ�У��λ
    CComboBox m_cmbSendDatabits; // ���Ͷ�����λ
    CComboBox m_cmbSendStopbits; // ���Ͷ�ֹͣλ

    CComboBox m_cmbRecvPort; // ���ն˿ں�
    CComboBox m_cmbRecvBaud; // ���ն˲�����
    CComboBox m_cmbRecvDatabits; // ���ն�����λ
    CComboBox m_cmbRecvCheckbits; // ���ն�У��λ
    CComboBox m_cmbRecvStopbits; // ���ն�ֹͣλ

    CEdit m_editCmd; // ���͵�����
    CEdit m_editSendShow; // ���Ͷ���ʾ����
    CEdit m_editRecvShow; // ���ն���ʾ����


public:
    // �ԶԻ���ķ�ʽ��ʾ��ʾ��Ϣ
    void ShowTipMessage(const CString& tip);
    // ��ʾ���Ͷ˿ڴ��ڵ���Ϣ
    void ShowSendMessage(CString msg, int srflag);
    // ��ʾ���ս��մ��ڵ���Ϣ
    void ShowRecvMessage(CString msg, int srflag);

protected:
    // ��ʾ������Ϣ
    void GetErrorDesc();
    // ��ȡ��ǰ�����ϵĴ����б�
    void GetSerialPortList();
    // ��ʼ������ؼ�
    void InitControle();

    // ���������߳�
    bool StartSendThread();
    // ���������߳�
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

    std::map<CString, DWORD>  m_mapBaud; // ������key-value
    std::map<CString, int> m_mapDataBits; // ����λ��key-value
    std::map<CString, int> m_mapCheck; // У��λkey-value
    std::map<CString, int> m_mapStopbits; // ֹͣλkey-value
    std::map<CString, CString> m_mapCmdDesc; // �������������
public:
    afx_msg void OnBnClickedMfcbuttonOpen();
    afx_msg void OnBnClickedMfcbuttonClose();
    afx_msg void OnBnClickedMfcbuttonClear();
    afx_msg void OnClose();
    afx_msg void OnBnClickedMfcbuttonFresh();
    afx_msg void OnBnClickedMfcbtnctlLog();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
};


// ChangeSizeDlg.h : ͷ�ļ�
//

#pragma once


// CChangeSizeDlg �Ի���
class CChangeSizeDlg : public CDialogEx
{
// ����
public:
	CChangeSizeDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_CHANGESIZE_DIALOG };

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
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();

protected:
    BOOL LoadGdiplus();
};


// ChangeSizeDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ChangeSize.h"
#include "ChangeSizeDlg.h"
#include "afxdialogex.h"
#include "CompressPicture.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CChangeSizeDlg �Ի���




CChangeSizeDlg::CChangeSizeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CChangeSizeDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CChangeSizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CChangeSizeDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CChangeSizeDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CChangeSizeDlg ��Ϣ�������

BOOL CChangeSizeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
    // ���ض�̬��
    LoadGdiplus();
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CChangeSizeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CChangeSizeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CChangeSizeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CChangeSizeDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString strOldFileName;

    // ������Ի���
    CFileDialog fdlg(TRUE, _T(".jpg"), NULL, OFN_HIDEREADONLY, 
        _T("jpg|*.jpg|jpeg|*.jpeg|bmp|*.bmp|png|*.png|gif|*.gif|tiff|*.tiff|*|*.*||"));

    if (IDOK != fdlg.DoModal())
    {
        return;
    }

    strOldFileName = fdlg.GetPathName();
    if (0 >= strOldFileName.GetLength())
    {
        MessageBox(_T("��ѡ���ļ�"));
        return;
    }

    // ��ȡ�ļ���չ�����ļ�·�����ļ���
    LPWSTR lpExt = PathFindExtensionW(strOldFileName); 

    // ����Ƿ�����Ҫѹ����ͼƬ��ʽ
    CString strExt(lpExt);
    if (0 != strExt.CompareNoCase(_T(".jpg"))  &&
        0 != strExt.CompareNoCase(_T(".jpeg")) &&
        0 != strExt.CompareNoCase(_T(".bmp"))  &&
        0 != strExt.CompareNoCase(_T(".png"))  &&
        0 != strExt.CompareNoCase(_T(".tiff")) &&
        0 != strExt.CompareNoCase(_T(".gif")))
    {
        return;
    }
    
    CCompressPicture pressPic;
    pressPic.CompressPicture(strOldFileName);
}


BOOL CChangeSizeDlg::LoadGdiplus()
{
    // ���gdiplus.dll�Ƿ����,�Ѵ��򷵻�
    if (PathFileExists(_T("./GdiPlus.dll")))
    {
        return TRUE;
    }

    HRSRC hRsrc = FindResourceW(NULL, MAKEINTRESOURCE(IDR_GDIPLUS), _T("dll"));
    if (NULL == hRsrc)
    {
        return FALSE;
    }

    DWORD dwSize =  SizeofResource(NULL, hRsrc);
    if (0 >= dwSize)
    {
        return FALSE;
    }

    HGLOBAL hGlobal =  LoadResource(NULL, hRsrc);
    if (NULL == hGlobal)
    {
        return FALSE;
    }

    LPVOID pBuffer = LockResource(hGlobal);
    if (NULL == pBuffer)
    {
        return FALSE;
    }

    CFile file(_T("./GdiPlus.dll"), CFile::modeCreate|CFile::modeWrite);
    if (NULL == file)
    {
        return FALSE;
    }
    file.Write(pBuffer, dwSize);
    file.Close();
    FreeResource(hGlobal);

    return TRUE;
}
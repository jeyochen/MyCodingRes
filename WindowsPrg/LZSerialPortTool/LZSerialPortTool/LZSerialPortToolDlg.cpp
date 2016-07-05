
// LZSerialPortToolDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "LZSerialPortTool.h"
#include "LZSerialPortToolDlg.h"
#include "afxdialogex.h"

#include "setupapi.h"
#include "devguid.h"
#include "afxmt.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define  CLEAR_TIME 1
#define  SEND_MSG 1
#define  RECV_MSG 2
#define  BUF_SIZE 1024
#define  SHOWTIP ((CLZSerialPortToolDlg*)(theApp.m_pMainWnd))->ShowTipMessage
#define  SHOWSEND ((CLZSerialPortToolDlg*)(theApp.m_pMainWnd))->ShowSendMessage
#define  SHOWRECV ((CLZSerialPortToolDlg*)(theApp.m_pMainWnd))->ShowRecvMessage

bool g_isruning = false; // �߳��Ƿ�������� true �������У�false ֹͣ����
bool g_startWork = false; // �����Ƿ�ʼ������true��ʼ������ false ������
//CCriticalSection g_sendSection;
//CCriticalSection g_recvSection;
CRITICAL_SECTION g_section;

CString g_cmd = "";
DWORD g_send_len;

UINT SendPortThreadFunc(LPVOID lpParam)
{
    CMySerialPort *pSerialPort = (CMySerialPort *)lpParam;
    DWORD dwThread = GetCurrentThreadId();

    unsigned char sRecvBuf[BUF_SIZE] = {0};
    unsigned char sSendBuf[BUF_SIZE] = {0};
    CString strSendTip;
    CString strRecvTip;
    DWORD dwSenbytes = 0;
    DWORD dwRecvBytes = 0;
    bool b_newcmd = false;
    bool b_isfirst = true;

    LOG.debug_log("����[%s]�߳�����", (LPCSTR)pSerialPort->GetPortName(),
        (LPCSTR)g_cmd);
    while(g_isruning)
    {
        if (!g_startWork)
        {
            Sleep(10);
            continue;
        }

        CString cmd;

        // ���뷢���ٽ���, ���͵�һ������
        EnterCriticalSection(&g_section);
        if(g_cmd.GetLength() > 0)
        {
            //srand(time(NULL));
            //int n = rand()%10000;
            //CString strRand;
            //strRand.Format("%d", n);
            //g_cmd += strRand;
            // �������ݵ�Ŀ�Ĵ���
            memset(sSendBuf, 0, sizeof(sSendBuf));
            memcpy(sSendBuf, (LPCSTR)g_cmd, g_cmd.GetLength());

            // ��������ͨ�ŵĹ����У��û��ֹ�������������
            if (false == b_isfirst)
            {
                b_newcmd = true;
            }
            else
            {
                // ��һ��ͨ�ţ�����������
                dwSenbytes = pSerialPort->WriteData(sSendBuf, g_cmd.GetLength());
                if (dwSenbytes > 0)
                {
                    SHOWSEND(g_cmd, SEND_MSG);
                    g_cmd = "";  
                }
                b_isfirst = false;
            }
        }
        LeaveCriticalSection(&g_section);
        Sleep(10);

        // ������շ����ٽ���
        EnterCriticalSection(&g_section);
        // ���Լ��Ĵ��ڽ�������
        memset(sRecvBuf, 0, sizeof(sRecvBuf));
        dwRecvBytes = pSerialPort->ReadData(sRecvBuf, BUF_SIZE /*g_send_len*/);
        if (dwRecvBytes > 0)
        {
            CString strRecv(sRecvBuf);
            //SHOWSEND(strRecv, RECV_MSG);
        }
        else if(-1 == dwRecvBytes)
        {
            LOG.err_log("����[%s]���������쳣", 
                (LPCSTR)pSerialPort->GetPortName());
        }

        // ���յ������ݷ��͸��Զ�
        if (dwRecvBytes > 0)
        {
            // �����û��ֹ��·���������
            if (b_newcmd)
            {
                memset(sRecvBuf, 0, sizeof(sRecvBuf));
                memcpy(sRecvBuf, (LPCSTR)g_cmd, g_cmd.GetLength());
                dwRecvBytes = g_cmd.GetLength();
                g_cmd = "";
                b_newcmd = false;
            }

            // ���ͽ��յ������ݸ�Ŀ�Ĵ���
            dwSenbytes = pSerialPort->WriteData(sRecvBuf, dwRecvBytes);
            if (dwSenbytes > 0)
            {
                CString strSend(sRecvBuf);
                SHOWSEND(strSend, SEND_MSG);
            }
            else if (-1 == dwSenbytes)
            {
                LOG.err_log("����[%s]���������쳣", 
                    (LPCSTR)pSerialPort->GetPortName());
            }
        }
        LeaveCriticalSection(&g_section);
        
        if (-1 == dwRecvBytes)
        {
            // �˳��߳�
            LOG.warn_log("�����߳��쳣�˳�");
            break;
        }
    }

    LOG.debug_log("����[%s]�߳������˳�", (LPCSTR)pSerialPort->GetPortName(),
        (LPCSTR)g_cmd);
    return 0;
}

UINT RecvPortThreadFunc(LPVOID lpParam)
{
    CMySerialPort *pSerialPort = (CMySerialPort *)lpParam;

    unsigned char sRecvBuf[BUF_SIZE] = {0};
    unsigned char sSendBuf[BUF_SIZE] = {0};
    CString strRecv;
    CString strSendTip;
    CString strRecvTip;
    DWORD dwSenbytes = 0;
    DWORD dwRecvBytes = 0;

    LOG.debug_log("����[%s]�߳�����", (LPCSTR)pSerialPort->GetPortName(),
        (LPCSTR)g_cmd);
    while(g_isruning)
    {
        if (!g_startWork)
        {
            Sleep(10);
            continue;
        }

        // ���Լ��Ĵ��ڽ�������
        //g_recvSection.Lock();
        // ������պͷ����ٽ���
        EnterCriticalSection(&g_section);
        memset(sRecvBuf, 0, sizeof(sRecvBuf));
        dwRecvBytes = pSerialPort->ReadData(sRecvBuf, BUF_SIZE /*g_send_len*/);
        if (dwRecvBytes > 0)
        {
            CString strRecv(sRecvBuf);
            SHOWRECV(strRecv, RECV_MSG);
        }
        else if (-1 == dwRecvBytes)
        {
            LOG.err_log("����[%s]���������쳣", 
                (LPCSTR)pSerialPort->GetPortName());
        }

        // �ѽ��յ��������ڷ��ͳ�
        if (dwRecvBytes > 0)
        {
            dwSenbytes = pSerialPort->WriteData(sRecvBuf, dwRecvBytes);
            if (dwSenbytes > 0)
            {
                //CString strRecv(sRecvBuf);
                //SHOWRECV(strRecv, SEND_MSG);
            }
        }
        else if (-1 == dwSenbytes)
        {
            // �˳��߳�
            LOG.err_log("����[%s]���������쳣", 
                (LPCSTR)pSerialPort->GetPortName());
        }
        LeaveCriticalSection(&g_section);
        Sleep(10);
    }

    LOG.debug_log("����[%s]�߳������˳�", (LPCSTR)pSerialPort->GetPortName(),
        (LPCSTR)g_cmd);
    return 0;
}

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
public:
//    afx_msg void OnClose();
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
//    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CLZSerialPortToolDlg �Ի���




CLZSerialPortToolDlg::CLZSerialPortToolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLZSerialPortToolDlg::IDD, pParent)
    ,m_sendHandle(INVALID_HANDLE_VALUE)
    ,m_recvHandle(INVALID_HANDLE_VALUE)
    ,m_sendThread(NULL)
    ,m_recvThread(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLZSerialPortToolDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CMB_SEND_PORT, m_cmbSendPort);
    DDX_Control(pDX, IDC_CMB_SEND_BUAD, m_cmbSendBaud);
    DDX_Control(pDX, IDC_CMB_SEND_CHECKBITS, m_cmbSendCheckbits);
    DDX_Control(pDX, IDC_CMB_SEND_DATABITS, m_cmbSendDatabits);
    DDX_Control(pDX, IDC_CMB_SEND_TOPBITS, m_cmbSendStopbits);
    DDX_Control(pDX, IDC_CMB_RECV_PORT, m_cmbRecvPort);
    DDX_Control(pDX, IDC_CMB_RECV_BUAD, m_cmbRecvBaud);
    DDX_Control(pDX, IDC_CMB_RECV_DATABITS, m_cmbRecvDatabits);
    DDX_Control(pDX, IDC_CMB_RECV_CHECKBITS, m_cmbRecvCheckbits);
    DDX_Control(pDX, IDC_CMB_RECV_TOPBITS, m_cmbRecvStopbits);
    DDX_Control(pDX, IDC_EDIT_CMD, m_editCmd);
    DDX_Control(pDX, IDC_EDIT_SEND, m_editSendShow);
    DDX_Control(pDX, IDC_EDIT_RECEIVE, m_editRecvShow);
}

BEGIN_MESSAGE_MAP(CLZSerialPortToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_MFCBUTTON_SEND, &CLZSerialPortToolDlg::OnBnClickedMfcbuttonSend)
    ON_BN_CLICKED(IDC_MFCBUTTON_OPEN, &CLZSerialPortToolDlg::OnBnClickedMfcbuttonOpen)
    ON_BN_CLICKED(IDC_MFCBUTTON_CLOSE, &CLZSerialPortToolDlg::OnBnClickedMfcbuttonClose)
    ON_BN_CLICKED(IDC_MFCBUTTON_CLEAR, &CLZSerialPortToolDlg::OnBnClickedMfcbuttonClear)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_MFCBUTTON_FRESH, &CLZSerialPortToolDlg::OnBnClickedMfcbuttonFresh)
    ON_BN_CLICKED(IDC_MFCBTNCTL_LOG, &CLZSerialPortToolDlg::OnBnClickedMfcbtnctlLog)
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CLZSerialPortToolDlg ��Ϣ�������

BOOL CLZSerialPortToolDlg::OnInitDialog()
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
    // ��ʼ�������б�
    GetSerialPortList();
    if (m_cmbSendPort.GetCount() >= 1)
    {
        m_cmbSendPort.SetCurSel(0);
    }
    if (m_cmbRecvPort.GetCount() >= 1)
    {
        m_cmbRecvPort.SetCurSel(0);
    }
    
    // ��ʼ������ؼ�
    InitControle();

    //������շ��Ϳ�ͽ��տ����ݵĶ�ʱ��, ��������һ��
    SetTimer(CLEAR_TIME,1000*60,0);
    
    LOG.SetPreName("lzSerialPort");
    LOG.SetLogLevel(LOG_DEBUG|LOG_INFO|LOG_ERR|LOG_WARN);
    LOG.debug_log("===========Ӧ�ó�������===========");

    InitializeCriticalSection(&g_section);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CLZSerialPortToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CLZSerialPortToolDlg::OnPaint()
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
HCURSOR CLZSerialPortToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// ��ʾ������Ϣ
void CLZSerialPortToolDlg::GetErrorDesc()
{
    LPVOID lpMsgBuf =0;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL
        );
    MessageBox((LPCTSTR)lpMsgBuf, _T("����"), MB_OK|MB_ICONERROR);
}

// ��ȡ��ǰ�����ϵĴ����б�
void CLZSerialPortToolDlg::GetSerialPortList()
{
    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA deviceInfoData;
    DWORD deviceIndex;

    hDevInfo = SetupDiGetClassDevs((LPGUID)&GUID_DEVCLASS_PORTS, 0, 0, DIGCF_PRESENT);
    if (hDevInfo == INVALID_HANDLE_VALUE)
    {
        for (int i = 0; i < 10; i++)
        {
            CString strPortName;
            strPortName.Format("COM%d", i);
            m_cmbSendPort.AddString(strPortName);
            m_cmbRecvPort.AddString(strPortName);
        }
        
        GetErrorDesc();
        return;
    }

    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    for (deviceIndex = 0; SetupDiEnumDeviceInfo(hDevInfo, deviceIndex, &deviceInfoData); ++deviceIndex)
    {
        DWORD dataPropertyType;
        LPTSTR buffer = NULL;
        DWORD buffersize = 0;

        while (!SetupDiGetDeviceRegistryProperty(
            hDevInfo,
            &deviceInfoData,
            SPDRP_FRIENDLYNAME,
            &dataPropertyType,
            (PBYTE)buffer,
            buffersize,
            &buffersize))
        {
            DWORD ErrorCode=GetLastError();
            if (ErrorCode== ERROR_INSUFFICIENT_BUFFER)
            {
                if (buffer) LocalFree(buffer);
                buffer = (LPTSTR)LocalAlloc(LPTR,buffersize);
            }
            else
            {
                break;
            }
        }
        CString FriendlyPortName=buffer;   /*Extract Serial Port Name*/
        int pos = FriendlyPortName.ReverseFind('(');
        CString SerialPortName=FriendlyPortName.Right(FriendlyPortName.GetLength() - pos -1);
        SerialPortName.TrimRight(')');
        m_cmbSendPort.AddString(SerialPortName);
        m_cmbRecvPort.AddString(SerialPortName);
        if(buffer) 
        {
            LocalFree(buffer);
        } 
    }
    if (GetLastError()!=NO_ERROR
        &&GetLastError()!=ERROR_NO_MORE_ITEMS )
    {
        return;
    }
    // Cleanup
    SetupDiDestroyDeviceInfoList(hDevInfo);
    return;
}

// ��ʼ������ؼ�
void CLZSerialPortToolDlg::InitControle()
{
    // ������
    m_cmbSendBaud.AddString("600");
    m_cmbSendBaud.AddString("1200");
    m_cmbSendBaud.AddString("2400");
    m_cmbSendBaud.AddString("4800");
    m_cmbSendBaud.AddString("9600");
    m_cmbSendBaud.AddString("14400");
    m_cmbSendBaud.AddString("19200");
    m_cmbSendBaud.AddString("38400");
    m_cmbSendBaud.AddString("57600");
    m_cmbSendBaud.AddString("115200");
    m_cmbSendBaud.AddString("57600");
    m_cmbSendBaud.SetCurSel(3);

    m_cmbRecvBaud.AddString("600");
    m_cmbRecvBaud.AddString("1200");
    m_cmbRecvBaud.AddString("2400");
    m_cmbRecvBaud.AddString("4800");
    m_cmbRecvBaud.AddString("9600");
    m_cmbRecvBaud.AddString("14400");
    m_cmbRecvBaud.AddString("19200");
    m_cmbRecvBaud.AddString("38400");
    m_cmbRecvBaud.AddString("57600");
    m_cmbRecvBaud.AddString("115200");
    m_cmbRecvBaud.AddString("57600");
    m_cmbRecvBaud.SetCurSel(3);

    m_mapBaud["600"] = 600;
    m_mapBaud["1200"] = 1200;
    m_mapBaud["2400"] = 2400;
    m_mapBaud["4800"] = 4800;
    m_mapBaud["9600"] = 9600;
    m_mapBaud["14400"] = 14400;
    m_mapBaud["19200"] = 19200;
    m_mapBaud["38400"] = 38400;
    m_mapBaud["57600"] = 57600;
    m_mapBaud["115200"] = 115200;
    m_mapBaud["57600"] = 57600;


    // ��żУ��
    m_cmbSendCheckbits.AddString(_T("��У��"));
    m_cmbSendCheckbits.AddString(_T("��У��"));
    m_cmbSendCheckbits.AddString(_T("żУ��"));
    m_cmbSendCheckbits.AddString(_T("Mark"));
    m_cmbSendCheckbits.AddString(_T("Space"));
    m_cmbSendCheckbits.SetCurSel(2);

    m_cmbRecvCheckbits.AddString(_T("��У��"));
    m_cmbRecvCheckbits.AddString(_T("��У��"));
    m_cmbRecvCheckbits.AddString(_T("żУ��"));
    m_cmbRecvCheckbits.AddString(_T("Mark"));
    m_cmbRecvCheckbits.AddString(_T("Space"));
    m_cmbRecvCheckbits.SetCurSel(2);
    m_mapCheck["��У��"] = NOPARITY;
    m_mapCheck["��У��"] = ODDPARITY;
    m_mapCheck["żУ��"] = EVENPARITY;
    m_mapCheck["Mark"] = MARKPARITY;
    m_mapCheck["Space"] = SPACEPARITY;

    // ����λ
    m_cmbSendDatabits.AddString("4");
    m_cmbSendDatabits.AddString("5");
    m_cmbSendDatabits.AddString("6");
    m_cmbSendDatabits.AddString("7");
    m_cmbSendDatabits.AddString("8");
    m_cmbSendDatabits.SetCurSel(4);

    m_cmbRecvDatabits.AddString("4");
    m_cmbRecvDatabits.AddString("5");
    m_cmbRecvDatabits.AddString("6");
    m_cmbRecvDatabits.AddString("7");
    m_cmbRecvDatabits.AddString("8");
    m_cmbRecvDatabits.SetCurSel(4);
    m_mapDataBits["4"] = 4;
    m_mapDataBits["5"] = 5;
    m_mapDataBits["6"] = 6;
    m_mapDataBits["7"] = 7;
    m_mapDataBits["8"] = 8;

    // ֹͣλ
    m_cmbSendStopbits.AddString("1");
    m_cmbSendStopbits.AddString("1.5");
    m_cmbSendStopbits.AddString("2");
    m_cmbSendStopbits.SetCurSel(0);

    m_cmbRecvStopbits.AddString("1");
    m_cmbRecvStopbits.AddString("1.5");
    m_cmbRecvStopbits.AddString("2");
    m_cmbRecvStopbits.SetCurSel(0);
    m_mapStopbits["1"] = ONESTOPBIT;
    m_mapStopbits["1.5"] = ONE5STOPBITS;
    m_mapStopbits["2"] = TWOSTOPBITS;

    // �������������
    // �ۻ�����
    m_mapCmdDesc["02 43 52 03 12"] = "CR [״̬Ҫ��]ָ���Ӧ��";
    m_mapCmdDesc["02 43 45 03 05"] = "CE ���ų�ָ��";
    m_mapCmdDesc["02 53 4C 03 1C"] = "slѡ����Ʒ";
    m_mapCmdDesc["02 43 57 20 00 00 31 35 03 33 "] = "CW ������ָ��";
    m_mapCmdDesc["02 4E 53 03 1E "] = "NS ��Ʒδѡ��ָ���Ӧ��";

    //չʾ��
    m_mapCmdDesc["02 63 72 00 40 00 00 10 00 00 30 03 72"] = "CR��ʾ��� 1Ԫ";
    m_mapCmdDesc["02 43 45 30 03 35"] = "CE ���ų�ָ���Ӧ��"; 
    m_mapCmdDesc["02 63 72 00 40 00 00 20 00 00 30 03 42"] = "CR��ʾ��� 2Ԫ";
    m_mapCmdDesc["02 43 57 30 03 27"] = "CW ������ָ���Ӧ��";
    m_mapCmdDesc["02 53 4C 30 03 2C"] = "slѡ����Ʒ ";
    m_mapCmdDesc["02 63 72 00 40 00 00 00 00 00 30 03 62"] = "CR��ʾ��� 0Ԫ";
    m_mapCmdDesc["02 43 52 00 00 00 00 30 03 22"] = "CR�޿�����";
    m_mapCmdDesc["02 4E 53 30 03 2E"] = "NS ��Ʒδѡ��ָ���Ӧ��";
    m_mapCmdDesc["02 4C 52 03 1D"] = "LR �汾ȷ��";
    m_mapCmdDesc["02 50 44 03 17"] = "PD ��λȷ��ָ���Ӧ��";
    m_mapCmdDesc["02 43 50 03 10"] = "CP �����տ�ָ���Ӧ��";
    m_mapCmdDesc["02 4E 53 03 1E"] = "NS ��Ʒδѡ��ָ���Ӧ��";
    m_mapCmdDesc["02 43 52 03 12"] = "CR [״̬Ҫ��]ָ���Ӧ��";
    m_mapCmdDesc["02 43 52 30 30 30 30 30 03"] = "CR00000  �޿�����";
    m_mapCmdDesc["02 63 72 30 30 34 30 56 34 00 30 03"] = "CR ��ʾ���";
    //m_mapCmdDesc["02 53 4C 03 1C"] = "SL ��Ʒѡ����ָ���Ӧ��";
    m_mapCmdDesc["02 43 57 "] = "CW ������ָ���Ӧ��";
    m_mapCmdDesc["02 43 45 03 05 "] = "CE ���ų�ָ���Ӧ��";
    m_mapCmdDesc["02 50 44 03 17 "] = "PD ��λȷ��ָ���Ӧ��";
    m_mapCmdDesc["02 43 50 03 10 "] = "CP �����տ�ָ���Ӧ��";
    m_mapCmdDesc["02 4E 53 03 1E "] = "NS ��Ʒδѡ��ָ���Ӧ��";
    m_mapCmdDesc["02 43 52 03 12 "] = "CR [״̬Ҫ��]ָ���Ӧ��";
    m_mapCmdDesc["02 53 4C 03 1C "] = "SL ��Ʒѡ����ָ���Ӧ��";
    m_mapCmdDesc["02 43 57 "] = "CW ������ָ���Ӧ��";

    m_mapCmdDesc["30 31 32 "] = "��������";
}

// �ԶԻ���ķ�ʽ��ʾ��ʾ��Ϣ
void CLZSerialPortToolDlg::ShowTipMessage(const CString& tip)
{
    MessageBox(tip, _T("��ʾ��Ϣ"), MB_OK|MB_ICONEXCLAMATION);
    
    return;
}

// ��ʾ���Ͷ˿ڴ��ڵ���Ϣ
void CLZSerialPortToolDlg::ShowSendMessage(CString msg, int srflag)
{
    CString strOldContext;
    m_editSendShow.GetWindowText(strOldContext);

    // ���յ�������תΪʮ������
    CString strNewContext = ""; 
    const char *p = (LPCSTR)msg;
    int dwbytes = msg.GetLength();
    for (int i = 0; i < dwbytes; i++)
    {
        char tempbuf[32] = {0};
        sprintf(tempbuf, "%02X ", p[i]);
        strNewContext += tempbuf;
    }

    CString strCmdDesc = "";
    std::map<CString, CString>::iterator iter = m_mapCmdDesc.find(strNewContext);
    if (iter != m_mapCmdDesc.end())
    {
        strCmdDesc = iter->second;
    }
    strCmdDesc.Format("%-20s", strCmdDesc);
    if (SEND_MSG == srflag)
    {
        LOG.debug_log("%s��@@@@@@@@@@@��:%s        %s", (LPCSTR)m_sendPort.GetPortName(),
            (LPCSTR)strNewContext, (LPCSTR)strCmdDesc);
        strNewContext =  "����: " + strCmdDesc + strNewContext;
    }
    else if (RECV_MSG == srflag)
    {
        LOG.debug_log("%s��##########��:%s        %s", (LPCSTR)m_sendPort.GetPortName(),
            (LPCSTR)strNewContext, (LPCSTR)strCmdDesc);
        strNewContext =  "����: " + strCmdDesc + strNewContext;
    }
    else
    {
        return;
    }

    strNewContext = strNewContext +  "\r\n" + strOldContext;
    m_editSendShow.SetWindowText(strNewContext);
}

// ��ʾ���ն˿ڴ��ڵ���Ϣ
void CLZSerialPortToolDlg::ShowRecvMessage(CString msg, int srflag)
{
    CString strOldContext;
    m_editRecvShow.GetWindowText(strOldContext);

    // ���յ�������תΪʮ������
    CString strNewContext = ""; 
    const char *p = (LPCSTR)msg;
    int dwbytes = msg.GetLength();
    for (int i = 0; i < dwbytes; i++)
    {
        char tempbuf[32] = {0};
        sprintf(tempbuf, "%02X ", p[i]);
        strNewContext += tempbuf;
    }

    CString strCmdDesc = "";
    std::map<CString, CString>::iterator iter = m_mapCmdDesc.find(strNewContext);
    if (iter != m_mapCmdDesc.end())
    {
        strCmdDesc = iter->second;
    }
    strCmdDesc.Format("%-20s", strCmdDesc);
    if (SEND_MSG == srflag)
    {
        LOG.debug_log("%s��----------��:%s", (LPCSTR)m_recvPort.GetPortName(),
            (LPCSTR)strNewContext);
        strNewContext =  "����: " + strCmdDesc + strNewContext;
    }
    else if (RECV_MSG == srflag)
    {
        LOG.debug_log("%s��##########��:%s", (LPCSTR)m_recvPort.GetPortName(),
            (LPCSTR)strNewContext);
        strNewContext =  "����: " + strCmdDesc + strNewContext;
    }
    else
    {
        return;
    }

    strNewContext = strNewContext + "\r\n" + strOldContext;
    m_editRecvShow.SetWindowText(strNewContext);
}

void CLZSerialPortToolDlg::OnBnClickedMfcbuttonSend()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    CString strCmd;
   
    if (false == m_sendPort.IsOpen())
    {
        SHOWTIP("����δ��!");
        return;
    }

    m_editCmd.GetWindowText(strCmd);
    g_cmd = strCmd;

    if (false == g_startWork)
    {
        g_startWork = true;
    }
}


void CLZSerialPortToolDlg::OnBnClickedMfcbuttonOpen()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    /*********************** �򿪴��� *******************/
    if(true == m_sendPort.IsOpen() || true == m_recvPort.IsOpen())
    {
        SHOWTIP("���ڴ��ڴ�״̬�������ظ���");
        return;
    }

    CString strSendPortName;
    m_cmbSendPort.GetWindowText(strSendPortName);
    if (0 >= strSendPortName.GetLength())
    {
        ShowTipMessage("��ѡ���Ͷ˿�");
        m_cmbSendPort.SetFocus();
        return;
    }
    m_strSendPortName = strSendPortName;

    CString strRecvPortName;
    m_cmbRecvPort.GetWindowText(strRecvPortName);
    if (0 >= strRecvPortName.GetLength())
    {
        ShowTipMessage("��ѡ����ն˿�");
        m_cmbRecvPort.SetFocus();
        return;
    }
    m_strRecvPortName = strRecvPortName;

    // ������
    CString strSendBaud;
    m_cmbSendBaud.GetWindowText(strSendBaud);
    DWORD dwSendBaud = m_mapBaud[strSendBaud];
    CString strRecvBaud;
    m_cmbRecvBaud.GetWindowText(strRecvBaud);
    DWORD dwRecvBaud = m_mapBaud[strRecvBaud];

    // У��λ
    CString strSendParity;
    m_cmbSendCheckbits.GetWindowText(strSendParity);
    int nSendParity = m_mapCheck[strSendParity];
    CString strRecvParity;
    m_cmbRecvCheckbits.GetWindowText(strRecvParity);
    int nRecvParity = m_mapCheck[strRecvParity];

    // ����λ
    CString strSendDatabits;
    m_cmbSendDatabits.GetWindowText(strSendDatabits);
    int nSendDatabits = m_mapDataBits[strSendDatabits];
    CString strRecvDatabits;
    m_cmbRecvDatabits.GetWindowText(strRecvDatabits);
    int nRecvDatabits = m_mapDataBits[strRecvDatabits];

    // ֹͣλ
    CString strSendStopbits;
    m_cmbSendStopbits.GetWindowText(strSendStopbits);
    int nSendStopbits = m_mapStopbits[strSendStopbits];
    CString strRecvStopbits;
    m_cmbRecvDatabits.GetWindowText(strRecvStopbits);
    int nRecvStopbits = m_mapStopbits[strRecvStopbits];

    // ���Ͷ˿ںͽ��ն˿���ͬ����ֻ�򿪷��Ͷ˿�
    g_isruning = true;
    if(strRecvPortName == strSendPortName)
    {
        bool bret = m_sendPort.InitSerialParam(strSendPortName, dwSendBaud, 
            nSendParity, nSendDatabits, nSendStopbits);
        if (false == bret)
        {
            MessageBox(_T("���÷��Ͷ˴��ڲ���ʧ��!"));
            return;
        }

        m_sendHandle = m_sendPort.OpenPort();
        if (INVALID_HANDLE_VALUE == m_sendHandle)
        {
            return;
        }

        // ����Ŀ�Ĵ���Ϊ�Լ�
        m_sendPort.SetPeerPort(m_sendHandle);

        // ���������߳�
        StartSendThread();
    }
    else
    {
        bool bret = m_sendPort.InitSerialParam(strSendPortName, dwSendBaud, 
            nSendParity, nSendDatabits, nSendStopbits);
        if (false == bret)
        {
            MessageBox(_T("���÷��Ͷ˴��ڲ���ʧ��!"));
            return;
        }
        bret = m_recvPort.InitSerialParam(strRecvPortName, dwRecvBaud, 
            nRecvParity, nRecvDatabits, nRecvStopbits);

        if (false == bret)
        {
            MessageBox(_T("���ý��ն˴��ڲ���ʧ��!"));
        }

        m_sendHandle = m_sendPort.OpenPort();
        if (INVALID_HANDLE_VALUE == m_sendHandle)
        {
            MessageBox(_T("�򿪷��Ͷ˿�ʧ��!"));
            return;
        }

        m_recvHandle = m_recvPort.OpenPort();
        if (INVALID_HANDLE_VALUE == m_recvHandle)
        {
            MessageBox(_T("�򿪽��ն˿�ʧ��!"));
            return;
        }

        m_sendPort.SetPeerPort(m_recvHandle);
        m_recvPort.SetPeerPort(m_sendHandle);

        // ���������߳�
        if (false == StartSendThread())
        {
            OnBnClickedMfcbuttonClose();
            SHOWTIP("�������Ͷ˿��߳�ʧ��!");
            return;
        }
        Sleep(10);

        if (false == StartRecvThread())
        {
            OnBnClickedMfcbuttonClose();
            SHOWTIP("�������ն˿��߳�ʧ��!");
            return;
        }
    }

    if (false == g_startWork)
    {
        g_startWork = true;
    }
}


void CLZSerialPortToolDlg::OnBnClickedMfcbuttonClose()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    // �رմ����߳�
    if (m_strRecvPortName == m_strSendPortName)
    {
        m_sendPort.SetClosePort();
    }
    else
    {
        m_sendPort.SetClosePort();
        m_recvPort.SetClosePort();
    }

    g_isruning = false;
    g_startWork = false;
}


void CLZSerialPortToolDlg::OnBnClickedMfcbuttonClear()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    m_editSendShow.SetWindowText("");
    m_editRecvShow.SetWindowText("");
}

// �������Ͷ˿��߳�
bool CLZSerialPortToolDlg::StartSendThread()
{
    m_sendThread = AfxBeginThread(SendPortThreadFunc,
        &m_sendPort,
        THREAD_PRIORITY_NORMAL,
        0,
        0,
        NULL);

    if (NULL == m_sendThread)
    {
        SHOWTIP(" �������Ͷ˿��߳�ʧ��");
        return false;
    }

    return true;
}

// �������ն˿��߳�
bool CLZSerialPortToolDlg::StartRecvThread()
{
    m_recvThread = AfxBeginThread(RecvPortThreadFunc,
        &m_recvPort,
        THREAD_PRIORITY_NORMAL,
        0,
        0,
        NULL);

    if(NULL == m_recvThread)
    {
        SHOWTIP(" �������Ͷ˿��߳�ʧ��");
        return false;
    }

    return true;
}

//void CAboutDlg::OnClose()
//{
//    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
//    CDialogEx::OnClose();
//}


void CLZSerialPortToolDlg::OnClose()
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
    OnBnClickedMfcbuttonClose();
    CDialogEx::OnClose();
}

void CLZSerialPortToolDlg::OnBnClickedMfcbuttonFresh()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    m_cmbSendPort.ResetContent();
    m_cmbRecvPort.ResetContent();
    GetSerialPortList();

    if (m_cmbSendPort.GetCount() >= 1)
    {
        m_cmbSendPort.SetCurSel(0);
    }
    if (m_cmbRecvPort.GetCount() >= 1)
    {
        m_cmbRecvPort.SetCurSel(0);
    }
}


void CLZSerialPortToolDlg::OnBnClickedMfcbtnctlLog()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    static bool bFlag = true;
    if (bFlag)
    {
        LOG.SetLogLevel(LOG_NOLOG);
        ((CMFCButton *)GetDlgItem(IDC_MFCBTNCTL_LOG))->SetWindowText("����־");
        bFlag = !bFlag;
    }
    else
    {
        LOG.SetLogLevel(LOG_DEBUG|LOG_INFO|LOG_WARN|LOG_ERR);
        ((CMFCButton *)GetDlgItem(IDC_MFCBTNCTL_LOG))->SetWindowText("�ر���־");
        bFlag = !bFlag;
    }
}


void CLZSerialPortToolDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
    m_editSendShow.SetWindowText("");
    m_editRecvShow.SetWindowText("");
    CDialogEx::OnTimer(nIDEvent);
}

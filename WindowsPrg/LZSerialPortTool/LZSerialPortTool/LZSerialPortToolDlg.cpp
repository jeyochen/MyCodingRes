
// LZSerialPortToolDlg.cpp : 实现文件
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

bool g_isruning = false; // 线程是否继续运行 true 继续运行，false 停止运行
bool g_startWork = false; // 串口是否开始工作，true开始工作， false 不工作
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

    LOG.debug_log("串口[%s]线程启动", (LPCSTR)pSerialPort->GetPortName(),
        (LPCSTR)g_cmd);
    while(g_isruning)
    {
        if (!g_startWork)
        {
            Sleep(10);
            continue;
        }

        CString cmd;

        // 进入发送临界区, 发送第一个命令
        EnterCriticalSection(&g_section);
        if(g_cmd.GetLength() > 0)
        {
            //srand(time(NULL));
            //int n = rand()%10000;
            //CString strRand;
            //strRand.Format("%d", n);
            //g_cmd += strRand;
            // 发送数据到目的串口
            memset(sSendBuf, 0, sizeof(sSendBuf));
            memcpy(sSendBuf, (LPCSTR)g_cmd, g_cmd.GetLength());

            // 表明正常通信的过程中，用户手工发送了新命令
            if (false == b_isfirst)
            {
                b_newcmd = true;
            }
            else
            {
                // 第一次通信，怎发送命令
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

        // 进入接收发送临界区
        EnterCriticalSection(&g_section);
        // 从自己的串口接收数据
        memset(sRecvBuf, 0, sizeof(sRecvBuf));
        dwRecvBytes = pSerialPort->ReadData(sRecvBuf, BUF_SIZE /*g_send_len*/);
        if (dwRecvBytes > 0)
        {
            CString strRecv(sRecvBuf);
            //SHOWSEND(strRecv, RECV_MSG);
        }
        else if(-1 == dwRecvBytes)
        {
            LOG.err_log("串口[%s]接收数据异常", 
                (LPCSTR)pSerialPort->GetPortName());
        }

        // 把收到的数据发送给对端
        if (dwRecvBytes > 0)
        {
            // 发送用户手工下发的新命令
            if (b_newcmd)
            {
                memset(sRecvBuf, 0, sizeof(sRecvBuf));
                memcpy(sRecvBuf, (LPCSTR)g_cmd, g_cmd.GetLength());
                dwRecvBytes = g_cmd.GetLength();
                g_cmd = "";
                b_newcmd = false;
            }

            // 发送接收到的数据给目的串口
            dwSenbytes = pSerialPort->WriteData(sRecvBuf, dwRecvBytes);
            if (dwSenbytes > 0)
            {
                CString strSend(sRecvBuf);
                SHOWSEND(strSend, SEND_MSG);
            }
            else if (-1 == dwSenbytes)
            {
                LOG.err_log("串口[%s]发送数据异常", 
                    (LPCSTR)pSerialPort->GetPortName());
            }
        }
        LeaveCriticalSection(&g_section);
        
        if (-1 == dwRecvBytes)
        {
            // 退出线程
            LOG.warn_log("发送线程异常退出");
            break;
        }
    }

    LOG.debug_log("串口[%s]线程正常退出", (LPCSTR)pSerialPort->GetPortName(),
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

    LOG.debug_log("串口[%s]线程启动", (LPCSTR)pSerialPort->GetPortName(),
        (LPCSTR)g_cmd);
    while(g_isruning)
    {
        if (!g_startWork)
        {
            Sleep(10);
            continue;
        }

        // 从自己的串口接收数据
        //g_recvSection.Lock();
        // 进入接收和发送临界区
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
            LOG.err_log("串口[%s]接收数据异常", 
                (LPCSTR)pSerialPort->GetPortName());
        }

        // 把接收到的数据在发送出
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
            // 退出线程
            LOG.err_log("串口[%s]发送数据异常", 
                (LPCSTR)pSerialPort->GetPortName());
        }
        LeaveCriticalSection(&g_section);
        Sleep(10);
    }

    LOG.debug_log("串口[%s]线程正常退出", (LPCSTR)pSerialPort->GetPortName(),
        (LPCSTR)g_cmd);
    return 0;
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CLZSerialPortToolDlg 对话框




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


// CLZSerialPortToolDlg 消息处理程序

BOOL CLZSerialPortToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
    // 初始化串口列表
    GetSerialPortList();
    if (m_cmbSendPort.GetCount() >= 1)
    {
        m_cmbSendPort.SetCurSel(0);
    }
    if (m_cmbRecvPort.GetCount() >= 1)
    {
        m_cmbRecvPort.SetCurSel(0);
    }
    
    // 初始化界面控件
    InitControle();

    //设置清空发送框和接收框内容的定时器, 五分钟清除一次
    SetTimer(CLEAR_TIME,1000*60,0);
    
    LOG.SetPreName("lzSerialPort");
    LOG.SetLogLevel(LOG_DEBUG|LOG_INFO|LOG_ERR|LOG_WARN);
    LOG.debug_log("===========应用程序启动===========");

    InitializeCriticalSection(&g_section);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CLZSerialPortToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CLZSerialPortToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 显示错误信息
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
    MessageBox((LPCTSTR)lpMsgBuf, _T("错误"), MB_OK|MB_ICONERROR);
}

// 获取当前机器上的串口列表
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

// 初始化界面控件
void CLZSerialPortToolDlg::InitControle()
{
    // 波特率
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


    // 奇偶校验
    m_cmbSendCheckbits.AddString(_T("无校验"));
    m_cmbSendCheckbits.AddString(_T("奇校验"));
    m_cmbSendCheckbits.AddString(_T("偶校验"));
    m_cmbSendCheckbits.AddString(_T("Mark"));
    m_cmbSendCheckbits.AddString(_T("Space"));
    m_cmbSendCheckbits.SetCurSel(2);

    m_cmbRecvCheckbits.AddString(_T("无校验"));
    m_cmbRecvCheckbits.AddString(_T("奇校验"));
    m_cmbRecvCheckbits.AddString(_T("偶校验"));
    m_cmbRecvCheckbits.AddString(_T("Mark"));
    m_cmbRecvCheckbits.AddString(_T("Space"));
    m_cmbRecvCheckbits.SetCurSel(2);
    m_mapCheck["无校验"] = NOPARITY;
    m_mapCheck["奇校验"] = ODDPARITY;
    m_mapCheck["偶校验"] = EVENPARITY;
    m_mapCheck["Mark"] = MARKPARITY;
    m_mapCheck["Space"] = SPACEPARITY;

    // 数据位
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

    // 停止位
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

    // 命令和命令描述
    // 售货机发
    m_mapCmdDesc["02 43 52 03 12"] = "CR [状态要求]指令及其应答";
    m_mapCmdDesc["02 43 45 03 05"] = "CE 卡排出指令";
    m_mapCmdDesc["02 53 4C 03 1C"] = "sl选中商品";
    m_mapCmdDesc["02 43 57 20 00 00 31 35 03 33 "] = "CW 卡更新指令";
    m_mapCmdDesc["02 4E 53 03 1E "] = "NS 商品未选择指令及其应答";

    //展示柜发
    m_mapCmdDesc["02 63 72 00 40 00 00 10 00 00 30 03 72"] = "CR显示余额 1元";
    m_mapCmdDesc["02 43 45 30 03 35"] = "CE 卡排出指令及其应答"; 
    m_mapCmdDesc["02 63 72 00 40 00 00 20 00 00 30 03 42"] = "CR显示余额 2元";
    m_mapCmdDesc["02 43 57 30 03 27"] = "CW 卡更新指令及其应答";
    m_mapCmdDesc["02 53 4C 30 03 2C"] = "sl选中商品 ";
    m_mapCmdDesc["02 63 72 00 40 00 00 00 00 00 30 03 62"] = "CR显示余额 0元";
    m_mapCmdDesc["02 43 52 00 00 00 00 30 03 22"] = "CR无卡读入";
    m_mapCmdDesc["02 4E 53 30 03 2E"] = "NS 商品未选择指令及其应答";
    m_mapCmdDesc["02 4C 52 03 1D"] = "LR 版本确认";
    m_mapCmdDesc["02 50 44 03 17"] = "PD 复位确认指令及其应答";
    m_mapCmdDesc["02 43 50 03 10"] = "CP 卡接收可指令及其应答";
    m_mapCmdDesc["02 4E 53 03 1E"] = "NS 商品未选择指令及其应答";
    m_mapCmdDesc["02 43 52 03 12"] = "CR [状态要求]指令及其应答";
    m_mapCmdDesc["02 43 52 30 30 30 30 30 03"] = "CR00000  无卡读入";
    m_mapCmdDesc["02 63 72 30 30 34 30 56 34 00 30 03"] = "CR 显示余额";
    //m_mapCmdDesc["02 53 4C 03 1C"] = "SL 商品选择中指令及其应答";
    m_mapCmdDesc["02 43 57 "] = "CW 卡更新指令及其应答";
    m_mapCmdDesc["02 43 45 03 05 "] = "CE 卡排出指令及其应答";
    m_mapCmdDesc["02 50 44 03 17 "] = "PD 复位确认指令及其应答";
    m_mapCmdDesc["02 43 50 03 10 "] = "CP 卡接收可指令及其应答";
    m_mapCmdDesc["02 4E 53 03 1E "] = "NS 商品未选择指令及其应答";
    m_mapCmdDesc["02 43 52 03 12 "] = "CR [状态要求]指令及其应答";
    m_mapCmdDesc["02 53 4C 03 1C "] = "SL 商品选择中指令及其应答";
    m_mapCmdDesc["02 43 57 "] = "CW 卡更新指令及其应答";

    m_mapCmdDesc["30 31 32 "] = "测试数据";
}

// 以对话框的方式显示提示信息
void CLZSerialPortToolDlg::ShowTipMessage(const CString& tip)
{
    MessageBox(tip, _T("提示信息"), MB_OK|MB_ICONEXCLAMATION);
    
    return;
}

// 显示发送端口窗口的信息
void CLZSerialPortToolDlg::ShowSendMessage(CString msg, int srflag)
{
    CString strOldContext;
    m_editSendShow.GetWindowText(strOldContext);

    // 把收到的数据转为十六进制
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
        LOG.debug_log("%s发@@@@@@@@@@@》:%s        %s", (LPCSTR)m_sendPort.GetPortName(),
            (LPCSTR)strNewContext, (LPCSTR)strCmdDesc);
        strNewContext =  "发送: " + strCmdDesc + strNewContext;
    }
    else if (RECV_MSG == srflag)
    {
        LOG.debug_log("%s收##########》:%s        %s", (LPCSTR)m_sendPort.GetPortName(),
            (LPCSTR)strNewContext, (LPCSTR)strCmdDesc);
        strNewContext =  "接收: " + strCmdDesc + strNewContext;
    }
    else
    {
        return;
    }

    strNewContext = strNewContext +  "\r\n" + strOldContext;
    m_editSendShow.SetWindowText(strNewContext);
}

// 显示接收端口窗口的信息
void CLZSerialPortToolDlg::ShowRecvMessage(CString msg, int srflag)
{
    CString strOldContext;
    m_editRecvShow.GetWindowText(strOldContext);

    // 把收到的数据转为十六进制
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
        LOG.debug_log("%s发----------》:%s", (LPCSTR)m_recvPort.GetPortName(),
            (LPCSTR)strNewContext);
        strNewContext =  "发送: " + strCmdDesc + strNewContext;
    }
    else if (RECV_MSG == srflag)
    {
        LOG.debug_log("%s收##########》:%s", (LPCSTR)m_recvPort.GetPortName(),
            (LPCSTR)strNewContext);
        strNewContext =  "接收: " + strCmdDesc + strNewContext;
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
    // TODO: 在此添加控件通知处理程序代码
    CString strCmd;
   
    if (false == m_sendPort.IsOpen())
    {
        SHOWTIP("串口未打开!");
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
    // TODO: 在此添加控件通知处理程序代码
    /*********************** 打开串口 *******************/
    if(true == m_sendPort.IsOpen() || true == m_recvPort.IsOpen())
    {
        SHOWTIP("串口处于打开状态，无需重复打开");
        return;
    }

    CString strSendPortName;
    m_cmbSendPort.GetWindowText(strSendPortName);
    if (0 >= strSendPortName.GetLength())
    {
        ShowTipMessage("请选择发送端口");
        m_cmbSendPort.SetFocus();
        return;
    }
    m_strSendPortName = strSendPortName;

    CString strRecvPortName;
    m_cmbRecvPort.GetWindowText(strRecvPortName);
    if (0 >= strRecvPortName.GetLength())
    {
        ShowTipMessage("请选择接收端口");
        m_cmbRecvPort.SetFocus();
        return;
    }
    m_strRecvPortName = strRecvPortName;

    // 波特率
    CString strSendBaud;
    m_cmbSendBaud.GetWindowText(strSendBaud);
    DWORD dwSendBaud = m_mapBaud[strSendBaud];
    CString strRecvBaud;
    m_cmbRecvBaud.GetWindowText(strRecvBaud);
    DWORD dwRecvBaud = m_mapBaud[strRecvBaud];

    // 校验位
    CString strSendParity;
    m_cmbSendCheckbits.GetWindowText(strSendParity);
    int nSendParity = m_mapCheck[strSendParity];
    CString strRecvParity;
    m_cmbRecvCheckbits.GetWindowText(strRecvParity);
    int nRecvParity = m_mapCheck[strRecvParity];

    // 数据位
    CString strSendDatabits;
    m_cmbSendDatabits.GetWindowText(strSendDatabits);
    int nSendDatabits = m_mapDataBits[strSendDatabits];
    CString strRecvDatabits;
    m_cmbRecvDatabits.GetWindowText(strRecvDatabits);
    int nRecvDatabits = m_mapDataBits[strRecvDatabits];

    // 停止位
    CString strSendStopbits;
    m_cmbSendStopbits.GetWindowText(strSendStopbits);
    int nSendStopbits = m_mapStopbits[strSendStopbits];
    CString strRecvStopbits;
    m_cmbRecvDatabits.GetWindowText(strRecvStopbits);
    int nRecvStopbits = m_mapStopbits[strRecvStopbits];

    // 发送端口和接收端口相同，则只打开发送端口
    g_isruning = true;
    if(strRecvPortName == strSendPortName)
    {
        bool bret = m_sendPort.InitSerialParam(strSendPortName, dwSendBaud, 
            nSendParity, nSendDatabits, nSendStopbits);
        if (false == bret)
        {
            MessageBox(_T("设置发送端串口参数失败!"));
            return;
        }

        m_sendHandle = m_sendPort.OpenPort();
        if (INVALID_HANDLE_VALUE == m_sendHandle)
        {
            return;
        }

        // 设置目的串口为自己
        m_sendPort.SetPeerPort(m_sendHandle);

        // 启动串口线程
        StartSendThread();
    }
    else
    {
        bool bret = m_sendPort.InitSerialParam(strSendPortName, dwSendBaud, 
            nSendParity, nSendDatabits, nSendStopbits);
        if (false == bret)
        {
            MessageBox(_T("设置发送端串口参数失败!"));
            return;
        }
        bret = m_recvPort.InitSerialParam(strRecvPortName, dwRecvBaud, 
            nRecvParity, nRecvDatabits, nRecvStopbits);

        if (false == bret)
        {
            MessageBox(_T("设置接收端串口参数失败!"));
        }

        m_sendHandle = m_sendPort.OpenPort();
        if (INVALID_HANDLE_VALUE == m_sendHandle)
        {
            MessageBox(_T("打开发送端口失败!"));
            return;
        }

        m_recvHandle = m_recvPort.OpenPort();
        if (INVALID_HANDLE_VALUE == m_recvHandle)
        {
            MessageBox(_T("打开接收端口失败!"));
            return;
        }

        m_sendPort.SetPeerPort(m_recvHandle);
        m_recvPort.SetPeerPort(m_sendHandle);

        // 启动串口线程
        if (false == StartSendThread())
        {
            OnBnClickedMfcbuttonClose();
            SHOWTIP("启动发送端口线程失败!");
            return;
        }
        Sleep(10);

        if (false == StartRecvThread())
        {
            OnBnClickedMfcbuttonClose();
            SHOWTIP("启动接收端口线程失败!");
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
    // TODO: 在此添加控件通知处理程序代码
    // 关闭串口线程
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
    // TODO: 在此添加控件通知处理程序代码
    m_editSendShow.SetWindowText("");
    m_editRecvShow.SetWindowText("");
}

// 启动发送端口线程
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
        SHOWTIP(" 启动发送端口线程失败");
        return false;
    }

    return true;
}

// 启动接收端口线程
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
        SHOWTIP(" 启动发送端口线程失败");
        return false;
    }

    return true;
}

//void CAboutDlg::OnClose()
//{
//    // TODO: 在此添加消息处理程序代码和/或调用默认值
//    CDialogEx::OnClose();
//}


void CLZSerialPortToolDlg::OnClose()
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    OnBnClickedMfcbuttonClose();
    CDialogEx::OnClose();
}

void CLZSerialPortToolDlg::OnBnClickedMfcbuttonFresh()
{
    // TODO: 在此添加控件通知处理程序代码
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
    // TODO: 在此添加控件通知处理程序代码
    static bool bFlag = true;
    if (bFlag)
    {
        LOG.SetLogLevel(LOG_NOLOG);
        ((CMFCButton *)GetDlgItem(IDC_MFCBTNCTL_LOG))->SetWindowText("打开日志");
        bFlag = !bFlag;
    }
    else
    {
        LOG.SetLogLevel(LOG_DEBUG|LOG_INFO|LOG_WARN|LOG_ERR);
        ((CMFCButton *)GetDlgItem(IDC_MFCBTNCTL_LOG))->SetWindowText("关闭日志");
        bFlag = !bFlag;
    }
}


void CLZSerialPortToolDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    m_editSendShow.SetWindowText("");
    m_editRecvShow.SetWindowText("");
    CDialogEx::OnTimer(nIDEvent);
}

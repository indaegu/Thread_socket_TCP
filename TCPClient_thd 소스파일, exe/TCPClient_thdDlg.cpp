#include "stdafx.h"
#include "TCPClient_thd.h"
#include "TCPClient_thdDlg.h"
#include "afxdialogex.h"
#include "DataSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
CCriticalSection tx_cs;
CCriticalSection rx_cs;


class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);  

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


CTCPClient_thdDlg::CTCPClient_thdDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TCPCLIENT_THD_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTCPClient_thdDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_tx_edit_short);
	DDX_Control(pDX, IDC_EDIT2, m_tx_edit);
	DDX_Control(pDX, IDC_EDIT3, m_rx_edit);
	DDX_Control(pDX, IDC_IPADDRESS1, m_ipaddr);
}

BEGIN_MESSAGE_MAP(CTCPClient_thdDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECT, &CTCPClient_thdDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_DISCONNECT, &CTCPClient_thdDlg::OnBnClickedDisconnect)
	ON_BN_CLICKED(IDC_SEND, &CTCPClient_thdDlg::OnBnClickedSend)
END_MESSAGE_MAP()

UINT RXThread(LPVOID arg)
{
	ThreadArg*pArg = (ThreadArg*)arg;
	CStringList*plist = pArg->pList;
	CTCPClient_thdDlg*pDlg = (CTCPClient_thdDlg*)pArg->pDlg;
	while (pArg->Thread_run) {
		POSITION pos = plist->GetHeadPosition();
		POSITION current_pos;
		while (pos != NULL) {
			current_pos = pos;
			rx_cs.Lock();
			CString str = plist->GetNext(pos);
			rx_cs.Unlock();

			CString message;
			pDlg->m_rx_edit.GetWindowText(message);
			message += str;
			pDlg->m_rx_edit.SetWindowTextW(message);
			pDlg->m_rx_edit.LineScroll(pDlg->m_rx_edit.GetLineCount());

			plist->RemoveAt(current_pos);
		}
		Sleep(10);
	}
	return 0;
}
UINT TXThread(LPVOID arg)
{
	ThreadArg*pArg = (ThreadArg*)arg;
	CStringList*plist = pArg->pList;
	CTCPClient_thdDlg*pDlg = (CTCPClient_thdDlg*)pArg->pDlg;
	while (pArg->Thread_run) {
		POSITION pos = plist->GetHeadPosition();
		POSITION current_pos;
		while (pos != NULL) { 
			current_pos = pos;
			tx_cs.Lock(); // 임계구역 설정해줌
			CString str = plist->GetNext(pos);
			tx_cs.Unlock(); // 임계구역 해제
			CString message;
			message += str;
			pDlg->m_pDataSocket->Send((LPCTSTR)message, (message.GetLength() + 1) * sizeof(TCHAR));

			plist->RemoveAt(current_pos);
		}
		Sleep(10);
	}
	return 0;
}

BOOL CTCPClient_thdDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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

	SetIcon(m_hIcon, TRUE);			
	SetIcon(m_hIcon, FALSE);		

	CStringList*newlist = new CStringList;
	arg1.pList = newlist;
	arg1.Thread_run = 1;
	arg1.pDlg = this;

	CStringList*newlist2 = new CStringList;
	arg2.pList = newlist2;
	arg2.Thread_run = 1;
	arg2.pDlg = this;

	m_pDataSocket = NULL;
	m_ipaddr.SetWindowTextW(_T("127.0.0.1"));

	WSADATA wsa;
	int error_code;

	if ((error_code = WSAStartup(MAKEWORD(2, 2), &wsa)) != 0)
	{
		TCHAR buffer[256];
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, 256, NULL);
		AfxMessageBox(buffer, MB_ICONERROR);
	}

	pThread1 = AfxBeginThread(TXThread, (LPVOID)&arg1);
	pThread2 = AfxBeginThread(RXThread, (LPVOID)&arg2);
	return TRUE;  
}

void CTCPClient_thdDlg::OnSysCommand(UINT nID, LPARAM lParam)
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



void CTCPClient_thdDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); 

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}


HCURSOR CTCPClient_thdDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTCPClient_thdDlg::OnBnClickedConnect()
{
	if (m_pDataSocket == NULL) {
		m_pDataSocket = new CDataSocket(this);
		m_pDataSocket->Create();
		CString addr;
		CString str;
		m_ipaddr.GetWindowText(addr);
		if (m_pDataSocket->Connect(addr, 8000)) {
			MessageBox(_T("서버에 접속 성공!"), _T("알림"), MB_ICONINFORMATION);
			str.Format(_T("### 서버에 접속 성공! ###\r\n"));
			m_tx_edit.SetWindowTextW(str);
			m_tx_edit_short.SetFocus();
		}
		else {
			MessageBox(_T("서버에 접속 실패!"), _T("알림"), MB_ICONERROR);
			delete m_pDataSocket;
			m_pDataSocket = NULL;
		}
	}
	else {
		MessageBox(_T("서버에 이미 접속됨!"), _T("알림"), MB_ICONINFORMATION);
		m_tx_edit_short.SetFocus();
	}
}


void CTCPClient_thdDlg::ProcessReceive(CDataSocket* pSocket, int nErrorCode)
{
	TCHAR pBuf[1024 + 1];
	CString strData;
	int nbytes;

	nbytes = pSocket->Receive(pBuf, 1024);
	pBuf[nbytes] = NULL;
	strData = (LPCTSTR)pBuf;

	rx_cs.Lock();
	arg2.pList->AddTail((LPCTSTR)strData);
	rx_cs.Unlock();
}


void CTCPClient_thdDlg::OnBnClickedDisconnect()
{
	if (m_pDataSocket == NULL) {
		MessageBox(_T("서버에 접속 안 함!"), _T("알림"), MB_ICONERROR);
	}
	else
	{
		arg1.Thread_run = 0;
		arg2.Thread_run = 0;
		m_pDataSocket->Close();
		delete m_pDataSocket;
		m_pDataSocket = NULL;
	}
}


void CTCPClient_thdDlg::OnBnClickedSend()
{
	if (m_pDataSocket == NULL)
	{
		MessageBox(_T("서버에 접속 안 함!"), _T("알림"), MB_ICONERROR);
	}
	else {
		CString tx_message;
		m_tx_edit_short.GetWindowText(tx_message);
		tx_message += _T("\r\n");
		tx_cs.Lock();//임계구역 설정
		arg1.pList->AddTail(tx_message);
		tx_cs.Unlock();// 임계구역 해제
		m_tx_edit_short.SetWindowText(_T(""));
		m_tx_edit_short.SetFocus();

		int len = m_tx_edit.GetWindowTextLengthW();
		m_tx_edit.SetSel(len, len);
		m_tx_edit.ReplaceSel(tx_message);
	}
}
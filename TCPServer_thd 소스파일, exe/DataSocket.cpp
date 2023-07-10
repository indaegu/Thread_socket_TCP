
#include "stdafx.h"
#include "TCPServer_thd.h"
#include "DataSocket.h"
#include "TCPServer_thdDlg.h"



CDataSocket::CDataSocket(CTCPServer_thdDlg *pDlg)
{
	m_pDlg = pDlg;
}

CDataSocket::~CDataSocket()
{
}

void CDataSocket::OnReceive(int nErrorCode)
{
	CSocket::OnReceive(nErrorCode);
	m_pDlg->ProcessReceive(this, nErrorCode);
}

void CDataSocket::OnClose(int nErrorCode)
{
	CSocket::OnClose(nErrorCode);
	m_pDlg->ProcessClose(this, nErrorCode);
}


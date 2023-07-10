#pragma once

class CTCPClient_thdDlg;
class CDataSocket : public CSocket
{
public:
	CDataSocket(CTCPClient_thdDlg*pDlg);
	virtual ~CDataSocket();
	CTCPClient_thdDlg*m_pDlg;
	virtual void OnReceive(int nErrorCode);
};



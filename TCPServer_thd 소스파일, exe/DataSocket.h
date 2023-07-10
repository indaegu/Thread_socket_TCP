#pragma once

// CDataSocket 명령 대상입니다.
class CTCPServer_thdDlg;

class CDataSocket : public CSocket
{
public:
	CDataSocket(CTCPServer_thdDlg *pDlg);
	virtual ~CDataSocket();
	CTCPServer_thdDlg *m_pDlg;
	virtual void OnReceive(int nErrorCode);
	virtual void OnClose(int nErrorCode);

};




// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������ 
// ��� �ִ� ���� �����Դϴ�.

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // �Ϻ� CString �����ڴ� ��������� ����˴ϴ�.

// MFC�� ���� �κа� ���� ������ ��� �޽����� ���� ����⸦ �����մϴ�.
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC �ٽ� �� ǥ�� ���� ����Դϴ�.
#include <afxext.h>         // MFC Ȯ���Դϴ�.





#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // Internet Explorer 4 ���� ��Ʈ�ѿ� ���� MFC �����Դϴ�.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // Windows ���� ��Ʈ�ѿ� ���� MFC �����Դϴ�.
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC�� ���� �� ��Ʈ�� ���� ����

//������� �߰�
#include <FFmpeg.h>
//#include "SDL.h"
//#include "SDL_thread.h"
//#include "SDL_ttf.h"
#include "MessageDec.h"

//���ڿ� �������� ���� ���
#include <atlconv.h>



#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#define BUF_SIZE	1024*24

inline void DebugAndLogPrint(LPCTSTR fmt, ...) {
	TCHAR szBuffer[BUF_SIZE];
	ZeroMemory(szBuffer, BUF_SIZE);

	try {
		va_list argptr;
		va_start(argptr, fmt);
		_vsntprintf_s(szBuffer, BUF_SIZE, fmt, argptr);
		va_end(argptr);

		CString strDebugMsg;
		strDebugMsg.Format(_T("[KJPlayer] : %s\n"), szBuffer);

		OutputDebugString(strDebugMsg);
	}
	catch (...) {
		CString strDebugMsg;
		strDebugMsg.Format(_T("[KJPlayer] : DebugAndLogPrint(%d)\n"), GetLastError());
		OutputDebugString(strDebugMsg);
	}
}

inline void DebugAndLogPrintA(LPCSTR fmt, ...) {
	char szBuffer[BUF_SIZE];
	ZeroMemory(szBuffer, BUF_SIZE);

	try {
		va_list argptr;
		va_start(argptr, fmt);
		_vsnprintf_s(szBuffer, BUF_SIZE, fmt, argptr);
		va_end(argptr);

		CStringA strDebugMsg;
		strDebugMsg.Format("[KJPlayer] : %s\n", szBuffer);

		OutputDebugStringA(strDebugMsg);
	}
	catch (...) {
		CStringA strDebugMsg;
		strDebugMsg.Format("[KJPlayer] : DebugAndLogPrintA(%d)\n", GetLastError());
		OutputDebugStringA(strDebugMsg);
	}
}

inline static BOOL WaitThreadEnd(CWinThread* pWinThread, int nWaitMilSec, LPCTSTR szMSG)
{
	if (!pWinThread)
		return 0;

	DebugAndLogPrint(_T("%s in - %s"), _T(__FUNCTION__), szMSG);

	DWORD dwExit = 0;
	DWORD dwRetCode = WAIT_OBJECT_0;
_gRetry: dwRetCode = WaitForSingleObject(pWinThread->m_hThread, nWaitMilSec);
	if (dwRetCode == WAIT_TIMEOUT || dwRetCode == STILL_ACTIVE)
	{
		goto _gRetry;
	}

	DebugAndLogPrint(_T("%s out - %s"), _T(__FUNCTION__), szMSG);

	return 0;
}
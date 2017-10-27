
// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 및 프로젝트 관련 포함 파일이 
// 들어 있는 포함 파일입니다.

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 일부 CString 생성자는 명시적으로 선언됩니다.

// MFC의 공통 부분과 무시 가능한 경고 메시지에 대한 숨기기를 해제합니다.
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC 핵심 및 표준 구성 요소입니다.
#include <afxext.h>         // MFC 확장입니다.





#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // Internet Explorer 4 공용 컨트롤에 대한 MFC 지원입니다.
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // Windows 공용 컨트롤에 대한 MFC 지원입니다.
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC의 리본 및 컨트롤 막대 지원

//여기부터 추가
#include <FFmpeg.h>
//#include "SDL.h"
//#include "SDL_thread.h"
//#include "SDL_ttf.h"
#include "MessageDec.h"

//문자열 컨버전을 위한 헤더
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
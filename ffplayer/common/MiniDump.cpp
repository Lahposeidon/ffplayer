#include "stdafx.h"
#include "MiniDump.h"


LPCTSTR MiniDumper::m_szAppName;

void MiniDumper::OnExit() { free( (void *)m_szAppName); }

MiniDumper::MiniDumper( LPCTSTR szAppName )
{
	if (m_szAppName != NULL)
		;
	else {
		if (szAppName == NULL)
			m_szAppName = _T("Application");
		else {
			m_szAppName = _tcsdup(szAppName);
			atexit( OnExit);
		}
	} ::SetUnhandledExceptionFilter( TopLevelFilter );
}

#pragma warning(disable:4996)
LONG MiniDumper::TopLevelFilter( struct _EXCEPTION_POINTERS *pExceptionInfo )
{
	LONG retval = EXCEPTION_CONTINUE_SEARCH;
	HWND hParent = NULL;      // find a better value for your app
	// firstly see if dbghelp.dll is around and has the function we need
	// look next to the EXE first, as the one in System32 might be old 
	// (e.g. Windows 2000)
	HMODULE hDll = NULL;
	TCHAR szDumpPath[_MAX_PATH];
	if (GetModuleFileName( NULL, szDumpPath, _MAX_PATH ))
	{
		TCHAR *pSlash = _tcsrchr( szDumpPath, _T('\\'));
		if (pSlash == NULL)
			pSlash = szDumpPath;
		else {
			_tcscpy( ++pSlash, _T("dbghelp.dll"));
			OutputDebugString( szDumpPath);
			hDll = ::LoadLibrary( szDumpPath );
		}

		_tcscpy( pSlash, _T(""));
	}
	else {
		// work out a good place for the dump file
		if (!GetTempPath( _MAX_PATH, szDumpPath ))
			_tcscpy( szDumpPath, _T("c:\\temp\\"));
	}

	if (hDll==NULL) hDll = ::LoadLibrary( _T("dbghelp.dll"));

	LPCTSTR szResult = NULL;
	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump");
		if (pDump)
		{
			TCHAR szScratch [_MAX_PATH];

			// ask the user if they want to save a dump file
			_tcscat( szDumpPath, m_szAppName );
			_tcscat( szDumpPath, _T(".dmp"));
			/* if (::MessageBox( NULL, _T("Something bad happened in your program, would you like to save a diagnostic file?"), 
				m_szAppName, MB_YESNO )==IDYES) */
			{
				// create the file
				HANDLE hFile = ::CreateFile( szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL, NULL );
				if (hFile!=INVALID_HANDLE_VALUE)
				{
					_MINIDUMP_EXCEPTION_INFORMATION ExInfo;
					ExInfo.ThreadId = ::GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionInfo;
					ExInfo.ClientPointers = NULL;
					// write the dump
					BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
					if (bOK)
					{
						_stprintf( szScratch, _T("Saved dump file to '%s'"), szDumpPath );
						szResult = szScratch;
						retval = EXCEPTION_EXECUTE_HANDLER;
					}
					else
					{
						_stprintf( szScratch, _T("Failed to save dump file to '%s' (error %d)"), szDumpPath, GetLastError() );
						szResult = szScratch;
					}
					::CloseHandle(hFile);
				}
				else
				{
					_stprintf( szScratch, _T("Failed to create dump file '%s' (error %d)"), szDumpPath, GetLastError() );
					szResult = szScratch;
				}
			}
		}
		else
		{
			szResult = _T("DBGHELP.DLL too old");
		}
	}
	else
	{
		szResult = _T("DBGHELP.DLL not found");
	}
	if (szResult)
		OutputDebugString(szResult); //::MessageBox( NULL, szResult, m_szAppName, MB_OK );
	return retval;
}
#pragma warning(default:4996)


// ffplayer.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CffplayerApp:
// �� Ŭ������ ������ ���ؼ��� ffplayer.cpp�� �����Ͻʽÿ�.
//

class CffplayerApp : public CWinApp
{
public:
	CffplayerApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CffplayerApp theApp;

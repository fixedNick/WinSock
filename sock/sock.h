// sock.h : main header file for the sock DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CsockApp
// See sock.cpp for the implementation of this class
//

class CsockApp : public CWinApp
{
public:
	CsockApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

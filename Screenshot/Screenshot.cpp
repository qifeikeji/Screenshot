// Screenshot.cpp : ???????????????????
//

#include "stdafx.h"
#include "Screenshot.h"
#include "CatchScreenDlg.h"
#include "AppSettings.h"
#include "qt/MainWindow.h"
#include "qt/ScreenshotBridge.h"
#include "StartupUtil.h"
#include <GdiPlus.h>
#include <QApplication>


using namespace Gdiplus;
#pragma comment(lib,"GdiPlus.lib")





#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define SHIFTED 0x8000 


// CScreenshotApp

BEGIN_MESSAGE_MAP(CScreenshotApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CScreenshotApp ????

CScreenshotApp::CScreenshotApp()
{
	m_hwndDlg = NULL;
}


// ???????? CScreenshotApp ????

CScreenshotApp theApp;


// CScreenshotApp ?????

BOOL CScreenshotApp::InitInstance()
{
	// Per-monitor DPI???? app.manifest ?????????? API ??????????
	HMODULE hUser32 = GetModuleHandle(_T("user32.dll"));
	if (hUser32)
	{
		typedef BOOL(WINAPI* SetDpiCtxFn)(DPI_AWARENESS_CONTEXT);
		SetDpiCtxFn pfn = (SetDpiCtxFn)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
		if (pfn)
			pfn(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	}

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ?????????????????????????????????
	// ?????????
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	
	GdiplusStartupInput input;
	GdiplusStartup(&m_gdiplusToken,&input,NULL);

	GetAppSettings().Load();

	if (!TryBeginSingleInstance())
	{
		static int qtArgc = 1;
		static char qtArg0[] = "Screenshot";
		static char* qtArgv[] = { qtArg0, nullptr };
		QApplication qtApp(qtArgc, qtArgv);
		ApplyQtDarkTheme(&qtApp);
		ShowAlreadyRunningMessage();
		ActivateExistingScreenshotInstance();
		return FALSE;
	}

	AfxEnableControlContainer();

	// ????????
	// ????????????????????????
	// ????????????????????????????
	// ???????????????????
	// ?????????????????????
	// TODO: ???????????????
	// ??????????????????
	SetRegistryKey(_T("????????????????????????"));

	static int qtArgc = 1;
	static char qtArg0[] = "Screenshot";
	static char* qtArgv[] = { qtArg0, nullptr };
	QApplication qtApp(qtArgc, qtArgv);
	ApplyQtDarkTheme(&qtApp);

	MainWindow mainWin;
	mainWin.show();

	qtApp.exec();

	return FALSE;
}

BOOL CScreenshotApp::ProcessMessageFilter(int code, LPMSG lpMsg) 
{
	if (m_hwndDlg != NULL)
	{
		if ((lpMsg->hwnd == m_hwndDlg) || ::IsChild(m_hwndDlg, lpMsg->hwnd))
		{
			if (lpMsg->message == WM_KEYDOWN)
			{
				CWnd* pWnd = CWnd::FromHandlePermanent(m_hwndDlg);
				CCatchScreenDlg* pDlg = dynamic_cast<CCatchScreenDlg*>(pWnd);
				if (!pDlg)
					return CWinApp::ProcessMessageFilter(code, lpMsg);

				CRect rect(0, 0, 0, 0);
				rect = pDlg->m_rectTracker.m_rect;

				if (pDlg->m_bFirstDraw)
				{
					//???Shift???????????????????
					BOOL bIsShiftDown = FALSE;

					if (GetKeyState(VK_SHIFT) & SHIFTED) 
						bIsShiftDown = TRUE;

					////////////////////////////////////////////////////

					switch(lpMsg->wParam)
					{
					case VK_UP:

						//???????Shift,??????????
						if(!bIsShiftDown)
							rect.top-=1;

						rect.bottom-=1;
						pDlg->m_rectTracker.m_rect = rect;
						pDlg->InvalidateAroundRect(rect);
						break;

					case VK_DOWN:
						rect.top+=1;
						if(!bIsShiftDown)
							rect.bottom+=1;

						pDlg->m_rectTracker.m_rect=rect;
						pDlg->InvalidateAroundRect(rect);
						break;

					case VK_LEFT:
						if(!bIsShiftDown)
							rect.left-=1;
						rect.right-=1;

						pDlg->m_rectTracker.m_rect=rect;
						pDlg->InvalidateAroundRect(rect);
						break;

					case VK_RIGHT:
						rect.left+=1;
						if(!bIsShiftDown)
							rect.right+=1;

						pDlg->m_rectTracker.m_rect=rect;
						pDlg->InvalidateAroundRect(rect);
						break;
					}
				}
			}

		}
	} 

	return CWinApp::ProcessMessageFilter(code, lpMsg);
}
int CScreenshotApp::ExitInstance()
{
	ReleaseSingleInstanceMutex();
	GdiplusShutdown(m_gdiplusToken);

	return CWinApp::ExitInstance();
}

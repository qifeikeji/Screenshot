#include "stdafx.h"
#include "Screenshot.h"

// Qt 6 链接 Qt6EntryPoint，需要 main()；MFC 仍由 theApp / AfxWinInit 启动。
int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

#ifdef _UNICODE
	HINSTANCE hInstance = ::GetModuleHandleW(nullptr);
	LPTSTR lpCmdLine = ::GetCommandLineW();
#else
	HINSTANCE hInstance = ::GetModuleHandleA(nullptr);
	LPTSTR lpCmdLine = ::GetCommandLineA();
#endif

	if (!AfxWinInit(hInstance, nullptr, lpCmdLine, SW_SHOWDEFAULT))
		return 1;

	CWinApp* const pApp = AfxGetApp();
	if (pApp == nullptr || !pApp->InitInstance())
		return 1;

	return pApp->ExitInstance();
}

#include "stdafx.h"

// Qt 6 links Qt6EntryPoint (expects main). MFC uses AfxWinMain / theApp.
int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
#ifdef _UNICODE
	return AfxWinMain(::GetModuleHandleW(nullptr), nullptr, ::GetCommandLineW(), SW_SHOWDEFAULT);
#else
	return AfxWinMain(::GetModuleHandle(nullptr), nullptr, ::GetCommandLineA(), SW_SHOWDEFAULT);
#endif
}

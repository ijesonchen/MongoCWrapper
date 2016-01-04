// WrapperMfc.cpp : 定义控制台应用程序的入口点。
//
#ifdef _MSC_VER
	#pragma warning(disable: 4324 4189)
#endif // _MSC_VER

#include "stdafx.h"
#include "TestFunc.h"
#include "TestObs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// 唯一的应用程序对象

CWinApp theApp;

using namespace std;

int _tmain(/*int argc, TCHAR* argv[], TCHAR* envp[]*/)
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO:  更改错误代码以符合您的需要
			_tprintf(_T("错误:  MFC 初始化失败\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO:  在此处为应用程序的行为编写代码。
		}
	}
	else
	{
		// TODO:  更改错误代码以符合您的需要
		_tprintf(_T("错误:  GetModuleHandle 失败\n"));
		nRetCode = 1;
	}

	// main test code
	TestMongo();
	TestObs();

	std::cout << "press enter to exit." << std::endl;
	getchar();
	// this code is used to check if memory leak detect is activated.
	char* p = new char[1234];

	return nRetCode;
}

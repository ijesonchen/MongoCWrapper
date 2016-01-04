// WrapperMfc.cpp : �������̨Ӧ�ó������ڵ㡣
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



// Ψһ��Ӧ�ó������

CWinApp theApp;

using namespace std;

int _tmain(/*int argc, TCHAR* argv[], TCHAR* envp[]*/)
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// ��ʼ�� MFC ����ʧ��ʱ��ʾ����
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO:  ���Ĵ�������Է���������Ҫ
			_tprintf(_T("����:  MFC ��ʼ��ʧ��\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO:  �ڴ˴�ΪӦ�ó������Ϊ��д���롣
		}
	}
	else
	{
		// TODO:  ���Ĵ�������Է���������Ҫ
		_tprintf(_T("����:  GetModuleHandle ʧ��\n"));
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

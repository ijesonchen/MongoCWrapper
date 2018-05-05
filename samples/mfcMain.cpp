// WrapperMfc.cpp : �������̨Ӧ�ó������ڵ㡣
//
#ifdef _MSC_VER
	#pragma warning(disable: 4324 4189)
#endif // _MSC_VER

#include "stdafx.h"
#include "TestFunc.h"
#include "TestObs.h"
#include "TestMongoc.h"
#include <cstdlib>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _MSC_VER
	#ifdef _WIN64 
		#ifdef _DEBUG
			#pragma comment(lib, "..\\..\\winlibmongoc\\lib\\x64Debug\\bson-1.0.lib")
			#pragma comment(lib, "..\\..\\winlibmongoc\\lib\\x64Debug\\mongoc-1.0.lib")
		#else
			#pragma comment(lib, "..\\..\\winlibmongoc\\lib\\x64Release\\bson-1.0.lib")		// x64RelWithDebInfo or x64Release
			#pragma comment(lib, "..\\..\\winlibmongoc\\lib\\x64Release\\mongoc-1.0.lib")	// x64RelWithDebInfo or x64Release
		#endif // _DEBUG
	#else
		#pragma comment(lib, "..\\..\\winlibmongoc\\lib\\x86Debug\\bson-1.0.lib")
		#pragma comment(lib, "..\\..\\winlibmongoc\\lib\\x86Debug\\mongoc-1.0.lib")
	#endif // _WIN64
#endif // _MSC_VER


// Ψһ��Ӧ�ó������

CWinApp theApp;

using namespace std;

//int _tmain(int argc, TCHAR* argv[]/*, TCHAR* envp[]*/)
int _tmain(void)
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

// 	if (argc != 4)
// 	{
// 		std::cout << "exec DbAddr DbName" << std::endl;
// 		return 0;
// 	}
// 
// 	std::wstring dbaddr;
// 	std::wstring dbname;
// 	CStringA scount;
// 	int nCount = 0; 
// 
// 	dbaddr = argv[1];
// 	dbname = argv[2];
// 	scount = argv[3];
// 	nCount = std::atoi(scount.GetString());
// 
// 	std::wcout << L"param: " << dbaddr << L" " << dbname << " " << nCount << std::endl;
// 	ConnTestStress(dbaddr, dbname, nCount);
// 	
// 	std::cout << "finished" << std::endl;
// 	return 0;

	// main test code
	TestBson();
//	TestMongoC();
//	TestMongo();
//	TestObs();

	std::cout << "press enter to exit." << std::endl;
	getchar();
	// this code is used to check if memory leak detect is activated.
	char* p = new char[1234];

	return nRetCode;
}

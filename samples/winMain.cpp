#ifdef _MSC_VER
	#pragma warning(disable: 4324 4189)
#endif // _MSC_VER

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

#include <iostream>
#include "TestFunc.h"
#include "TestObs.h"

using namespace std;
using namespace MongoClib;
int main(void)
{
	cout << "test begin" << endl;
	// main test code
	TestBson();
	TestMongo();
	TestObs();

	cout << "press enter to exit." << endl;
	getchar();

	// this code is used to check if memory leak detect is activated.
	char* p = new char[1234];

	return 0;
}
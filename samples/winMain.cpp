#ifdef _MSC_VER
	#pragma warning(disable: 4324 4189)
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
	TestMongo();
	TestObs();

	cout << "press enter to exit." << endl;
	getchar();

	// this code is used to check if memory leak detect is activated.
	char* p = new char[1234];

	return 0;
}
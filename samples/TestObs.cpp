#include "mfcafx.h"
#include "TestObs.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <memory>

#include <mongoc.h>

#include "MongoAuto.h"
#include "MongocHelp.h"
#include "MongoClient.h"
#include "MongoObs.h"

#include "TeacherAccess.h"

#ifdef __AFX_DEBUG_NEW__
	#include "StudentAccess.h"
#endif // __AFX_DEBUG_NEW__

#include "mfcnew.h"

using namespace std;
using namespace MongoClib;


extern const char* szSrv;
extern const char* szDb;
extern const char* szLoc;

extern const wchar_t* wszSrv;
extern const wchar_t* wszDb;

extern void Trap(void* p = nullptr);

void TestObs(void)
{
	if (Init(wszSrv, wszDb, szLoc, 2000))
	{
		Trap();
	}

	Cleanup();
	cout << endl;
	cout << "finished. press enter to exit." << endl;
}

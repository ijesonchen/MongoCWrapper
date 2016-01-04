#ifndef TestFunc_h__
#define TestFunc_h__

#include <iostream>
#include <string>
#include <vector>
#include <bson.h>
#include <mongoc.h>
#include "MongoClient.h"



extern const char* szSrv;
extern const char* szDb;
extern const char* szLoc;

extern const wchar_t* wszSrv;
extern const wchar_t* wszDb;


inline void Trap(void* p = nullptr)
{
	if (p)
	{
		return;
	}
	std::wcout << L"trapped!" << std::endl;
	exit(-1);
}


void TestIndex(void);

void TestBuildDoc(void);

void TestClient(void);

void TestConn(void);

void TestStudent(void);
void TestTeacher(void);

void TestLoadBin(void);

void TestInsert(void);

void TestU8(void);

void TestAnsi(void);

void TestBulk(void);

inline void TestMongo(void)
{
	if (MongoClib::Init(wszSrv, wszDb, szLoc, 2000))
	{
		Trap();
	}

	if (!MongoClib::DropDataBase())
	{
		Trap();
	}
	
	TestStudent();
	TestTeacher();
	TestBulk();

	MongoClib::Cleanup();
	std::cout << std::endl;
	std::cout << "finished. press enter to exit." << std::endl;
}

#endif // TestFunc_h__

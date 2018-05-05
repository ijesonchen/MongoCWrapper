#ifndef TestFunc_h__
#define TestFunc_h__

#include <iostream>
#include <string>
#include <vector>
#include <bson.h>
#include <mongoc.h>
#include "MongoClient.h"
#include <tuple>


/*
// $cmp Returns: 0 if the two values are equivalent, 1 if the first value is greater than the second, and -1 if the
// 	first value is less than the second.
// $eq Returns true if the values are equivalent.
// $gt Returns true if the first value is greater than the second.
// $gte Returns true if the first value is greater than or equal to the second.
// $lt Returns true if the first value is less than the second.
// $lte Returns true if the first value is less than or equal to the second.
// $ne Returns true if the values are not equivalent.

*/



extern const char* szSrv;
extern const char* szDb;
extern const char* szLoc;

extern const wchar_t* wszSrv;
extern const wchar_t* wszDb;




void TestIndex(void);

void TestBuildDoc(void);

void TestClient(void);

void TestConn(void);

void TestRename(void);
void TestStudent(void);
void TestTeacher(void);

void TestLoadBin(void);

void TestInsert(void);

void TestU8(void);

void TestAnsi(void);

void TestBulk(void);

void TestCombinUnique(void);

void TestBigDoc(void);

void TestPerf(void);

void TestBit(void);
void TestTimet(void);
void TestDateTime(void);

void TestSetOr(void);

void TestArray(void);

void TestMfc(void);

void TestTuple(void);

void TestTriAngle(void);

void TestOidTime(void);
void TestOidClass(void);
void ConnTestStress(const std::wstring& srv, const std::wstring& dbname, const int count);
void TestStress(const int count = 100000);

void TestParseNoFail(void);
void TestLogColl(void);

void TestBsonFromJson(void);
void TestBsonReader(void); 
MongoClib::AutoBson TestBsonArray(void);
void TestArrayParser(MongoClib::AutoBson& doc);

inline void Trap(void* p = nullptr)
{
	if (p)
	{
		return;
	}
	std::wcout << L"trapped!" << std::endl;
	abort();
	exit(-1);
}

inline void Pause(void)
{
	std::cout << "press enter." << std::endl;
	getchar();
}

inline void TestBson(void)
{
	TestBsonFromJson();
	auto d = TestBsonArray();
	TestArrayParser(d);
//	TestBsonReader();
}

inline void TestMongo(void)
{
	if (MongoClib::Init(wszSrv, wszDb, 2000))
	{
		Trap();
	}

// 	if (!MongoClib::DropDataBase())
// 	{
// 		Trap();
// 	}
//	TestRename();
//	TestOidClass();
//	TestOidTime();
//	TestBulk();
//	TestStress();
			
// 	TestStudent();
// 	TestTeacher();

// 	bool loop = true;
// 	int idx = 0;
// 	while (loop)
// 	{
// 		TestBulk();
// 		std::cout << "***************loop " << ++idx << std::endl;
// 	}
// 	TestCombinUnique();
// 	TestBigDoc();
//	TestPerf();
//	TestBit();
//	TestTimet();
//	TestDateTime();
//	TestBulk();
//	TestSetOr();
//	TestArray();
//	TestClient();
//	TestTuple();
//	TestLogColl();
//	TestParseNoFail();

	MongoClib::Cleanup();
	std::cout << std::endl;
	std::cout << "finished. press enter to exit." << std::endl;
}

#endif // TestFunc_h__

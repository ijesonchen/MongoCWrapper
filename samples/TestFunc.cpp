#include "mfcafx.h"


#include "TestFunc.h"

#include <iostream>
#include <chrono>
#include <sstream>
#include <vector>
#include <fstream>
#include <memory>
#include <thread>
#include <chrono>

#include <mongoc.h>

#include "MongoAuto.h"
#include "MongocHelp.h"
#include "MongoClient.h"

#include "TeacherAccess.h"
#include "MongoLog.h"

#ifdef __AFX_DEBUG_NEW__
	#include "StudentAccess.h"
#endif // __AFX_DEBUG_NEW__

#include "OidTimeAccess.h"

#include "mfcnew.h"

using namespace std;
using namespace MongoClib;

const char* szSrv = "192.168.1.120";
const char* szDb = "mytestdb";

#ifdef _WIN32
const char* szLoc = "chs";
#endif // _WIN32
#ifdef __linux__
const char* szLoc = "zh_CN.utf8";
#endif // _DEBUG



const wchar_t* wszSrv = L"192.168.13.238";
const wchar_t* wszDb = L"Test230";


void TestIndex(void)
{
}

struct DocData
{
	struct SubData
	{
		int idx = 118;
		string name = "subname";
	};
	int idx = 18;
	string name = "strname";
	vector<wstring> paths;
	SubData child;
	vector<SubData> subarray;
};

AutoBson BuildDocData(const DocData& obj)
{
	AutoBson doc;
	if (!doc)
	{
		return nullptr;
	}
	doc.Add("idx", obj.idx);
	doc.Add("name", obj.name);
	doc.Add("paths", obj.paths, wstring2str);
	
	// add doc array
	ArrayBuilder arrayBuilder;
	for (auto it : obj.subarray)
	{
		AutoBson sub;
		sub.Add("idx", it.idx);
		sub.Add("name", it.name);
		arrayBuilder.AddSub(sub);
	}
	doc.AddArray("subarray", arrayBuilder);

	AutoBson child;
	child.Add("idx", obj.child.idx);
	child.Add("name", obj.child.name);
	doc.AddDoc("child", child);

	if (doc.IsFailed())
	{
		return nullptr;
	}

	return std::move(doc);
}

bool ParseDocBson(DocData& obj, const AutoCursor& cursor)
{
	bool bFailed = false;
	string oid = cursor.Oid();
	time_t tmOid = cursor.OidTime();
	time_t tmSpan = time(nullptr) - tmOid;
	cout << "timespan " << tmSpan << endl;

	obj.idx = cursor.Int32("idx");
	obj.name = cursor.Str("name");
	cursor.GetCntr("paths", obj.paths, str2wstring);

	ArrayParser arrayParser(cursor.ResultBson(), "subarray");
	if (!arrayParser.HasElement())
	{
		Trap();
	}
	while (arrayParser.Next())
	{
		obj.subarray.emplace_back();
		DocData::SubData& s = obj.subarray.back();
		s.idx = arrayParser.Int32("idx");
		s.name = arrayParser.Str("name");
		bFailed |= arrayParser.IsFailed();
	}

	const AutoBson child = cursor.Doc("child");
	obj.child.idx = child.Int32("idx");
	obj.child.name = child.Str("name");

	bFailed |= child.IsFailed();

	bFailed |= cursor.IsFailed();
	if (bFailed)
	{
		Trap();
	}

	return true;
}

void TestBuildDoc(void)
{
	bool bRet = false;
	DocData obj;
// 	int nvt = 5;
// 	for (int ii = 0; ii < nvt; ++ii)
// 	{
// 		wstringstream ss;
// 		ss << L"path" << ii;
// 		obj.paths.push_back(ss.str());
// 	}
// 	for (int ii = 0; ii < nvt; ++ii)
// 	{
// 		stringstream ss;
// 		ss << "arraymem" << ii;
// 		DocData::SubData s;
// 		s.idx = 100 + ii;
// 		s.name = ss.str();
// 		obj.subarray.push_back(s);
// 	}

	AutoBson doc = BuildDocData(obj);

	if (!doc)
	{
		Trap();
	}

	AutoJson js(bson_as_json(doc, nullptr));

	cout << "Object: " << endl;
	cout << js << endl;

	AutoPoolColl collection("mycoll");

	if (!collection.Insert(doc))
	{
		Trap();
	}
	
	AutoBson query;

	AutoCursor cursor = collection.Find(query);

	while (cursor.Next())
	{
		DocData d;
		if (!ParseDocBson(d, cursor))
		{
			Trap();
		}
		AutoJson j(cursor.ResultBson());
		cout << "Object Read: " << endl;
		cout << j << endl;
	}
	
	cout << "return " << bRet << endl;
}

void TestClient(void)
{
	int nRet = 0;

	if (nRet)
	{
		Trap();
	}

	vector<mongoc_client_t*> vtClient;
	mongoc_client_t* client = TryPopClient();

	if (!client)
	{
		Trap();
	}


	PushClient(client);
}


void TestConn(void)
{
	int nRet = Init(L"192.168.1.120", L"mydb", 2000, 2000, 2000);
	if (nRet)
	{
		cout << L"Init failed: " << nRet << endl;
		return;
	}

	AutoPoolColl collection("mycoll");

	if (!collection)
	{
		return;
	}

	AutoBson query;
	AutoBson field;


	AutoCursor cursor = collection.Find(query, 0, field);
	
	const bson_t* current = mongoc_cursor_current(cursor.Cursor());
	if (current)
	{
		AutoJson json(current);
		cout << json << endl;
	}

	while (cursor.Next())
	{
		AutoJson json(cursor.ResultBson());
		cout << json << endl;
	}
	
	long long n64 = mongoc_collection_count(collection, MONGOC_QUERY_NONE, query, 0, 0, nullptr, nullptr);
	cout << "count : " << n64 << endl;
}



template <class Facet>
struct deletable_facet : Facet
{
	template <class ...Args>
	deletable_facet(Args& ... args) : Facet(std::forward<Args>(args)...)
	{ }
	~deletable_facet() {}
};

void TestLoadBin(void)
{
	TestInsert();
	int nRet = Init(L"192.168.1.120", L"mydb", 2000, 2000, 2000);
	if (nRet)
	{
		Trap();
	}

	AutoPoolColl collection("mycoll");
	if (!collection)
	{
		Trap();
	}

	AutoCursor cursor = collection.Find(AutoBson());
	int ii = 0;
	while (cursor.Next())
	{
		AutoJson json(cursor.ResultBson());
		cout << "    ****    " <<  ii++ << endl;
		cout << json << endl;
		string str = cursor.Bin("bin");

		const char* p = str.c_str();
		cout << p << endl;
	}
}


void TestInsert(void)
{
// 	mongoc_client_t *client;
// 	mongoc_collection_t *collection;
// 	bson_error_t error = {};
// 	bson_t *doc;
// 
// 	client = mongoc_client_new("mongodb://192.168.1.120:27017/");
// 	collection = mongoc_client_get_collection(client, "mydb", "mycoll");
// 
// 	string err;
// 	if (!CreateIndex(collection, "group", true, true, &err))
// 	{
// 		Trap();
// 	}
// 
// 	const int nBinLen = 100;
// 	char* pBin = new char[nBinLen];
// 	for (int ii = 0; ii < nBinLen; ++ii)
// 	{
// 		pBin[ii] = (char)ii;
// 	}
// 	unique_ptr<char> upBin(pBin);
// 
// 	for (int ii = 0; ii < 1000; ++ii)
// 	{
// 		doc = bson_new();
// 		BSON_APPEND_INT32(doc, "idx", ii);
// 		stringstream ss;
// 		ss << "object_" << ii;
// 		BSON_APPEND_UTF8(doc, "name", ss.str().c_str());
// 		ss.str("");
// 
// 		BSON_APPEND_INT32(doc, "group", ii % 10);
// 		BSON_APPEND_INT32(doc, "tag", ii / 10);
// 
// 		BSON_APPEND_BINARY(doc, "bin", BSON_SUBTYPE_BINARY, (uint8_t*)pBin, nBinLen);
// 
// 		AutoJson json(doc);
// 
// 		if (!mongoc_collection_insert(collection, MONGOC_INSERT_NONE, doc, NULL, &error)) {
// 			fprintf(stderr, "%s\n", error.message);
// 		}
// 
// 		bson_destroy(doc);
// 	}
// 	mongoc_collection_destroy(collection);
// 	mongoc_client_destroy(client);
}

void TestU8(void)
{
	// only passed in MSVS2013, gcc 4.8.2 not support <codecvt>
//	wstring wstr = L"测试aaa";
//	std::wstring_convert < codecvt<wchar_t, char, mbstate_t>, wchar_t > convertor;

// 	std::wstring_convert < deletable_facet<codecvt<wchar_t, char, mbstate_t>>, wchar_t > convertor;
// 	string str3 = convertor.to_bytes(wstr); 
// 	string str4 = Unicode2Ansi(wstr.c_str());
// 	if (str3 != str4)
// 	{
// 		Trap();
// 	}
// 
// 	wstring wstr3 = convertor.from_bytes(str3);
// 	wstring wstr4 = Ansi2Unicode(str3.c_str());
// 	if (wstr3 != wstr4)
// 	{
// 		Trap();
// 	}
// 
// 
// 	string str1 = EncUtf8(wstr);
// 	string str2 = toU8(wstr);
// 	if (str1 != str2)
// 	{
// 		Trap();
// 	}
// 	wstring wstr1 = DecUtf8(str1);
// 	wstring wstr2 = fromU8(str1);
// 	if (wstr1 != wstr2)
// 	{
// 		Trap();
// 	}
}



void TestAnsi(void)
{
	const char* p = "你好test最近";
	const wchar_t* pw = L"你好test最近";

	string astr = EncAnsi(pw);
	wstring wstr = DecAnsi(p);

	if (astr != p)
	{
		Trap();
	}
	if (wstr != pw)
	{
		Trap();
	}
}


void TestStudent(void)
{
#ifdef __AFX_DEBUG_NEW__
	static int nCounter = 100;
			
	deque<Student> vt0;
	int nCount = 100;
	int nAccount = MongoClib::knAccountPerPeople;
	for (int ii = 0; ii < nCount; ++ii)
	{
		Student stu;
		int idx = ++nCounter;
		stu.strName.Format(L"Name_%04d", ii);
		stu.strMemo.Format(L"memo_%d", ii % 10);
		stu.bValue = ii & 1;
		stu.nValue = ii % 10;
		stu.fValue = (float)(ii % 10);
		stu.vtText.push_back(L"aaa");
		stu.vtText.push_back(L"bbb");
		stu.vtText.push_back(L"ccc");
		for (int jj = 0; jj < nAccount; ++jj)
		{
			Account acc;
			acc.guid = idx + jj * 3;
			acc.name = "account";
			acc.memo = "memo";
			acc.inuse = (idx + jj) % 2 == 1;
			stu.vtAccout.push_back(acc);
		}
		vt0.push_back(stu);
	}

	StudentAccess acc;
	if (!acc.CreateIndexs())
	{
		Trap();
	}
	for (auto it : vt0)
	{
		if (!acc.Insert(it))
		{
			Trap();
		}
	}


	deque<Student> vt1;
	if (!acc.LoadAll(vt1))
	{
		Trap();
	}

	if (vt0 != vt1)
	{
		for (size_t ii = 0, ni = vt0.size();
			ii < ni; ++ii)
		{
			if (vt0[ii] == vt1[ii])
			{
				continue;
			}
			Trap();
		}
	}
	if (!acc.Delete(L"Name_0001"))
	{
		Trap();
	}
	if (!acc.Delete(L"Name_0001"))
	{
		Trap();
	}

	Student student = vt0.front();
	student.strMemo = L"changed.";
	student.bValue = true;
	student.nValue = 999;
	student.fValue = (float)123.45;
	if (!acc.Update(student))
	{
		Trap();
	}

	if (acc.UpdateBool(L"Name_0001", true))
	{
		Trap();
	}
	
	if (!acc.UpdateBool(L"Name_0002", true))
	{
		Trap();
	}

	if (!acc.UpdateBool(L"Name_0002", false))
	{
		Trap();
	}

	if (!acc.LoadOne(L"Name_0002", student))
	{
		Trap();
	}

	vt1.clear();
	if (!acc.LoadCond(vt1, 5))
	{
		Trap();
	}

	int n = 0;
	n = acc.LoadState(L"Name_0002");
	n = acc.LoadState(L"Name_0003");
	n = acc.LoadState(L"Name_0013");
	n = acc.LoadState(L"Name_0033");

	cout << "**** student test passed." << endl;
#endif //__AFX_DEBUG_NEW__
}



void TestTeacher(void)
{
	static int nCounter = 10;

	deque<Teacher> vt0;
	int nCount = 100;
	int nAccount = MongoClib::knAccountPerPeople;
	for (int ii = 0; ii < nCount; ++ii)
	{
		Teacher obj;
		int idx = ++nCounter;
		obj.idx = idx;
		obj.group = idx % 5;
		stringstream ss;
		ss << "name_中文_" << obj.idx << "_" << obj.group;
		obj.name = ss.str();
		obj.wname = DecAnsi(obj.name);

		if (!obj.wname.length() && obj.name.length())
		{
			cout << "DecAnsi failed. maybe bad loc string in Init()" << endl;
			Trap();
		}

		obj.students.push_back("student1");
		obj.students.push_back("student2"); 
		obj.students.push_back("student3");
		obj.students.push_back("student4");

		for (int jj = 0; jj < nAccount; ++jj)
		{
			Account acc;
			acc.guid = idx + jj * 3;
			acc.name = "account";
			acc.memo = "memo";
			acc.inuse = (idx + jj) % 2 == 1;
			obj.accounts.push_back(acc);
		}

		const int binlen = 100;
		char bin[binlen] = { 0 };
		char cnt = (char)idx;
		for (int jj = 0; jj < binlen; ++jj)
		{
			bin[jj] = ++cnt;
		}
		obj.binData.assign(bin, binlen);
		vt0.push_back(obj);
	}

	TeacherAccess acc;
	if (!acc.CreateIndexs())
	{
		Trap();
	}

	for (auto it : vt0)
	{
		if (!acc.Insert(it))
		{
			Trap();
		}
	}


	deque<Teacher> vt1;
	if (!acc.LoadAll(vt1))
	{
		Trap();
	}

	if (vt0 != vt1)
	{
		for (size_t ii = 0, ni = vt0.size();
			ii < ni; ++ii)
		{
			if (vt0[ii] == vt1[ii])
			{
				continue;
			}
			Trap();
		}
	}

	if (!acc.Delete(1234))
	{
		Trap();
	}

	Teacher obj0 = vt0.front();

	Teacher objLoad;
	if (!acc.LoadOne(obj0.idx, objLoad))
	{
		Trap();
	}
	if (obj0 != objLoad)
	{
		Trap();
	}


	obj0.name = "changed.";
	obj0.group = 999;
	if (!acc.Update(obj0))
	{
		Trap();
	}

	string obj0bin = acc.LoadBinData(obj0.idx);
	if (obj0bin != obj0.binData)
	{
		Trap();
	}

	string newbin = "this is new test";
	if (!acc.UpdateBin(obj0.idx, newbin))
	{
		Trap();
	}
	obj0bin = acc.LoadBinData(obj0.idx);
	if (obj0bin != newbin)
	{
		Trap();
	}

	if (!acc.Delete(obj0.idx))
	{
		Trap();
	}

	// obj0 removed, should fail.
	if (acc.UpdateBin(obj0.idx, newbin))
	{
		Trap();
	}

	deque<Teacher> vt2;
	if (!acc.LoadCond(vt2, 3))
	{
		Trap();
	}	

	cout << "**** Teacher test passed." << endl;
}


void TestBulk(void)
{
	/*
	10,000 record
	for release build, insert
	bulk is about 4-5 times fast than single
	single: 3.3K per sec
	bulk: 14-17K per sec

	for bulk:
	insert1:
		for: struct -> doc -> bulk
		exec
	insert2:
		for: struct -> doc
		for: doc -> bulk
		exec
	insert2 is about 20% fast than insert1

	**** WrapperMfc_Debug_Win32
	cntr assign cost (us): 442,025
	cntr push cost (us): 443,025
	Inserted: 10,000
	bulk inserter cost (us): 3,462,198
	ResetName cost (us): 260,014
	nModified: 10,000
	bulk Update cost (us): 1,562,089
	ResetName cost (us): 253,014
	nModified: 10,000
	bulk Replace cost (us): 2,040,116
	nRemoved: 10,000
	bulk Delete cost (us): 1,587,090
	single insert cost (us): 6,445,368
	single delete cost (us): 3,695,211
	**** Bulk test passed.


	**** WrapperMfc_Release_Win32
	cntr assign cost (us): 13,000
	cntr push cost (us): 14,000
	Inserted: 10,000
	bulk inserter2 cost (us): 602,034
	nRemoved: 10,000
	Inserted: 10,000
	bulk inserter cost (us): 723,041
	ResetName cost (us): 250,014
	nModified: 10,000
	bulk Update cost (us): 873,050
	ResetName cost (us): 268,015
	nModified: 10,000
	bulk Replace cost (us): 1,108,063
	nRemoved: 10,000
	bulk Delete cost (us): 905,051
	single insert cost (us): 2,946,168
	single delete cost (us): 2,993,171
	**** Bulk test passed.
	*/
	static int nCounter = 100000;

	deque<Teacher> vt0;
	int nCount = 100000;
	cout << "test count " << nCount << endl;
	int nAccount = MongoClib::knAccountPerPeople;
	for (int ii = 0; ii < nCount; ++ii)
	{
		Teacher obj;
		int idx = ++nCounter;
		obj.idx = idx;
		obj.group = idx % 5;
		stringstream ss;
		ss << "name_中文_" << obj.idx << "_" << obj.group;
		obj.name = ss.str();
		obj.wname = DecAnsi(obj.name);

		if (!obj.wname.length() && obj.name.length())
		{
			cout << "DecAnsi failed. maybe bad loc string in Init()" << endl;
			Trap();
		}

		obj.students.push_back("student1");
		obj.students.push_back("student2");
		obj.students.push_back("student3");
		obj.students.push_back("student4");

		for (int jj = 0; jj < nAccount; ++jj)
		{
			Account acc;
			acc.guid = idx + jj * 3;
			acc.name = "account";
			acc.memo = "memo";
			acc.inuse = (idx + jj) % 2 == 1;
			obj.accounts.push_back(acc);
		}

		const int binlen = 100;
		char bin[binlen] = { 0 };
		char cnt = (char)idx;
		for (int jj = 0; jj < binlen; ++jj)
		{
			bin[jj] = ++cnt;
		}
		obj.binData.assign(bin, binlen);
		vt0.push_back(obj);
	}


	TeacherAccess acc;
	if (!acc.CreateIndexsNoUnique())
	{
		Trap();
	}

	Timer timer;
	locale loc(szLoc);
	cout.imbue(loc);

	timer.Tick();
	auto vt1 = vt0;
	cout << "cntr assign cost (us): " << timer.Tock() << endl;

	timer.Tick();
	decltype(vt0) vt2;
	for (auto& it: vt0)
	{
		vt2.push_back(it);
	}
	cout << "cntr push cost (us): " << timer.Tock() << endl;

	static bool bFirst = true;

	if (bFirst)
	{
		timer.Tick();
		if (!acc.Insert2(vt0))
		{
			Trap();
		}
		bFirst = false;
		cout << "bulk inserter2 cost (us): " << timer.Tock() << endl;
	}
	Pause();

	/*
	decltype(vt0) vt3;
	for (auto& it : vt0)
	{
		Teacher obj;
		obj.idx = it.idx;
		vt3.push_back(obj);
	}

	AutoPoolColl coll("Teacher");

	AutoBson field;
	field.Add("group", true);
	field.Add("wname", true);

	for (auto& it : vt3)
	{
		AutoBson query;
		query.Add("idx", it.idx);
		AutoCursor cursor = coll.Find(query, 1, field);
		if (cursor.Next())
		{
			it.group = cursor.Int32("group");
			it.wname = cursor.WStr("wname");
		}
	}
	

	acc.Delete(vt0);

	timer.Tick();
	if (!acc.Insert(vt0))
	{
		Trap();
	}
	cout << "bulk inserter cost (us): " << timer.Tock() << endl;
	*/
	cout << "ResetName" << endl;
	timer.Tick();
	if (!acc.ResetName())
	{
		Trap();
	}
	cout << "ResetName cost (us): " << timer.Tock() << endl;
	Pause();
	
	timer.Tick();
	if (!acc.Update(vt0))
	{
		Trap();
	}
	cout << "bulk Update cost (us): " << timer.Tock() << endl;
	Pause();

	
	timer.Tick();
	if (!acc.ResetName())
	{
		Trap();
	}
	cout << "ResetName cost (us): " << timer.Tock() << endl;

	timer.Tick();
	if (!acc.Replace(vt0))
	{
		Trap();
	}
	cout << "bulk Replace cost (us): " << timer.Tock() << endl;

	timer.Tick();
	if (!acc.Delete(vt0))
	{
		Trap();
	}
	cout << "bulk Delete cost (us): " << timer.Tock() << endl;
	cout << "bulk test finished." << endl;
	
	timer.Tick();
	for (auto& it: vt0)
	{
		if (!acc.Insert(it))
		{
			Trap();
		}
	}
	cout << "single insert cost (us): " << timer.Tock() << endl;

	timer.Tick();
	for (auto& it : vt0)
	{
		if (!acc.Delete(it.idx))
		{
			Trap();
		}
	}
	cout << "single delete cost (us): " << timer.Tock() << endl;
	
	cout << "**** Bulk test passed." << endl;
}

std::deque<Teacher> PrepareTeacher(const int count, const int StartIdx)
{
	deque<Teacher> vt0;
	int idx = StartIdx;
	int nAccount = MongoClib::knAccountPerPeople;
	for (int ii = 0; ii < count; ++ii)
	{
		Teacher obj;
		++idx;
		obj.idx = idx;
		obj.group = idx % 5;
		wstringstream wss;
		wss << L"name_中文_" << obj.idx << "_" << obj.group;
		obj.name = EncUtf8(wss.str());
		obj.wname = wss.str();

		if (!obj.wname.length() && obj.name.length())
		{
			cout << "DecAnsi failed. maybe bad loc string in Init()" << endl;
			Trap();
		}

		stringstream ss;
		ss.str("");
		ss << "student_" << idx << "_1";
		obj.students.push_back(ss.str());
		ss.str("");
		ss << "student_" << idx << "_2";
		obj.students.push_back(ss.str());
		ss.str("");
		ss << "student_" << idx << "_3";
		obj.students.push_back(ss.str());
		ss.str("");
		ss << "student_" << idx << "_4";
		obj.students.push_back(ss.str());

		for (int jj = 0; jj < nAccount; ++jj)
		{
			Account acc;
			acc.guid = idx + jj * 3;
			acc.name = "account";
			acc.memo = "memo";
			acc.inuse = (idx + jj) % 2 == 1;
			obj.accounts.push_back(acc);
		}

		const int binlen = 100;
		char bin[binlen] = { 0 };
		char cnt = (char)idx;
		for (int jj = 0; jj < binlen; ++jj)
		{
			bin[jj] = ++cnt;
		}
		obj.binData.assign(bin, binlen);
		vt0.push_back(obj);
	}

	return std::move(vt0);
}

void StressInsert(int& idx, const int count)
{
	cout << "current idx " << idx << endl;

	Timer timer;
	timer.Tick();

	deque<Teacher> vt0 = PrepareTeacher(count, idx);
	idx += (int)vt0.size();

	
	cout << "data prepare cost (us): " << timer.Tock() << endl;


	timer.Tick();
	if (!TeacherAccess().Insert2(vt0))
	{
		Trap();
	}
	cout << "bulk inserter cost (us): " << timer.Tock() << endl;
}

void ThreadStressInsert(int idx, const int count)
{
	StressInsert(idx, count);
}

void ConnTestStress(const std::wstring& srv, const std::wstring& dbname, const int count)
{
	if (MongoClib::Init(srv, dbname, 2000))
	{
		Trap();
	}

	if (!MongoClib::DropDataBase())
	{
		Trap();
	}

	TestStress(count);
}


void TestStress(const int count /*= 100000*/)
{
	/************************************************************************/
	/* test result:
	16 loop, 10W per loop, 16 core server
	16 thread : 143,273,140 us
	Loop cost 126,882,347 us	
	*/
	/************************************************************************/
	TeacherAccess acc;
	if (!acc.CreateComplexIndexs())
	{
		Trap();
	}

	cout << "loop count " << count << endl;

	Timer timer;
	locale loc(szLoc);
	cout.imbue(loc);

	deque<Teacher> vt0;

//	vt0 = PrepareTeacher(count);

	int nLoop = 4;

	cout << "Thread Loop " << nLoop << endl;

	timer.Tick();
	vector<thread> ths;
	for (int ii = 0; ii < nLoop; ++ii)
	{
		ths.push_back(thread(ThreadStressInsert, ii * count, count));
	}

	for (auto& th : ths)
	{
		th.join();
	}

	cout << "**** Thread cost " << timer.Tock() << endl;

	timer.Tick();
	int idx = nLoop * (int)vt0.size();
	for (int ii = 0; ii < nLoop; ++ii)
	{
		StressInsert(idx, count);
	}

	cout << "**** Loop cost " << timer.Tock() << endl;
}


void TestCombinUnique(void)
{
	std::string collName("TUnique");
	AutoPoolColl coll(collName);
	AutoBson key;
	key.Add("name", 1);
	key.Add("tag", 1);
	if (!coll.CreateIndex(key, true))
	{
		Trap();
	}

	AutoBson doc1;
	doc1.Add("name", "name1");
	doc1.Add("tag", 1);

	if (!coll.Insert(doc1))
	{
		Trap();
	}

	AutoBson doc2;
	doc2.Add("name", "name1");
	doc2.Add("tag", 2);

	if (!coll.Insert(doc2))
	{
		Trap();
	}
	
	AutoBson doc3;
	doc3.Add("name", "name1");
	doc3.Add("tag", 1);

	if (coll.Insert(doc3))
	{
		Trap();
	}
}


void TestBigDoc(void)
{
	string strColl("testbig");
	unsigned nLen = 20 * 1024 * 1024;

	unique_ptr<char, default_delete<char[]>> up(new char[nLen]);


	memset(up.get(), 0x5a, nLen);

	string bin(up.get(), nLen);
	if (bin.length() != nLen)
	{
		Trap();
	}
		
	AutoBson doc;
	doc.Add("name", "test");
	doc.AddBin("bin", bin);

	AutoPoolColl coll(strColl);

	if (!coll.Insert(doc))
	{
		cout << "insert failed: " << coll.Error() << endl;
	}

	AutoBson query;
	AutoCursor cursor(strColl, query);

	while (cursor.Next())
	{
		string name = cursor.Str("name");
		string data = cursor.Bin("bin");
		if (data != bin)
		{
			Trap();
		}
	}

	cout << "**** TestBigDoc finished " << endl;
}


int ThreadTestPerf(int idx, int nDocCount)
{
	for (int ii = 0; ii < nDocCount; ++ii)
	{
		stringstream ss;
		ss << "thread-" << idx << ": count " << ii << endl;
		cout << ss.str();
	}
	stringstream ss;
	ss << "thread-" << idx << ": exit." << endl;
	cout << ss.str();
	return 10;
}

void TestPerf(void)
{
	vector<thread> threads;

	int nThread = 10;
	const int nDoc = 100;
	for (int ii = 0; ii < nThread; ++ii)
	{
		threads.push_back(thread(ThreadTestPerf, ii, nDoc));
	}

	for (int ii = 0; ii < nThread; ++ii)
	{
		threads[ii].join();
	}

	cout << "**** TestPerf finished " << endl;
}


void TestBit(void)
{
	AutoBson doc;
	doc.Add("title", "name1");
	doc.Add("float", 23.8);
	doc.Add("int", 8);

	std::string collName("TBit");
	AutoPoolColl coll(collName);

	coll.Insert(doc);

	AutoBson query;
	query.Add("title", "name1");

	/*
	8    		1000
	or 5 = 13	0101	1101
	and 7 = 5		0111	0101
	xor 12 = 9			1100	1001
	*/

	BsonCmd cmd;
	cmd.BitOr("int", 5);

	coll.Update(query, cmd, MONGOC_UPDATE_UPSERT);


	BsonCmd cmd2;
	cmd2.BitAnd("int", 7);

	coll.Update(query, cmd2, MONGOC_UPDATE_UPSERT);
	BsonCmd cmd3;
	cmd3.BitXor("int", 12);

	coll.Update(query, cmd3, MONGOC_UPDATE_UPSERT);
}

void TestTimet(void)
{
	AutoBson query;
	AutoCursor cursor("test", query);

	while (cursor.Next())
	{
		auto tmAssign = cursor.Timet("tmAssign");
		cout << tmAssign << endl;
	}
}

void TestDateTime(void)
{


	AutoBson doc;
	doc.Add("name", "name1");
	long long t = 1;
	doc.Add("long long", t);
	doc.AddTime("lt1", t);
	doc.AddTime("lt2", time(nullptr));
	doc.AddTime("create", 1234);	
	doc.AddTime("update");

	string key("time");

	bson_append_time_t(doc, key.c_str(), (int)key.length(), 1234);
		
	AutoJson json(doc);

	string s = json;

	std::string collName("TDate");
	AutoPoolColl coll(collName);

	coll.Insert(doc);

	AutoBson query;
	AutoCursor cursor(collName, query);

	while (cursor.Next())
	{
		string name = cursor.Str("name");
		auto d = cursor.ResultBson();
		bson_iter_t ival;

		if (BsonKeyIter(d, "create", ival) && BSON_ITER_HOLDS_DATE_TIME(&ival))
		{
			auto a = bson_iter_date_time(&ival);
			auto b = bson_iter_time_t(&ival);

			if (a != b)
			{
				cout << "date_time & time_t different" << endl;
			}
		}

		auto tt = cursor.Timet("create");
		cout << "craete: " << tt << endl;

		if (cursor.HasKey("time"))
		{
			if (BsonKeyIter(d, "time", ival) && BSON_ITER_HOLDS_DATE_TIME(&ival))
			{
				auto a = bson_iter_date_time(&ival);
				auto b = bson_iter_time_t(&ival);
				if (a != b)
				{
					cout << "date_time & time_t different" << endl;
				}
			}
		}

		if (!cursor.HasKey("test"))
		{
			cout << "do not has key test" << endl;
		}


	}

	cout << "**** TestDateTime finished " << endl;
}


void TestSetOr(void)
{
	// test query doc : group = 2 & idx > 100100
	BsonCmd bcquery;
	bcquery.Comp("idx", BsonCmd::CTgte, 100100);
	bcquery.Comp("group", BsonCmd::CTeq, 2);

	AutoJson jjj(bcquery);

	// test move 
	BsonCmd bc1;
	bc1.Set("a", 12);

	AutoJson j(bc1);

	BsonCmd bc2;
	bc2 = std::move(bc1);

	AutoJson j2(bc2);
	AutoJson j3(bc1);



	std::string collName("TDate");
	AutoPoolColl coll(collName);



	AutoBson doc;
	doc.Add("name", "name1");
	long t = 5;
	doc.Add("int", t);
	coll.Insert(doc);


	AutoBson doc2;
	doc2.Add("name", "name2");
	doc2.Add("int", t);
	coll.Insert(doc2);
	
	
	AutoBson query;
	query.Add("name", "name1");


	AutoBson query2;
	query2.Add("name", "name3");

	BsonCmd update;
	update.Set("setkey", "setval");
	update.BitOr("int", 9);

	coll.Update(query, update);
	coll.Update(query2, update);
	coll.Update(query2, update, MONGOC_UPDATE_UPSERT);

	BsonCmd update2;
	update2.Add("setkey", "setval");
	coll.Update(query2, update2, MONGOC_UPDATE_UPSERT);
	



	cout << "**** TestSetOr finished " << endl;
}


AutoBson TestBsonArray(void)
{
	AutoBson doc;
	doc.Add("asdf", 123);

	ArrayBuilder ar1;
	ar1.ArrAdd("s1");
	ar1.ArrAdd("s2");
	ar1.ArrAdd("s3");

	doc.AddArray("ar_s", ar1);

	ArrayBuilder ar2;
	ar2.ArrAddTime(12341);
	ar2.ArrAddTime(12342);
	ar2.ArrAddTime(12343);
	doc.AddArray("ar_t", ar2);


	string s("0000");
	ArrayBuilder ar3;
	s[1] = 1;
	ar3.ArrAddBin(s);
	s[1] = 2;
	ar3.ArrAddBin(s);
	s[1] = 3;
	ar3.ArrAddBin(s);
	doc.AddArray("ar_b", ar3);

	ArrayBuilder ar4;
	AutoBson d4;
	d4.Add("key1", "val1");
	d4.Add("key2", "val2");
	ar4.ArrAddDoc(d4);
	AutoBson d41;
	d41.Add("key11", "val11");
	d41.Add("key21", "val21");
	ar4.ArrAddDoc(d41);
	doc.AddArray("ar_d", ar4);


	ArrayBuilder ar5;
	ar5.ArrAddArr(ar1);
	ar5.ArrAddArr(ar2);
	ar5.ArrAddArr(ar3);
	ar5.ArrAddArr(ar4);
	doc.AddArray("ar_a", ar5);

	AutoJson j(doc);
	cout << j << endl;

	return std::move(doc);
}

void TestArrayParser(AutoBson& doc)
{
	vector<string> vb;
	if (!BsonBinContainer(doc, "ar_b", vb))
	{
		Trap();
	}

	vector<string> vs;
	if (!BsonGetContainer(doc, "ar_s", vs, str2string))
	{
		Trap();
	}

}


void TestArray(void)
{
	std::string collName("TDate");
	AutoPoolColl coll(collName);
	

	AutoBson doc;
	doc.Add("name", "name1");
	long t = 5;
	doc.Add("int", t);

	int ni = 3;
	ArrayBuilder docarray2;
	for (int ii = 0; ii < ni; ++ii)
	{
		docarray2.AddVal(100 + ii);
	}
	doc.AddArray("w_vtFileGenders", docarray2);

	coll.Insert(doc);

	coll.Insert(TestBsonArray());

	AutoBson query;
	query.Add("name", "name1");

	BsonCmd update;
	ArrayBuilder docarray;
	for (int ii = 0; ii < ni; ++ii)
	{
		docarray.AddVal(200 + ii);
	}
	update.SetArray("w_vtFileGenders", docarray);

	coll.Modify(query, update);
}

std::tuple<int, std::string> GetFlagOper(void)
{
	AutoBson query;
	query.Add("w_strGuid", "923#E09EF2A2-80A9-E6E4-DFA7-611F60C3E194");

	AutoPoolColl coll("TRecord");

	int flag = 0;
	string oper;

	AutoCursor cursor("TRecord", query);
//	AutoCursor cursor = coll.Find(query);
	if (cursor.Next())
	{
		flag = cursor.Int32("w_nProcessModeBits");
		oper = cursor.Str("w_strOperSysID");
	}

	return make_tuple(flag, oper);
}

void TestTuple(void)
{
	auto res = GetFlagOper();

	int flag = std::get<0>(res);
	string oper = std::get<1>(res);

	int f2;
	string s2;

	std::tie(f2, s2) = res;


	cout << flag << oper << endl;
}

vector<int> GenAngle(int n)
{
	vector<int> v;
	if (n <= 1)
	{
		v.assign(n, 1);
		return v;
	}
	v.assign(1, 1);
	auto v0 = v;
	while (v.size() < (unsigned)n)
	{
		for (size_t ii = 1, ni = v0.size(); ii < ni; ++ii)
		{
			v[ii] = v0[ii] + v0[ii - 1];
		}
		v.push_back(1);
		v0 = v;
	}
	return v;
}

vector<int> GenAngle2(int n)
{
	vector<int> v;
	if (n <= 1)
	{
		v.assign(n, 1);
		return v;
	}
	auto vn1 = GenAngle2(n - 1);
	v = vn1;
	for (size_t ii = 1, ni = vn1.size(); ii < ni; ++ii)
	{
		v[ii] = vn1[ii] + vn1[ii - 1];
	}
	v.push_back(1);
	return v;
}

void TestTriAngle(void)
{
	int ni = 11;
	for (int ii = 0; ii < ni; ++ii)
	{
		auto v = GenAngle(ii);
		for (auto i : v)
		{
			cout << i << " ";
		}
		cout << endl;
	}

	cout << endl;


	for (int ii = 0; ii < ni; ++ii)
	{
		auto v = GenAngle2(ii);
		for (auto i : v)
		{
			cout << i << " ";
		}
		cout << endl;
	}
	
}

using namespace chrono;

auto g_tpDoze = system_clock::now();
void Doze(const int millisec = 1000)
{
	cout << "doze " << millisec << " milliseconds ...";
	auto span = milliseconds(millisec);
	g_tpDoze += span;
	this_thread::sleep_until(g_tpDoze);
	cout << " done." << endl;
}

std::string TimetTo8BitHexStr(time_t tm)
{
	int32_t t = (int)tm;
	stringstream ss;
	ss << hex << t;
// 	auto s = ss.str();
// 	transform(s.begin(), s.end(), s.begin(), toupper);
// 	s = string("0x") + s;
	return ss.str();
}

void InsertOidWithTime(void)
{
	int64_t idx = 345930;
	int insertOnce = 10;
	while (true)
	{
		auto tmGen = time(nullptr);
		auto tmStru = localtime(&tmGen);
		char buffer[80];
		strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", tmStru);
		cout << buffer << endl;

		auto tmHex = TimetTo8BitHexStr(tmGen);

		OidTimeAccess acc;

		for (int ii = 0; ii < insertOnce; ++ii)
		{
			OidTime obj;
			obj.idx = ++idx;
			obj.tmGen = tmGen;
			obj.str = buffer;
			obj.tmHex = tmHex;
			acc.Insert(obj);
		}		

		Doze();
	}
}


bool Timet2Oid(time_t t, bson_oid_t& oid)
{
	uint32_t u = BSON_UINT32_TO_BE(t);
	memset(&oid, 0, sizeof(oid));
	memcpy(&oid, &u, sizeof(u));
	return true;
}

bool Timet2Oid2(time_t t, bson_oid_t& oid)
{
	auto h1 = high_resolution_clock::now();
	auto s1 = TimetTo8BitHexStr(t);
	string s2(24 - 8, '0');
	s1 += s2;
	if (!bson_oid_is_valid(s1.c_str(), s1.length()))
	{
		return false;
	}
	bson_oid_init_from_string(&oid, s1.c_str());
	auto h2 = high_resolution_clock::now();

	bson_oid_t oid2;

	auto h3 = high_resolution_clock::now();
	uint32_t u = BSON_UINT32_TO_BE(t);
	;
	memset(&oid2, 0, sizeof(oid2));
	memcpy(&oid2, &u, sizeof(u));

	auto h4 = high_resolution_clock::now();

	auto d1 = h2 - h1;
	auto d2 = h4 - h3;
	cout << d1.count() << endl;
	cout << d2.count() << endl;

	auto n = bson_oid_compare(&oid, &oid2);

	cout << n << endl;
	return true;
}

std::int64_t ReadData(time_t t1, time_t t2)
{
	bson_oid_t oid1;
	bson_oid_t oid2;
	Timet2Oid(t1, oid1);
	Timet2Oid(t2, oid2);

	deque<OidTime> vt;
	deque<OidTime> vt2;
	OidTimeAccess().LoadCond(vt, oid1, oid2);
	OidTimeAccess().LoadCond(vt2, t1, t2);
	for (auto i : vt)
	{
		cout << i.ToString() << endl;
	}
	cout << endl;
	return vt.size();
}

void i2a(void)
{
	char str[16];
	const char *key;
	uint32_t i;

	for (i = 0; i < 10; i++) {
		bson_uint32_to_string(i, &key, str, sizeof str);
		printf("Key: %s\n", key);
	}
}

void TestOidTime(void)
{
	bson_oid_t oid;
	auto t = time(nullptr);
	Timet2Oid(t, oid);
	auto n1 = bson_oid_hash(&oid);
	
	// read data with oid by time_t
	auto start = time(nullptr);
	auto offset = 60;
	auto sleepsec = 5;
	while (true)
	{
		cout << Timet2String(start) << endl;
		auto n = ReadData(start - offset, start - offset + sleepsec);
		cout << Timet2String(start) << " " << n << " records" << endl;
		Doze(sleepsec * 1000);
		start += sleepsec;
	}
}



void TestOidClass(void)
{
	AutoBson q;
	
	AutoCursor cursor("test230", q);
	if (!cursor)
	{
	}
	if (!cursor.Next())
	{
		// bool mongoc_cursor_error(mongoc_cursor_t *cursor, bson_error_t *error);
		bson_error_t e = {};
		auto b = mongoc_cursor_error(cursor.Cursor(), &e);

		auto s = e.message;

	}



	TestOidTime();
	BsonCmd cmd;

	auto tm1 = time(nullptr);
	auto tm2 = tm1 + 16;

	AutoOid oid1(tm1);
	AutoOid oid2(tm2);
	AutoOid oid3(oid2);

	cout << oid2.ToString() << endl;
	cout << oid3.ToString() << endl;
	cout << oid3.HexTimeString() << endl;

	AutoOid oid4(std::move(oid3));

	string key = "_id";
	cmd.Comp(key, BsonCmd::CTgte, oid1);
	cmd.Comp(key, BsonCmd::CTlt , oid2);


	bson_oid_t oid;
	Timet2Oid(tm2, oid);

	bson_oid_t oid11(oid);

	AutoOid oid6;
	oid6 = oid3;
	

	AutoJson j(cmd);

	// { "_id" : { "$gte" : { "$oid" : "596db4440000000000000000" } }, "_id" : { "$lt" : { "$oid" : "596db4540000000000000000" } } }

}


void TestRename(void)
{
	AutoPoolColl coll("OidTime");
	auto b = coll.Rename("OidTime");
	auto e = coll.Error();
}

void TestParseNoFail(void)
{
	AutoPoolColl coll("TRecord");
	AutoBson query;
	AutoCursor cursor("TRecord", query);

	while (cursor.Next())
	{
		auto _id = cursor.Oid();
		auto t = cursor.OidTime();
		auto guid = cursor.Str("guid");
		auto st = cursor.Timet("startTime");
		auto score = cursor.Double("score");
		auto score2 = cursor.Double("score", false);

		auto f = cursor.IsFailed();
	}
}

void TestLogColl(void)
{
// 	MLog(MLCTeacher, "enroll", "teacher 01");
// 	MLog(MLCTeacher, "enroll", "teacher 02");
// 	MLog(MLCStudent, "up", "stu 01");
// 	MLog(MLCStudent, "down", "stu 02");
// 	MLog(MLCStudent, "left", "stu 01");
// 	MLog(MLCStudent, "right", "stu 01");


// 	MLog(mlDef.teacher, MLLWarn, mlDef.teacher.enroll, "teacher_01");
// 	MLog(mlDef.teacher, MLLWarn, mlDef.teacher.out, "teacher_01");
// 	MLog(mlDef.teacher, MLLWarn, mlDef.teacher.enroll, "teacher_02");
// 	MLog(mlDef.teacher, MLLDetail, mlDef.teacher.tStart, "teacher_02");
// 	MLog(mlDef.teacher, MLLDetail, mlDef.teacher.tEnd, "teacher_02");

	mlDef.teacher.Enroll("teacher01");
	mlDef.teacher.Enroll("teacher02");
	mlDef.teacher.Out("teacher01");
	mlDef.teacher.Start("teacher02");
	mlDef.teacher.End("teacher02");
	mlDef.teacher.WillAutoGenKeyByFuncName("test");
}

void TestBsonFromJson(void)
{
	/*
	bson_t *
bson_new_from_json (const uint8_t *data, ssize_t len, bson_error_t *error);
	*/
	string s = "{ \"key\":\"value\" }";
	bson_error_t e;
	auto p = bson_new_from_json((uint8_t*)s.c_str(), -1, &e);
	Trap(p);
	AutoBson b(p);
	AutoJson j(b);
	cout << j << endl;
}

void TestBsonReader(void)
{
	char* fnJson = nullptr;
	fnJson = "BsonReaderTest.json";
	fnJson = "BsonReaderTestExtend.json";	// mongoAddr : "192.168.13.238"
	fnJson = "BsonReaderTestComma.json";	// {"xxx" : "xxx" , }
	fnJson = "BsonReaderTestSpace.json";	// "mongoAddr" : "192.168.13.238"  // (pass with 1.2.1)

	/*
	good: 
		"mongoAddr" : "192.168.13.238",
	bad:
		mongoAddr : "192.168.13.238",
	bad: TRAILING comma
		{"xxx" : "xxx" , }
		     error here^
	*/

	cout << "json file name: " << fnJson << endl;

	// test & read file
	fstream f(fnJson, ios::in);
	if (!f)
	{
		abort();
	}
	string s;
	while (getline(f, s))
	{
		cout << s << endl;
	}

	f.close();


	// read and conv to bson
	bson_json_reader_t *reader;
	bson_error_t error;
	reader = bson_json_reader_new_from_file(fnJson, &error);
	Trap(reader);
// 	StackBson doc;
	bson_t doc = BSON_INITIALIZER;

	auto p = bson_new();

	auto ret = 0;
	while ((ret = bson_json_reader_read(reader, &doc , &error)) > 0)
	{
		cout << endl << "Json read ok: " << endl;
		AutoJson j(&doc);
		cout << j << endl;
		
		cout << endl << "***PARSE***" << endl;
		AutoBson ab(doc);
		cout << ab.Int32("NotExist") << endl;
		cout << ab.Str("mongoAddr") << endl;
		cout << ab.Int32("mongoPort") << endl;
		cout << ab.Str("mongoColl") << endl;
		cout << ab.Str("sidModelPath") << endl;
		cout << ab.Str("resPath") << endl;
		cout << "***END***" << endl;
	}
	if (ret < 0)
	{
		cout << "bson_json_reader_read failed:" << endl;
		cout << error.message << endl;
	}
	
	bson_json_reader_destroy (reader);  
	bson_destroy (&doc); // can be ignored.

	cout << endl << "ReadJsonFile: " << endl;
	std::vector<AutoBson> v;
	std::tie(v,s) = ReadJsonFile(fnJson);
	for (auto& d : v)
	{
		AutoJson j(d);
		cout << j << endl << endl;
	}

	cout << endl << "ReadJsonFileFirst: " << endl;
	AutoBson d;
	std::tie(d,s) = ReadJsonFileFirst(fnJson);
	AutoJson j(d);
	cout << j << endl;




	cout << endl << "ReadJsonFileLast: " << endl;
	AutoBson d2;
	std::tie(d2, s) = ReadJsonFileLast(fnJson);
	AutoJson j2(d2);
	cout << j2 << endl;
	cout << "appendDoc: " << d2.Str("appendDoc") << endl;
}
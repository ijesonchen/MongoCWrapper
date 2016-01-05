#include "mfcafx.h"


#include "TestFunc.h"

#include <iostream>
#include <chrono>
#include <sstream>
#include <vector>
#include <fstream>
#include <memory>

#include <mongoc.h>

#include "MongoAuto.h"
#include "MongocHelp.h"
#include "MongoClient.h"

#include "TeacherAccess.h"

#ifdef __AFX_DEBUG_NEW__
	#include "StudentAccess.h"
#endif // __AFX_DEBUG_NEW__

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



const wchar_t* wszSrv = L"192.168.1.120";
const wchar_t* wszDb = L"mytestdb";


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
// 	int nRet = Init(L"192.168.1.128", L"mydb", "chs");
// 
// 	if (nRet)
// 	{
// 		Trap();
// 	}
// 
// 	vector<mongoc_client_t*> vtClient;
// 	mongoc_client_t* client = TryPopClient();
// 
// 	if (!client)
// 	{
// 		Trap();
// 	}
// 
// 	AutoColl collection(mongoc_client_get_collection(client, "mydb", "mycoll"));
// 	
// 	nRet = CreateIndex(client, "mycoll", "name");
// 
// 
// 	PushClient(client);
}


void TestConn(void)
{
	int nRet = Init(L"192.168.1.120", L"mydb", "chs", 2000, 2000, 2000);
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
	int nRet = Init(L"192.168.1.120", L"mydb",  "chs", 2000, 2000, 2000);
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
// 	bson_error_t error;
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
	static int nCounter = 100000;

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

	Timer timer;
	locale loc(szLoc);
	cout.imbue(loc);

	timer.Tick();	
	if (!acc.Insert(vt0))
	{
		Trap();
	}	
	cout << "bulk inserter cost (us): " << timer.Tock() << endl;

	timer.Tick();
	if (!acc.ResetName())
	{
		Trap();
	}
	cout << "ResetName cost (us): " << timer.Tock() << endl;

	timer.Tick();
	if (!acc.Update(vt0))
	{
		Trap();
	}
	cout << "bulk Update cost (us): " << timer.Tock() << endl;

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

	cout << "**** Bulk test passed." << endl;
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
#include "stdafx.h"
#include "mfcafx.h"
#include "TestMfcFunc.h"

#include <iostream>
#include <chrono>
#include <sstream>
#include <vector>
#include <fstream>
#include <memory>
#include <thread>

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

struct SRecordAssist
{
	CString w_strGuid;
	int     w_nAssignState;//与TRecord中的状态保持一致
	int		w_nPriority;
	long long w_nAssignTime; // 分配时间，UpSert时自动生成
	SRecordAssist() : w_nAssignState(0), w_nAssignTime(0), w_nPriority(2){}
};

bool ParseBson(SRecordAssist& obj, const MongoClib::AutoCursor& cursor)
{
	obj.w_strGuid = cursor.CStr("w_strGuid");
	obj.w_nAssignState = cursor.Int32("w_nAssignState");
	obj.w_nPriority = cursor.Int32("w_nPriority");
	obj.w_nAssignTime = cursor.Timet("w_nAssignTime");
	if (cursor.IsFailed())
	{
		Trap();
	}
	return true;
}

void TestMfc(void)
{

	time_t tm041400 = 1460563200;
	time_t tm041500 = tm041400 + 3600 * 24;
	time_t tm041512 = tm041500 + 3600 * 12;

	// 1460563200 = ISODate("2016-04-14T00:00:00.000+08:00")

	// 1460709112

	/*
	
	db.getCollection('TRecord').find({
	"$and" : [
	{"w_nTmVprFinish" : {"$gte" : ISODate("2016-04-14T00:00:00.000+08:00")}},
	{"w_strLang" : "汉语"},
	{"w_nProcessModeBits" : 127}
	]
	}).count()
	
	*/
	BsonCmd b1;
	b1.CompTime("w_nTmVprFinish", BsonCmd::CTlt, tm041400);
	BsonCmd b2;
	string strmand = EncUtf8(L"汉语");
	b2.Add("w_strLang", strmand);
	BsonCmd b3;
	b3.Add("w_nProcessModeBits", 127);
// 	BsonCmd b4;
// 	b4.Exists("w_nTmTransFinish", false);

	ArrayBuilder ar;
	ar.AddSub(b1);
	ar.AddSub(b2);
	ar.AddSub(b3);
//	ar.AddSub(b4);

	BsonCmd bq;
	bq.AddArray("$and", ar);

	AutoPoolColl coll("TRecord");

// 	int n = coll.Count(bq);
// 	cout << "count : " << n << endl;
// 	return;

	AutoCursor cursor = coll.Find(bq);
	
	AutoPoolColl collk("TPretreatKwd");
	int statek = 33;
	AutoPoolColl collt("TPretreatTrans");
	int statet = 65;

// 	"w_strGuid" : "923#12D654A5-4DBA-6397-C546-A0BF60C301DC",
// 	"w_nAssignState" : 65,
// 	"w_nPriority" : 1,
// 	"w_nAssignTime" : ISODate("2016-04-11T16:10:50.000+08:00"),

	time_t tmAssign = tm041400;
	while (cursor.Next())
	{
		auto guid = cursor.Str("w_strGuid");
		auto priority = cursor.Int32("w_nPriority");
		AutoBson dock;
		AutoBson doct;
		dock.Add("w_strGuid", guid);
		dock.Add("w_nAssignState", statek);
		dock.Add("w_nPriority", priority);
		dock.AddTime("w_nAssignTime", ++tmAssign);
		dock.AddTime("tmInsertTime");
		collk.Insert(dock);

		doct.Add("w_strGuid", guid);
		doct.Add("w_nAssignState", statet);
		doct.Add("w_nPriority", priority);
		doct.AddTime("w_nAssignTime", ++tmAssign);
		doct.AddTime("tmInsertTime");
		collt.Insert(doct);
	}
	cout << "count : " << tmAssign - tm041400 << endl;

	
// 
// 	vector<string> guidv1;
// 	vector<string> guidv2;
// 	vector<string> guidv3;
// 	
// 	BsonCmd condv1;
// 	condv1.Exists("bEnd", false);
// 	condv1.CompTime("tmStartTime", BsonCmd::CTlte, tmNow - 180);
// 
// 	AutoJson jcond(condv1);
// 		
// 
// 
// 	AutoCursor cursor = coll.Find(condv1);
// 	while (cursor.Next())
// 	{
// 		string guid = cursor.Str("w_strGuid");
// 		guidv1.push_back(guid);
// 	}
// 	
// 	BsonCmd condv2;
// 	condv2.Exists("bEnd");
// 
// 	AutoCursor cursor2 = coll.Find(condv2);
// 	while (cursor2.Next())
// 	{
// 		string guid = cursor2.Str("w_strGuid");
// 		guidv2.push_back(guid);
// 	}
// 
// 	BsonCmd condv1v2;
// 	condv1v2.AndOr(BsonCmd::CTor, condv1, condv2);
// 
// 	AutoCursor cursor3 = coll.Find(condv1v2);
// 	while (cursor3.Next())
// 	{
// 		string guid = cursor3.Str("w_strGuid");
// 		guidv3.push_back(guid);
// 	}
// 
// 	auto b = coll.Delete(condv1v2);
//
// 	AutoBson queryall;
// 	queryall.AddTime("tmStartTime", 1460361936);
// 
// 
// 	AutoJson jquall(queryall);
// 
// 	AutoCursor cursor2 = coll.Find(queryall);
// 	while (cursor2.Next())
// 	{
// 		string guid = cursor2.Str("w_strGuid");
// 		time_t tm = cursor2.Timet("tmStartTime");
// 		guids.push_back(guid);
// 	}
	


	return;
}
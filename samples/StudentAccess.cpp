#include "stdafx.h"
#include "StudentAccess.h"
#include <iostream>

#include "..\src\MongocHelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace MongoClib;


StudentAccess::StudentAccess()
	:AccessBase("Student")
{
}


StudentAccess::~StudentAccess()
{
}

AutoBson StudentAccess::BuildBson(const Student& obj)
{
	AutoBson doc;
	if (!doc) { return nullptr; }

	doc.Add("strName", obj.strName);
	doc.Add("strMemo", obj.strMemo);
	doc.Add("bValue", obj.bValue);
	doc.Add("nValue", obj.nValue);
	doc.Add("fValue", obj.fValue);
	doc.Add("vtText", obj.vtText, cstring2str);
	if (!AddAccount(doc, "vtAccout", obj.vtAccout)) { return nullptr; }

	if (doc.IsFailed()) { return nullptr; }

	return std::move(doc);
}

bool StudentAccess::ParseBson(Student& obj, const MongoClib::AutoCursor& cursor)
{
	obj.strName = cursor.CStr("strName");
	obj.strMemo = cursor.CStr("strMemo");
	obj.bValue = cursor.Bool("bValue");
	obj.nValue = cursor.Int32("nValue");
	obj.fValue = static_cast<float>(cursor.Double("fValue"));
	obj.vtText = cursor.GetVecCStr("vtText");
	obj.vtAccout = GetAccount(cursor, "vtAccout");
	if (cursor.IsFailed())
	{
		cout << "ParseBson failed: " << cursor.Message() << endl;
		return false;
	}
	return true;
}

bool StudentAccess::QueryUpdate(const CString& name, const bson_t* update)
{
	AutoBson query;
	query.Add("strName", name);
	return Modify(query, update);
}

bool StudentAccess::Delete(const CString& name)
{
	AutoBson query;
	query.Add("strName", name);
	return Remove(query);
}

bool StudentAccess::Update(const Student& obj)
{
	AutoBson cond;
	cond = BuildBson(obj);
	if (!cond) { return false; }

	BsonCmd update;
	update.Set(cond);


	return QueryUpdate(obj.strName, update);
}

bool StudentAccess::UpdateBool(const CString& name, bool b)
{
	BsonCmd update;
	update.Set("bValue", b);
	return QueryUpdate(name, update);
}

bool StudentAccess::LoadOne(const CString& name, Student& obj)
{
	AutoBson query;
	query.Add("strName", name);
	return QueryOne(obj, query);
}

bool StudentAccess::LoadAll(std::deque<Student>& dqObjects)
{
	return QueryLoad(dqObjects);
}

bool StudentAccess::LoadCond(std::deque<Student>& dqStudents, const int n)
{
	AutoBson query;
	query.Add("nValue", n);
	return QueryLoad(dqStudents, query);
}

int  StudentAccess::LoadState(const CString& name)
{
	Student obj;
	if (LoadOne(name, obj))
	{
		// return default value
		return 0;
	}

	return obj.nValue;
}


bool StudentAccess::CreateIndexs(void)
{
	AutoPoolColl coll(m_collname);
	if (!coll.CreateUniqueIndex("strName")) return false;
	if (!coll.CreateIndex("nValue")) return false;
	if (!coll.CreateIndex("fValue")) return false;
	return true;
}
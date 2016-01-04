#ifdef _MSC_VER
#pragma warning(disable: 4324)
#endif // _MSC_VER

#include "TeacherAccess.h"
#include <iostream>

using namespace std;
using namespace MongoClib;

TeacherAccess::TeacherAccess()
	: AccessBase("Teacher")
{
}


TeacherAccess::~TeacherAccess()
{
}

AutoBson TeacherAccess::BuildBson(const Teacher& obj)
{
	AutoBson doc;
	if (!doc) { return nullptr; }

	doc.Add("idx", obj.idx);
	doc.Add("group", obj.group);
	doc.Add("name", obj.name);
	doc.Add("wname", obj.wname);
	doc.Add("students", obj.students, str2string);
	if (!AddAccount(doc, "accounts", obj.accounts)) { return nullptr; }
	doc.AddBin("binData", obj.binData);

	if (doc.IsFailed()) { return nullptr; }

	return std::move(doc);
}

bool TeacherAccess::ParseBson(Teacher& obj, const MongoClib::AutoCursor& cursor)
{
	obj.idx = cursor.Int32("idx");
	obj.group = cursor.Int32("group");
	obj.name = cursor.Str("name");
	obj.wname = cursor.WStr("wname");
	if (!cursor.GetCntr("students", obj.students, str2string)) { return false; }
	obj.accounts = GetAccount(cursor, "accounts");
	obj.binData = cursor.Bin("binData");

	if (cursor.IsFailed())
	{
		cout << "ParseBson failed: " << cursor.Message() << endl;
		return false;
	}
	return true;
}

bool TeacherAccess::QueryUpdate(const int idx, const bson_t* update)
{
	AutoBson query;
	query.Add("idx", idx);
	return Modify(query, update);
}

bool TeacherAccess::Delete(const int idx)
{
	AutoBson query;
	query.Add("idx", idx);
	return Remove(query);
}

bool TeacherAccess::Update(const Teacher& obj)
{
	AutoBson cond;
	cond = BuildBson(obj);
	if (!cond) { return false; }

	BsonCmd update;
	update.Set(cond);


	return QueryUpdate(obj.idx, update);
}

bool TeacherAccess::UpdateBin(const int idx, const std::string& bin)
{
	AutoBson cond;
	cond.AddBin("binData", bin);
	BsonCmd update;
	update.Set(cond);
	return QueryUpdate(idx, update);
}

bool TeacherAccess::LoadOne(const int idx, Teacher& obj)
{
	AutoBson query;
	query.Add("idx", idx);
	return QueryOne(obj, query);
}

bool TeacherAccess::LoadAll(std::deque<Teacher>& dqObjects)
{
	return QueryLoad(dqObjects);
}

bool TeacherAccess::LoadCond(std::deque<Teacher>& dqTeachers, const int group)
{
	AutoBson query;
	query.Add("group", group);
	return QueryLoad(dqTeachers, query);
}

std::string TeacherAccess::LoadBinData(const int idx)
{
	Teacher obj;
	if (!LoadOne(idx, obj))
	{
		// return default value
		return "";
	}

	return obj.binData;
}

bool TeacherAccess::CreateIndexs(void)
{
	AutoPoolColl coll(m_collname);
	if (!coll.CreateUniqueIndex("idx")) return false;
	if (!coll.CreateIndex("group")) return false;
	if (!coll.CreateIndex("name")) return false;
	return true;
}

bool TeacherAccess::CreateIndexsNoUnique(void)
{
	AutoPoolColl coll(m_collname);
	if (!coll.CreateIndex("idx")) return false;
	if (!coll.CreateIndex("group")) return false;
	if (!coll.CreateIndex("name")) return false;
	return true;
}

template<typename Container>
bool TeacherAccess::Insert(const Container& cntr)
{
	BulkOperator bulk(m_collname);
	if (!bulk.IsValid())
	{
		return false;
	}

	for (auto it : cntr)
	{
		AutoBson doc = BuildBson(it);
		bulk.Insert(doc);
	}

	auto nRet = bulk.Execute();
	if (!nRet)
	{
		cout << "Bulk error: " << bulk.Error();
		return false;
	}
	cout << "Inserted: " << bulk.nInserted() << endl;
	return true;
}

template
bool TeacherAccess::Insert(const std::deque<Teacher>& cntr);

template 
bool TeacherAccess::Insert(const std::vector<Teacher>& cntr);

template 
bool TeacherAccess::Insert(const std::list<Teacher>& cntr);


bool TeacherAccess::Delete(const std::deque<Teacher>& dqObjects)
{
	BulkOperator bulk(m_collname);
	if (!bulk.IsValid())
	{
		return false;
	}

	for (auto it : dqObjects)
	{
		AutoBson doc;
		doc.Add("idx", it.idx);
		bulk.Remove(doc);
	}

	auto nRet = bulk.Execute();
	if (!nRet)
	{
		cout << "Bulk error: " << bulk.Error();
		return false;
	}
	cout << "nRemoved: " << bulk.nRemoved() << endl;
	return true;
}


bool TeacherAccess::Update(const std::deque<Teacher>& dqObjects)
{
	BulkOperator bulk(m_collname);
	if (!bulk.IsValid())
	{
		return false;
	}

	for (auto it : dqObjects)
	{
		AutoBson query;
		query.Add("idx", it.idx);

		BsonCmd update;
		update.Set("name", it.name);
		bulk.Update(query, update);
	}

	auto nRet = bulk.Execute();
	if (!nRet)
	{
		cout << "Bulk error: " << bulk.Error();
		return false;
	}
	cout << "nModified: " << bulk.nModified() << endl;
	return true;
}
bool TeacherAccess::Replace(const std::deque<Teacher>& dqObjects)
{
	BulkOperator bulk(m_collname);
	if (!bulk.IsValid())
	{
		return false;
	}

	for (auto it : dqObjects)
	{
		AutoBson query;
		query.Add("idx", it.idx);

		AutoBson doc;
		doc.Add("idx", it.idx);
		doc.Add("name", it.name);
		bulk.ReplaceOne(query, doc);
	}

	auto nRet = bulk.Execute();
	if (!nRet)
	{
		cout << "Bulk error: " << bulk.Error();
		return false;
	}
	cout << "nModified: " << bulk.nModified() << endl;
	return true;
}

// reset all name, test purpose only
bool TeacherAccess::ResetName(void)
{
	BsonCmd update;
	update.Set("name", "untitled");

	AutoBson query;
	return MultiUpdate(query, update);
}

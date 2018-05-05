#include "OidTimeAccess.h"
#include <iostream>

#include "..\src\MongocHelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;
using namespace MongoClib;


OidTimeAccess::OidTimeAccess()
	:AccessBase("OidTime")
{
}


OidTimeAccess::~OidTimeAccess()
{
}

AutoBson OidTimeAccess::BuildBson(const OidTime& obj)
{
	AutoBson doc;
	if (!doc) { return nullptr; }

	doc.Add(field.idx, obj.idx);
	doc.AddTime(field.tmGen, obj.tmGen);
	doc.Add(field.str, obj.str);
	doc.Add(field.tmHex, obj.tmHex);

	if (doc.IsFailed()) { return nullptr; }

	return std::move(doc);
}

bool OidTimeAccess::ParseBson(OidTime& obj, const MongoClib::AutoCursor& cursor)
{
	obj.oid = cursor.Oid(field._id);
	obj.idx = cursor.Int64(field.idx);
	obj.tmGen = cursor.Timet(field.tmGen);
	obj.tmOid = cursor.OidTime(field._id);
	obj.str = cursor.Str(field.str);
	obj.tmHex = cursor.Str(field.tmHex);
	if (cursor.IsFailed())
	{
		cout << "ParseBson failed: " << cursor.Message() << endl;
		return false;
	}
	return true;
}

bool OidTimeAccess::QueryUpdate(const std::int64_t idx, const bson_t* update)
{
	AutoBson query;
	query.Add(field.idx, idx);
	return Modify(query, update);
}

bool OidTimeAccess::Delete(const std::int64_t idx)
{
	AutoBson query;
	query.Add(field.idx, idx);
	return Remove(query);
}

bool OidTimeAccess::Update(const OidTime& obj)
{
	AutoBson cond;
	cond = BuildBson(obj);
	if (!cond) { return false; }

	BsonCmd update;
	update.Set(cond);


	return QueryUpdate(obj.idx, update);
}


bool OidTimeAccess::LoadOne(const std::int64_t idx, OidTime& obj)
{
	AutoBson query;
	query.Add(field.idx, idx);
	return QueryOne(obj, query);
}

bool OidTimeAccess::LoadAll(std::deque<OidTime>& dqObjects)
{
	return QueryLoad(dqObjects);
}

bool OidTimeAccess::LoadCond(std::deque<OidTime>& dqObjects, time_t tmGen)
{
	AutoBson query;	
	query.Add(field.tmGen, tmGen);
	return QueryLoad(dqObjects, query);
}


bool OidTimeAccess::LoadCond(std::deque<OidTime>& dqObjects, bson_oid_t& o1, bson_oid_t& o2)
{
	std::string gte = "$gte";
	std::string lt = "$lt";

	AutoBson query;
	AutoBson cond1;
	AutoBson cond2;

	BSON_APPEND_OID(cond1, gte.c_str(), &o1);
	BSON_APPEND_OID(cond2, lt.c_str(), &o2);

	query.AddDoc("_id", cond1);
	query.AddDoc("_id", cond2);

	AutoJson j(query);
	/*
	{ "_id" : { "$gte" : { "$oid" : "596db1d50000000000000000" } }, "_id" : { "$lt" : { "$oid" : "596db1da0000000000000000" } } }
	*/


	return QueryLoad(dqObjects, query);
}


bool OidTimeAccess::LoadCond(std::deque<OidTime>& dqObjects, time_t tm1, time_t tm2)
{
	BsonCmd query;
	const auto key = "_id";

	query.CompOidTime(key, BsonCmd::CTgte, tm1);
	query.CompOidTime(key, BsonCmd::CTlt, tm2);

	AutoJson j(query);
	/*
	{ "_id" : { "$gte" : { "$oid" : "596db1d50000000000000000" } }, "_id" : { "$lt" : { "$oid" : "596db1da0000000000000000" } } }
	*/

	return QueryLoad(dqObjects, query);
}

time_t  OidTimeAccess::LoadTmGen(const std::int64_t idx)
{
	OidTime obj;
	if (LoadOne(idx, obj))
	{
		// return default value
		return 0;
	}

	return obj.tmGen;
}


bool OidTimeAccess::CreateIndexs(void)
{
	AutoPoolColl coll(m_collname);
	if (!coll.CreateUniqueIndex(field.idx)) return false;
	if (!coll.CreateIndex(field.tmGen)) return false;
	return true;
}
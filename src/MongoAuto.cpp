#include "mfcafx.h"

#include "MongoAuto.h"
#include "MongoClient.h"
#include "MongocHelp.h"

#include "mfcnew.h"

using namespace std;

namespace MongoClib
{
	//////////////////////////////////////////////////////////////////////////
	// AutoUri
	// wrapper for mongoc_uri_t*

	AutoUri::AutoUri(mongoc_uri_t* p) 
		: m_p(p) 
	{ 
	};

	AutoUri::AutoUri(const std::string uri) 
	{
		m_p = mongoc_uri_new(uri.c_str()); 
	};

	AutoUri::~AutoUri() 
	{ 
		if (m_p) 
			mongoc_uri_destroy(m_p); 
	};

	AutoUri::operator mongoc_uri_t* () 
	{ 
		return m_p; 
	}


	//////////////////////////////////////////////////////////////////////////
	// class AutoOid

	void AutoOid::FromTime(const time_t tm)
	{
		Init();
		uint32_t tmbe = BSON_UINT32_TO_BE(tm);
		memcpy(&oid, &tmbe, sizeof(tmbe));
	}

	std::string AutoOid::ToString(void) const
	{
		char str[25];
		bson_oid_to_string(&oid, str);
		return std::string(str);
	}

	//////////////////////////////////////////////////////////////////////////
	// BsonParser
	// template class
	
	//////////////////////////////////////////////////////////////////////////
	// AutoBson
	// wrapper for bson_t*
	AutoBson::AutoBson() 
		: BsonParser(bson_new())
	{ };


	AutoBson::AutoBson(const std::string& json)
		: BsonParser(nullptr)
	{
		bson_error_t e;
		m_p = bson_new_from_json((uint8_t*)json.c_str(), -1, &e);
	}

	AutoBson::AutoBson(bson_t* p) 
		: BsonParser(p)
	{ };


	AutoBson::AutoBson(const bson_t& d)
		: BsonParser(bson_copy(&d))
	{
	}


	AutoBson& AutoBson::operator=(AutoBson& rhs)
	{
		auto p = bson_copy(rhs);
		if (m_p) { bson_destroy(m_p); }
		m_p = p;
		return *this;
	}

	// need move to BuildBson
	AutoBson::AutoBson(AutoBson&& rhs) 
		: BsonParser(rhs.Release())
	{ 
	}

	AutoBson& AutoBson::operator=(AutoBson&& rhs) 
	{
		auto p = rhs.Release();
		if (m_p) { bson_destroy(m_p); }
		m_p = p;
		return *this; 
	}

	AutoBson::~AutoBson() 
	{ 
		if (m_p) { bson_destroy(m_p); }
	};

	AutoBson::operator bson_t* () const 
	{ 
		return m_p; 
	};

	bson_t* AutoBson::Release(void)
	{
		bson_t* p = m_p;
		m_p = nullptr;
		return p;
	}

	// add key value
	bool AutoBson::Add(const std::string& key, const bool val)
	{
		if (!IsValid()) { return false; }
		return CheckCall(BSON_APPEND_BOOL(m_p, key.c_str(), val));
	}

	bool AutoBson::Add(const std::string& key, const int val)
	{
		if (!IsValid()) { return false; }
		return CheckCall(BSON_APPEND_INT32(m_p, key.c_str(), val));
	}

	bool AutoBson::Add(const std::string& key, const long long val)
	{
		if (!IsValid()) { return false; }
		return CheckCall(BSON_APPEND_INT64(m_p, key.c_str(), val));
	}

	bool AutoBson::Add(const std::string& key, const double val)
	{
		if (!IsValid()) { return false; }
		return CheckCall(BSON_APPEND_DOUBLE(m_p, key.c_str(), val));
	}

	bool AutoBson::Add(const std::string& key, const std::string& val)
	{
		if (!IsValid()) { return false; }
		return CheckCall(BSON_APPEND_UTF8(m_p, key.c_str(), val.c_str()));
	}

	bool AutoBson::Add(const std::string& key, const char* val)
	{
		if (!IsValid() || !CheckParam(val)) { return false; }
		return CheckCall(BSON_APPEND_UTF8(m_p, key.c_str(), val));
	}

	bool AutoBson::Add(const std::string& key, const std::wstring& val)
	{
		if (!IsValid()) { return false; }
		return CheckCall(BSON_APPEND_UTF8(m_p, key.c_str(), EncUtf8(val).c_str()));
	}

	bool AutoBson::Add(const std::string& key, const bson_oid_t* val)
	{
		if (!IsValid()) { return false; }
		return CheckCall(BSON_APPEND_OID(m_p, key.c_str(), val));
	}

	bool AutoBson::AddTime(const std::string& key)
	{
		if (!IsValid()) { return false; }
		return CheckCall(bson_append_now_utc(m_p, key.c_str(), static_cast<int>(key.length())));
	}

	bool AutoBson::AddTime(const std::string& key, time_t val)
	{
		if (!IsValid()) { return false; }
		return CheckCall(BSON_APPEND_TIME_T(m_p, key.c_str(), val));
	}

	bool AutoBson::AddBin(const std::string& key, const std::string& val)
	{
		if (!IsValid()) { return false; }
		return CheckCall(
			BSON_APPEND_BINARY(m_p,
				key.c_str(),
				BSON_SUBTYPE_BINARY,
				(const uint8_t*)val.c_str(),
				static_cast<unsigned>(val.length())));
	}

	bool AutoBson::AddDoc(const std::string& key, const bson_t* sub)
	{
		if (!IsValid() || !CheckParam(sub)) { return false; }
		return CheckCall(BSON_APPEND_DOCUMENT(m_p, key.c_str(), sub));
	}

	bool AutoBson::AddArray(const std::string& key, const bson_t* sub)
	{
		if (!IsValid() || !CheckParam(sub)) { return false; }
		return CheckCall(BSON_APPEND_ARRAY(m_p, key.c_str(), sub));
	}

#ifdef __AFXSTR_H__
	bool AutoBson::Add(const std::string& key, const CString& val)
	{
		return CheckCall(Add(key, EncUtf8MS(val)));
	}
#endif __AFXSTR_H__



	//////////////////////////////////////////////////////////////////////////
	// DocArray
	// used to add doc of array
	ArrayBuilder::ArrayBuilder()
	{ }

	ArrayBuilder::~ArrayBuilder()
	{ }

	//////////////////////////////////////////////////////////////////////////
	// QueryBson
	// used to query / set / sort etc.

	BsonCmd::BsonCmd() 
	{ };


	BsonCmd::BsonCmd(BsonCmd&& rhs)
		: AutoBson(std::move(rhs))
	{
	}

	BsonCmd& BsonCmd::operator = (BsonCmd&& rhs)
	{ 
		AutoBson::operator=(std::move(rhs));
		return *this; 
	}

	// $inc $mul $rename $setOnInsert $set $unset $min $max $currentDate
	bool BsonCmd::Cmd(const std::string& cmd, const AutoBson& cond)
	{
		return AddDoc(cmd, cond);
	}

	bool BsonCmd::Exists(const std::string& key, bool bExists /*= true*/)
	{
		return AddSub(key, "$exists", bExists);
	}

	bool BsonCmd::BitAnd(const std::string& key, int val)
	{
		AutoBson bitor;
		bitor.Add("and", val);
		AutoBson bkey;
		bkey.AddDoc(key, bitor);

		return AddDoc("$bit", bkey);
	}
	bool BsonCmd::BitOr(const std::string& key, int val)
	{
		AutoBson bitor;
		bitor.Add("or", val);
		AutoBson bkey;
		bkey.AddDoc(key, bitor);

		return AddDoc("$bit", bkey);
	}
	bool BsonCmd::BitXor(const std::string& key, int val)
	{
		AutoBson bitor;
		bitor.Add("xor", val);
		AutoBson bkey;
		bkey.AddDoc(key, bitor);

		return AddDoc("$bit", bkey);
	}

	bool BsonCmd::Query(const AutoBson& cond)
	{
		return AddDoc("$query", cond);
	}

	bool BsonCmd::Sort(const AutoBson& cond)
	{
		return AddDoc("$orderby", cond);
	}

	bool BsonCmd::Sort(const std::string& key, bool bDesc /*= true*/)
	{
		AutoBson cond;
		cond.Add(key, bDesc ? -1 : 1);
		return AddDoc("$orderby", cond);
	}

	bool BsonCmd::Set(const AutoBson& cond)
	{
		return AddDoc("$set", cond);
	}

	bool BsonCmd::SetArray(const std::string& key, const AutoBson& cond)
	{
		AutoBson doc;
		doc.AddArray(key, cond);
		return AddDoc("$set", doc);
	}

	bool BsonCmd::SetTime(const std::string& key)
	{
		AutoBson cond;
		cond.AddTime(key);
		return AddDoc("$set", cond);
	}

	bool BsonCmd::SetTime(const std::string& key, time_t val)
	{
		AutoBson cond;
		cond.AddTime(key, val);
		return AddDoc("$set", cond);
	}

	bool BsonCmd::AndOr(const CmdType type, const AutoBson& cond1, const AutoBson& cond2)
	{
		std::string cmd = CompStr(type);
		if (cmd.empty())
		{
			return false;
		}
		ArrayBuilder ar;
		ar.AddSub(cond1);
		ar.AddSub(cond2);
		return AddArray(cmd, ar);
	}

	bool BsonCmd::CompTime(const std::string& key, const CmdType type, time_t val)
	{
		std::string cmd = CompStr(type);
		if (cmd.empty())
		{
			return false;
		}
		AutoBson cond;
		cond.AddTime(cmd, val);
		return AddDoc(key, cond);
	}

	// $cmp : 0 if the two values are equivalent, 1 first > second, -1 first < second
	// $eq  : true if the values are equivalent.
	// $gt  : true if the first value is greater than the second.
	// $gte : true if the first value is greater than or equal to the second.
	// $lt  : true if the first value is less than the second.
	// $lte : true if the first value is less than or equal to the second.
	// $ne  : true if the values are not equivalent.
	std::string BsonCmd::CompStr(CmdType nComp)
	{
		switch (nComp)
		{
		case CTcmp:
			return "$cmp";
			break;
		case CTeq:
			return "$eq";
			break;
		case CTgt:
			return "$gt";
			break;
		case CTgte:
			return "$gte";
			break;
		case CTlt:
			return "$lt";
			break;
		case CTlte:
			return "$lte";
			break;
		case CTne:
			return "$ne";
			break;
		case CTand:
			return "$and";
			break;
		case CTor:
			return "$or";
			break;
		default:
			return "";
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoJson
	// wrapper for char* from bson_as_json
	AutoJson::AutoJson(char* p) 
		: m_p(p) 
	{ };

	AutoJson::AutoJson(const bson_t* p) 
	{ 
		if (p) 
			m_p = bson_as_json(p, nullptr);
	};

	AutoJson::~AutoJson() 
	{
		if (m_p) 
			bson_free(m_p); 
	};

	AutoJson::operator std::string () const
	{
		return m_p ? m_p : "";
	};

		
	//////////////////////////////////////////////////////////////////////////
	// BsonDocArray
	// used to parse array;
	ArrayParser::ArrayParser(const bson_t* doc, const std::string& key)
		: BsonParser(nullptr)
	{
		if (doc || key.length())
		{
			bson_iter_t idocarray;
			if (BsonKeyIter(doc, key, idocarray) && BSON_ITER_HOLDS_ARRAY(&idocarray))
			{
				if (bson_iter_recurse(&idocarray, &m_iBsonArray))
				{
					bInit = true;
					return;
				}
			}
		}
		bInit = false;
	}

	ArrayParser::~ArrayParser() 
	{
		ClearResultBson(); 
	};


	bool ArrayParser::HasElement(void) const
	{
		return bInit;
	}

	bool ArrayParser::Next()
	{
		if (bInit && bson_iter_next(&m_iBsonArray) && BSON_ITER_HOLDS_DOCUMENT(&m_iBsonArray))
		{
			uint8_t* pdoc = nullptr;
			uint32_t ndoc = 0;
			bson_iter_document(&m_iBsonArray, &ndoc, (const uint8_t**)&pdoc);
			ClearResultBson();
			m_p = bson_new_from_data(pdoc, ndoc);
			return true;
		}
		return false;
	}

	// return result bson by Next()
	const bson_t* ArrayParser::ElemBson() const { return m_p; }

	// m_p to retrieve key value, which created by bson_new_from_data, MUST be destroyed.
	void ArrayParser::ClearResultBson()
	{
		if (m_p)
		{
			bson_destroy(m_p);
			m_p = nullptr;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoCursor
	// wrapper for mongoc_cursor_t* and it's result (bson_t*) 

	AutoCursor::AutoCursor(mongoc_cursor_t* p) 
		: BsonParser(nullptr)
		, m_cursor(p) 
	{
	};
	
	AutoCursor::~AutoCursor() 
	{ 
		if (m_cursor)
			mongoc_cursor_destroy(m_cursor);
		if (m_coll)
			mongoc_collection_destroy(m_coll);
		if (m_client)
			PushClient(m_client);
	};
	
	AutoCursor::AutoCursor(AutoCursor&& rhs)
		: BsonParser(rhs.m_p)
	{
		auto client = rhs.m_client;
		auto coll = rhs.m_coll;
		auto doc = rhs.m_p;
		m_cursor = rhs.Release();
		m_client = client;
		m_coll = coll;
		m_p = doc;
	}

	// query
	AutoCursor::AutoCursor(const std::string& collname, const bson_t* query,
		int maxResult /*= 0*/, bson_t* field /*= nullptr*/,
		int skipResult /*= 0*/, mongoc_query_flags_t flags /*= MONGOC_QUERY_NONE*/,
		const mongoc_read_prefs_t* read_prefs /*= nullptr*/)
		: BsonParser(nullptr)
	{
		AutoPoolColl coll(collname);
		m_cursor = coll.Find(query, maxResult, field, skipResult, flags, read_prefs);
		if (m_cursor)
		{
			coll.Release(m_client, m_coll);
		}
	}

	bool AutoCursor::IsValid() 
	{ 
		return m_cursor ? true : false; 
	}

	bool AutoCursor::operator!() 
	{ 
		return !IsValid();
	}

	mongoc_cursor_t* AutoCursor::Cursor() const 
	{
		return m_cursor; 
	}

	mongoc_cursor_t* AutoCursor::Release()
	{
		mongoc_cursor_t* cursor = m_cursor;
		m_cursor = nullptr;
		m_client = nullptr;
		m_coll = nullptr;
		m_p = nullptr;
		return cursor;
	}

	bool AutoCursor::Next()
	{
		if (!m_cursor) { return false; }
		return mongoc_cursor_next(m_cursor, (const bson_t**)(&m_p));
	}

	// return result bson by Next()
	const bson_t* AutoCursor::ResultBson() const 
	{
		return m_p;
	}	
	
	//////////////////////////////////////////////////////////////////////////
	// AutoPoolColl
	// wrapper for client's pool
	// auto push & pop client from mongoc_client_pool_t and CRUD on collection
	// use Error() for detail when failed.

	AutoPoolColl::AutoPoolColl(const std::string& collname)
	{
		mongoc_client_t* client = TryPopClient();	
		auto clientdel = [](mongoc_client_t* p) { PushClient(p); };
		std::unique_ptr<mongoc_client_t, decltype(clientdel)> upClient(client, clientdel);
		if (!client)
		{
			return;
		}
		mongoc_collection_t* coll = 
			mongoc_client_get_collection(client, GetDbName(), collname.c_str());
		if (!coll)
		{
			return;
		}
		m_client = upClient.release();
		m_coll = coll;
	}
		
	AutoPoolColl::~AutoPoolColl() 
	{ 
		Destroy();
	};
	
	bool AutoPoolColl::IsValid(void)
	{
		if (m_client && m_coll) { return true; }

		SetErr("Null collection");
		return false;
	}

	bool AutoPoolColl::operator!() 
	{
		return !IsValid();
	}

	AutoPoolColl::operator mongoc_collection_t* () 
	{ 
		return m_coll; 
	}


	bool AutoPoolColl::Rename(const std::string& newName, bool drop_target_before_rename /*= false*/)
	{
		if (!IsValid() || newName.empty()) { return false; }
		return mongoc_collection_rename(m_coll, GetDbName(), newName.c_str(), drop_target_before_rename, &m_err);
	}

	bool AutoPoolColl::Insert(const bson_t* doc, mongoc_insert_flags_t flags /*= MONGOC_INSERT_NONE*/)
	{
		if (!IsValid() || !doc) { return false; }
		return mongoc_collection_insert(m_coll, flags, doc, nullptr, &m_err);
	};

	bool AutoPoolColl::Delete(const bson_t* query, mongoc_remove_flags_t flags /*= MONGOC_REMOVE_NONE*/)
	{
		if (!IsValid() || !query) { return false; }
		return mongoc_collection_remove(m_coll, flags, query, NULL, &m_err);
	}

	// return true if record not exist but no DB operation failure
	bool AutoPoolColl::Update(const bson_t* query, const bson_t* update, mongoc_update_flags_t flags /*= MONGOC_UPDATE_NONE*/)
	{
		if (!IsValid() || !query || !update) { return false; }
		return mongoc_collection_update(m_coll, flags, query, update, nullptr, &m_err);
	}

	bool AutoPoolColl::MultiUpdate(const bson_t* query, const bson_t* update)
	{
		return Update(query, update, MONGOC_UPDATE_MULTI_UPDATE);
	}

	bool AutoPoolColl::Modify(const bson_t* query, const bson_t* update)
	{
		if (!IsValid() || !query || !update)
		{
			return false;
		}
		StackBson reply;
		if (!mongoc_collection_find_and_modify(m_coll, query, nullptr, update, nullptr, false, false, false, reply.RawPtr(), &m_err))
		{
			return false;
		}
		string val;
		if (!BsonJson(reply.RawPtr(), "value", val))
		{
			SetErr("record not exist");
			return false;
		}
		return true;
	}

	mongoc_cursor_t* AutoPoolColl::Find(const bson_t* query, int maxResult /*= 0*/, bson_t* field /*= nullptr*/,
		int skipResult /*= 0*/, mongoc_query_flags_t flags /*= MONGOC_QUERY_NONE*/, const mongoc_read_prefs_t* read_prefs /*= nullptr*/)
	{
		if (!IsValid() || !query)  { return false; }
		return mongoc_collection_find(m_coll, flags, skipResult, maxResult, 0, query, field, read_prefs);
	}

	long long AutoPoolColl::Count(const bson_t* query, int limitResult /*= 0*/, mongoc_query_flags_t flags /*= MONGOC_QUERY_NONE*/,
		int skipResult /*= 0*/, const mongoc_read_prefs_t *read_prefs /*= nullptr*/)
	{
		if (!IsValid() || !query)  { return -1; }
		return mongoc_collection_count(m_coll, flags, query, skipResult, limitResult, read_prefs, &m_err);
	}

	bool AutoPoolColl::CreateIndex(const bson_t* indices, bool bUnique /*= false*/)
	{
		if (!IsValid() || !indices)  { return false; }

		mongoc_index_opt_t opt;
		mongoc_index_opt_init(&opt);
		opt.unique = bUnique;

		return mongoc_collection_create_index(m_coll, indices, &opt, &m_err);
	}

	bool AutoPoolColl::CreateIndex(const std::string& field, bool bUnique /*= false*/, bool bAsc /*= true*/)
	{
		if (!field.length())  { return false; }

		AutoBson keys;
		keys.Add(field, bAsc ? 1 : -1);

		return CreateIndex(keys, bUnique);
	}

	bool AutoPoolColl::CreateUniqueIndex(const std::string& field, bool bAsc /*= true*/)
	{
		return CreateIndex(field, true, bAsc);
	}

	bool AutoPoolColl::CreateExpireIndex(const std::string& field, int expireSec)
	{
		if (!field.length())  { return false; }

		AutoBson keys;
		// asc order
		keys.Add(field, 1);

		if (!IsValid() || !keys.IsValid())  { return false; }

		mongoc_index_opt_t opt;
		mongoc_index_opt_init(&opt);
		opt.expire_after_seconds = expireSec;

		return mongoc_collection_create_index(m_coll, keys, &opt, &m_err);
	}
	
	const std::string AutoPoolColl::Error(void) const
	{
		if (!m_err.code) { return ""; }
		stringstream ss;
		ss << m_err.domain << "-" << m_err.code << ": " << m_err.message;
		return ss.str(); 
	}
		
#ifdef __AFXSTR_H__
	CString AutoPoolColl::CError(void) const
	{ 
		return DecUtf8MS(Error());
	}

	bool AutoPoolColl::CreateIndex(const CString& field, bool bUnique /*= false*/, bool bAsc /*= true*/)
	{
		return CreateIndex(EncUtf8MS(field), bUnique, bAsc);
	}

	bool AutoPoolColl::CreateUniqueIndex(const CString& field, bool bAsc /*= true*/)
	{
		return CreateUniqueIndex(EncUtf8MS(field), bAsc);
	}
#endif // __AFXSTR_H__

	void AutoPoolColl::Destroy(void)
	{
		if (m_coll)
		{
			mongoc_collection_destroy(m_coll);
			m_coll = nullptr;
		}
		if (m_client)
		{
			PushClient(m_client);
			m_client = nullptr;
		}
	}

	void AutoPoolColl::Release(mongoc_client_t*& client, mongoc_collection_t*& coll)
	{
		client = m_client;
		coll = m_coll;
		m_client = nullptr;
		m_coll = nullptr;
	}

	void AutoPoolColl::SetErr(const std::string& str)
	{
		auto copylen = sizeof(m_err.message);
		auto strlen = str.length() + 1;
		if (strlen < copylen)
		{
			copylen = strlen;
		}

		memcpy(m_err.message, str.c_str(), copylen);

		m_err.message[copylen - 1] = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	// BulkOperator

	BulkOperator::BulkOperator(const std::string& collname, bool ordered /*= false*/, const mongoc_write_concern_t *write_concern /*= nullptr*/)
		: m_pollColl(collname)
	{
		if (!m_pollColl)
		{
			return;
		}
		m_bulk = mongoc_collection_create_bulk_operation(m_pollColl, ordered, write_concern);
	}

	BulkOperator::~BulkOperator()
	{
		if (m_bulk)
		{
			mongoc_bulk_operation_destroy(m_bulk);
		}
	}

	bool BulkOperator::IsValid(void)
	{
		return m_bulk ? true : false;
	}

	void BulkOperator::Insert(const bson_t* doc)
	{
		if (m_bulk && doc)
		{
			mongoc_bulk_operation_insert(m_bulk, doc);
		}
	}

	void BulkOperator::Remove(const bson_t* query, bool bMulti /*= false*/)
	{
		if (m_bulk && query)
		{
			if (bMulti)
			{
				mongoc_bulk_operation_remove(m_bulk, query);
			}
			else
			{
				mongoc_bulk_operation_remove_one(m_bulk, query);
			}
		}
	}

	void BulkOperator::Update(const bson_t* query, const bson_t* update, bool bUpsert /*= false*/, bool bMulti /*= false*/)
	{
		if (m_bulk && query)
		{
			if (bMulti)
			{
				mongoc_bulk_operation_update(m_bulk, query, update, bUpsert);
			}
			else
			{
				mongoc_bulk_operation_update_one(m_bulk, query, update, bUpsert);
			}
		}
	}

	void BulkOperator::ReplaceOne(const bson_t* query, const bson_t* doc, bool bUpsert /*= false*/)
	{
		if (m_bulk && query)
		{
			mongoc_bulk_operation_replace_one(m_bulk, query, doc, bUpsert);
		}
	}

	unsigned BulkOperator::Execute(void)
	{
		if (!m_bulk)
		{
			return false;
		}
		unsigned bRet = mongoc_bulk_operation_execute(m_bulk, m_reply.RawPtr(), &m_err);
		if (!bRet)
		{
			stringstream ss;
			ss << m_err.domain << "-" << m_err.code << ": " << m_err.message << ", " << AutoJson(m_reply.RawPtr());
			m_errString = ss.str();
		}
		return bRet;
	}

	// execute result
	int BulkOperator::nInserted(void) const
	{
		return m_reply.Int32("nInserted");
	}

	int BulkOperator::nMatched(void) const
	{
		return m_reply.Int32("nMatched");
	}

	int BulkOperator::nModified(void) const
	{
		return m_reply.Int32("nModified");
	}

	int BulkOperator::nRemoved(void) const
	{
		return m_reply.Int32("nRemoved");
	}

	int BulkOperator::nUpserted(void) const
	{
		return m_reply.Int32("nUpserted");
	}

	uint32_t BulkOperator::nEDomain(void) const
	{
		return m_err.domain;
	}

	uint32_t BulkOperator::nECode(void) const
	{
		return m_err.code;
	}

	std::string BulkOperator::writeErrors(void) const
	{
		return m_reply.Str("writeErrors");
	}

	std::string BulkOperator::writeConcernErrors(void) const
	{
		return m_reply.Str("writeConcernErrors");
	}

	const std::string BulkOperator::Error(void) const
	{
		return m_errString;
	}

	std::wstring BulkOperator::WError(void) const
	{
		return DecUtf8(Error());
	}

#ifdef __AFXSTR_H__
	CString BulkOperator::CError(void) const
	{
		return DecUtf8MS(Error());
	}

#endif // __AFXSTR_H__

	//////////////////////////////////////////////////////////////////////////
	// template body
	//////////////////////////////////////////////////////////////////////////

	template<typename BsonT>
	BsonParser<BsonT>::BsonParser(BsonT* p)
		: m_p(p)
	{ };

	template<typename BsonT>
	BsonParser<BsonT>::~BsonParser()
	{ };

	//////////////////////////////////////////////////////////////////////////
	// parser
	template<typename BsonT>
	bool BsonParser<BsonT>::Bool(const std::string& key, bool setFail = true) const
	{
		bool val = 0;
		CheckCall(BsonValue(m_p, key, val), setFail);
		return val;
	}

	template<typename BsonT>
	bool BsonParser<BsonT>::HasKey(const std::string&key) const
	{
		bool val = 0;
		CheckCall(BsonHasKey(m_p, key, val));
		return val;
	}

	template<typename BsonT>
	int BsonParser<BsonT>::Int32(const std::string& key, bool setFail = true) const
	{
		int val = 0;
		CheckCall(BsonValue(m_p, key, val), setFail);
		return val;
	}

	template<typename BsonT>
	long long BsonParser<BsonT>::Int64(const std::string& key, bool setFail = true) const
	{
		long long val = 0;
		CheckCall(BsonValue(m_p, key, val), setFail);
		return val;
	}

	template<typename BsonT>
	time_t BsonParser<BsonT>::Timet(const std::string& key, bool setFail = true) const
	{
		time_t val = 0;
		CheckCall(BsonValueTimet(m_p, key, val), setFail);
		return val;
	}

	template<typename BsonT>
	double BsonParser<BsonT>::Double(const std::string& key, bool setFail = true) const
	{
		double val = 0;
		CheckCall(BsonValue(m_p, key, val), setFail);
		return val;
	}

	template<typename BsonT>
	std::string BsonParser<BsonT>::Str(const std::string& key, bool setFail = true) const
	{
		std::string val;
		CheckCall(BsonValue(m_p, key, val), setFail);
		return val;
	}

	template<typename BsonT>
	std::wstring BsonParser<BsonT>::WStr(const std::string& key, bool setFail = true) const
	{
		return DecUtf8(Str(key, setFail));
	}

	template<typename BsonT>
	std::string BsonParser<BsonT>::Bin(const std::string& key, bool setFail = true) const
	{
		std::string val;
		CheckCall(BsonValueBin(m_p, key, val), setFail);
		return val;
	}		

	template<typename BsonT>
	AutoBson BsonParser<BsonT>::Doc(const std::string& key, bool setFail = true) const
	{
		bson_t* val = nullptr;
		CheckCall(BsonDoc(m_p, key, val), setFail);
		return val;
	}

	template<typename BsonT>
	std::string BsonParser<BsonT>::Oid(const std::string& key = "_id", bool setFail = true) const
	{
		std::string val;
		CheckCall(BsonOid(m_p, key, val), setFail);
		return val;
	}

	template<typename BsonT>
	time_t BsonParser<BsonT>::OidTime(const std::string& key = "_id", bool setFail = true) const
	{
		time_t val;
		CheckCall(BsonOidTime(m_p, key, val), setFail);
		return val;
	}
	
#ifdef __AFXSTR_H__
	template<typename BsonT>
	CString BsonParser<BsonT>::CStr(const std::string& key, bool setFail = true) const
	{
		return DecUtf8MS(Str(key, setFail));
	}

	template<typename BsonT>
	std::vector<CString> BsonParser<BsonT>::GetVecCStr(const std::string& key) const
	{
		std::vector<CString> vt;
		if (!GetCntr(key, vt, str2cstring))
		{
			return std::vector<CString>();
		}
		return std::move(vt);
	}
#endif // __AFXSTR_H__

	template<typename BsonT>
	bool BsonParser<BsonT>::IsValid(void)
	{
		if (!m_p)
		{
			m_failMsg = "bson not valid.";
			m_bFailed = true;
			return false;
		}
		return true;
	}

	template<typename BsonT>
	bool BsonParser<BsonT>::CheckCall(bool bRet, bool setFail /*= true*/) const
	{
		if (setFail && !bRet)
		{
			m_failMsg = "function call failed.";
			m_bFailed = true;
		}
		return bRet;
	}

	template<typename BsonT>
	bool BsonParser<BsonT>::CheckParam(const void* pParam)
	{
		if (!pParam)
		{
			m_failMsg = "parameter not valid.";
			m_bFailed = true;
			return false;
		}
		return true;
	}

	template<typename BsonT>
	bool BsonParser<BsonT>::IsFailed(void) const
	{
		return m_bFailed;
	}

	// only last failed message
	template<typename BsonT>
	std::string BsonParser<BsonT>::Message(void) const
	{
		return m_failMsg;
	}

#ifdef __AFXSTR_H__
	template<typename BsonT>
	CString BsonParser<BsonT>::CMessage(void) const
	{
		return DecUtf8MS(Message());
	}
#endif // __AFXSTR_H__


	template class BsonParser < bson_t > ;
	template class BsonParser < const bson_t >;

} // MongoClib
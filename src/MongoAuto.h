#ifndef MongoAuto_h__
#define MongoAuto_h__

#include <string>
#include <sstream>
#include <vector>
#include <chrono>
#include <bson.h>
#include <mongoc.h>
#include "MongocHelp.h"


//////////////////////////////////////////////////////////////////////////
// help class

namespace MongoClib
{
	// suggest use AutoPoolColl replace AutoClient & AutoColl & AutoPoolClient
	
	//////////////////////////////////////////////////////////////////////////
	// declare all classes	

	// wrapper for mongoc_uri_t*
	class AutoUri;

	// template wrapper for bson_t* / const bson_t*
// 	template<typename BsonT>
// 	class BsonParser;

	// wrapper for bson_t*
	class AutoBson;
	// wrapper for bson_t
	class StackBson;
	// add bson to array, rename to ArrayBuilder
	class ArrayBuilder;

	// used to build query / set / sort ... bson_t.
	// rename to BsonCmd
	class BsonCmd;
	// parse doc array in bson, rename to DocArrayParser
	class ArrayParser;

	// wrapper for char* from bson_as_json
	class AutoJson;
	// wrapper for mongoc_cursor_t* and its result (bson_t*) 
	class AutoCursor;
	// wrapper for client's pool
	// auto push & pop client from mongoc_client_pool_t and CRUD on collection
	// use Error() for detail when failed.
	class AutoPoolColl;


	//////////////////////////////////////////////////////////////////////////
	// AutoUri
	// wrapper for mongoc_uri_t*
	class AutoUri
	{
		AutoUri(AutoUri& rhs) = delete;
		AutoUri(AutoUri&& rhs) = delete;
		AutoUri& operator = (AutoUri& rhs) = delete;
		AutoUri& operator = (AutoUri&& rhs) = delete;

	public:
		AutoUri(mongoc_uri_t* p);
		AutoUri(const std::string uri);
		~AutoUri();

		operator mongoc_uri_t* ();

	private:
		mongoc_uri_t* m_p = nullptr;
	};

	//////////////////////////////////////////////////////////////////////////
	// class BsonBase
	template<typename BsonT>
	class BsonParser
	{
	public:
		BsonParser(BsonT* p);
		virtual ~BsonParser() = 0;

		//////////////////////////////////////////////////////////////////////////
		// parser
		bool Bool(const std::string& key) const;
		int Int32(const std::string& key) const;
		long long Int64(const std::string& key) const;
		double Double(const std::string& key) const;
		std::string Str(const std::string& key) const;
		std::wstring WStr(const std::string& key) const;
		std::string Bin(const std::string& key) const;
		AutoBson Doc(const std::string& key) const;
		std::string Oid(const std::string& key = "_id") const;
		time_t OidTime(const std::string& key = "_id") const;

		// get string array (in back_inserter supported container) from bson
		template<typename Container, typename TransFunc>
		inline bool GetCntr(const std::string& key, Container& cntr, TransFunc transFunc) const
		{
			return CheckCall(BsonGetContainer(m_p, key, cntr, transFunc));
		};

#ifdef __AFXSTR_H__
		CString CStr(const std::string& key) const;
		std::vector<CString> GetVecCStr(const std::string& key) const;
#endif // __AFXSTR_H__

	protected:
		BsonT* m_p = nullptr;

		// once failed, NEVER set to false.
		mutable bool m_bFailed = false;
		mutable std::string m_failMsg;
	public:
		bool IsValid(void);

		bool CheckCall(bool bRet) const;

		bool CheckParam(const void* pParam);

		bool IsFailed(void) const;

		// only last failed message
		std::string Message(void) const;

#ifdef __AFXSTR_H__
		CString CMessage(void) const;
#endif // __AFXSTR_H__
	};

	extern template class BsonParser < bson_t > ;
	extern template class BsonParser < const bson_t > ;


	class AutoBson : public BsonParser<bson_t>
	{
		AutoBson(AutoBson& rhs) = delete;
		AutoBson& operator=(AutoBson& rhs) = delete;
	public:
		AutoBson();
		AutoBson(bson_t* p);
		virtual ~AutoBson();

		// move ctor & assign for BuildBson
		AutoBson(AutoBson&& rhs);
		AutoBson& operator=(AutoBson&& rhs);

		operator bson_t* () const;

		//////////////////////////////////////////////////////////////////////////
		// builder
		bool Add(const std::string& key, const bool val);
		bool Add(const std::string& key, const int val);
		bool Add(const std::string& key, const long long val);
		bool Add(const std::string& key, const double val);
		bool Add(const std::string& key, const std::string& val);
		bool Add(const std::string& key, const char* val);
		bool Add(const std::string& key, const std::wstring& val);
		bool AddBin(const std::string& key, const std::string& val);
		bool AddDoc(const std::string& key, const bson_t* sub);
		bool AddArray(const std::string& key, const bson_t* sub);

		// add string container to bson
		template<typename Container, typename TransFunc>
		inline bool Add(const std::string& key, Container& cntr, TransFunc transFunc)
		{
			if (!IsValid()) { return false; }
			return CheckCall(BsonAddContainer(m_p, key, cntr, transFunc));
		}

#ifdef __AFXSTR_H__
		bool Add(const std::string& key, const CString& val);
#endif // __AFXSTR_H__

		bson_t* Release(void);
	};
	
	//////////////////////////////////////////////////////////////////////////
	// DocArray
	// add bson to array
	class ArrayBuilder : public AutoBson
	{
		ArrayBuilder(ArrayBuilder& rhs) = delete;
		ArrayBuilder(ArrayBuilder&& rhs) = delete;
		ArrayBuilder& operator = (ArrayBuilder& rhs) = delete;
		ArrayBuilder& operator = (ArrayBuilder&& rhs) = delete;
	public:
		ArrayBuilder();
		~ArrayBuilder();

		bool AddSub(const bson_t* sub);
	private:
		unsigned long long idx = static_cast<unsigned long long >(- 1);
	};

	//////////////////////////////////////////////////////////////////////////
	// QueryBson
	// used to query / set / sort etc.
	class BsonCmd : public AutoBson
	{
		BsonCmd(BsonCmd& rhs) = delete;
		BsonCmd(BsonCmd&& rhs) = delete;
		BsonCmd& operator = (BsonCmd& rhs) = delete;
		BsonCmd& operator = (BsonCmd&& rhs) = delete;

	public:
		BsonCmd();

		// $inc $mul $rename $setOnInsert $set $unset $min $max $currentDate
		bool Cmd(const std::string& cmd, const AutoBson& cond);
		bool Query(const AutoBson& cond);
		bool Sort(const AutoBson& cond);
		bool Sort(const std::string& key, bool bDesc = true);		
		bool Set(const AutoBson& cond);

		// with complex data, build bson & use Set(const AutoBson&)
		template<class T>
		bool Set(const std::string& key, T& val)
		{
			AutoBson cond;
			cond.Add(key, val);
			return AddDoc("$set", cond);
		};
	};

	//////////////////////////////////////////////////////////////////////////
	// StackBson
	// wrapper for bson_t
	class StackBson : public BsonParser<bson_t>
	{
		StackBson(StackBson& rhs) = delete;
		StackBson(StackBson&& rhs) = delete;
		StackBson& operator = (StackBson& rhs) = delete;
		StackBson& operator = (StackBson&& rhs) = delete;

	public:
		StackBson() : BsonParser(&doc) { bson_init(&doc); };
		~StackBson() { bson_destroy(&doc); };
		
		bson_t* RawPtr()
		{
			return &doc;
		}

	//	operator bson_t* () { return &doc; };
	private:
		bson_t doc;
	};

	//////////////////////////////////////////////////////////////////////////
	// AutoJson
	// wrapper for char* from bson_as_json
	class AutoJson
	{
		AutoJson(AutoJson& rhs) = delete;
		AutoJson(AutoJson&& rhs) = delete;
		AutoJson& operator = (AutoJson& rhs) = delete;
		AutoJson& operator = (AutoJson&& rhs) = delete;
	public:
		explicit AutoJson(char* p);
		AutoJson(const bson_t* p);
		~AutoJson();

		operator std::string () const;

	private:
		char* m_p = nullptr;
	};

	inline std::ostream& operator<<(std::ostream& os, const AutoJson& json)
	{
		return os << static_cast<std::string>(json);
	}
		
	//////////////////////////////////////////////////////////////////////////
	// BsonDocArray
	// used to parse array;
	class ArrayParser : public BsonParser<bson_t>
	{
		ArrayParser(ArrayParser& rhs) = delete;
		ArrayParser(ArrayParser&& rhs) = delete;
		ArrayParser& operator = (ArrayParser& rhs) = delete;
		ArrayParser& operator = (ArrayParser&& rhs) = delete;

	public:
		ArrayParser(const bson_t* doc, const std::string& key);
		~ArrayParser();

		bool HasElement(void) const;

		bool Next();

		// return result bson by Next()
		const bson_t* ElemBson() const;

	private:
		bson_iter_t m_iBsonArray;
		bool bInit = false;

		// m_p to retrieve key value, which created by bson_new_from_data, MUST be destroyed.
		void ClearResultBson();
	};


	//////////////////////////////////////////////////////////////////////////
	// AutoCursor
	// wrapper for mongoc_cursor_t* and it's result (bson_t*) 
	class AutoCursor : public BsonParser<const bson_t>
	{
		AutoCursor(AutoCursor& rhs) = delete;
		AutoCursor& operator=(AutoCursor& rhs) = delete;
		AutoCursor& operator=(AutoCursor&& rhs) = delete;
	public:
		AutoCursor(mongoc_cursor_t* p);
		~AutoCursor();

		AutoCursor(AutoCursor&& rhs);

		// ctor with query
		AutoCursor(const std::string& collname, const bson_t* query, int maxResult = 0, bson_t* field = nullptr,
			int skipResult = 0, mongoc_query_flags_t flags = MONGOC_QUERY_NONE, const mongoc_read_prefs_t* read_prefs = nullptr);

		bool IsValid();
		bool operator!();
		bool Next();

		mongoc_cursor_t* Cursor() const;
		mongoc_cursor_t* Release();

		// result bson by Next()
		const bson_t* ResultBson() const;

	private:
		mongoc_cursor_t* m_cursor = nullptr;
		// for query ctor
		mongoc_client_t* m_client = nullptr;
		mongoc_collection_t* m_coll = nullptr;
	};
	
	//////////////////////////////////////////////////////////////////////////
	// AutoPoolColl
	// wrapper for client's pool
	// auto push & pop client from mongoc_client_pool_t and CRUD on collection
	// use Error() for detail when failed.
	class AutoPoolColl
	{
		AutoPoolColl(AutoPoolColl& rhs) = delete;
		AutoPoolColl(AutoPoolColl&& rhs) = delete;
		AutoPoolColl& operator = (AutoPoolColl& rhs) = delete;
		AutoPoolColl& operator = (AutoPoolColl&& rhs) = delete;

	public:
		explicit AutoPoolColl(const std::string& collname);
		~AutoPoolColl();

		bool IsValid(void);

		bool operator!();

		operator mongoc_collection_t* ();

		bool Insert(const bson_t* doc, mongoc_insert_flags_t flags = MONGOC_INSERT_NONE);
		bool Delete(const bson_t* query, mongoc_remove_flags_t flags = MONGOC_REMOVE_NONE);
		// return true if record not exist but no DB operation failure
		bool Update(const bson_t* query, const bson_t* update, mongoc_update_flags_t flags = MONGOC_UPDATE_NONE);
		bool MultiUpdate(const bson_t* query, const bson_t* update);
		// return false if record not exist. only one record could be modified.
		bool Modify(const bson_t* query, const bson_t* update);
		
		mongoc_cursor_t* Find(const bson_t* query, int maxResult = 0, bson_t* field = nullptr,
			int skipResult = 0, mongoc_query_flags_t flags = MONGOC_QUERY_NONE, const mongoc_read_prefs_t* read_prefs = nullptr);

		long long Count(const bson_t* query, int limitResult = 0, mongoc_query_flags_t flags = MONGOC_QUERY_NONE,
			int skipResult = 0, const mongoc_read_prefs_t *read_prefs = nullptr);

		bool CreateIndex(const bson_t* indices, bool bUnique = false);
		bool CreateIndex(const std::string& field, bool bUnique = false, bool bAsc = true);
		bool CreateUniqueIndex(const std::string& field, bool bAsc = true);
		
		const std::string Error(void) const;
		std::wstring WError(void) const;

		void Release(mongoc_client_t*& client, mongoc_collection_t*& coll);

#ifdef __AFXSTR_H__
		CString CError(void) const;
		bool CreateIndex(const CString& field, bool bUnique = false, bool bAsc = true);
		bool CreateUniqueIndex(const CString& field, bool bAsc = true);
#endif // __AFXSTR_H__

	private:
		mongoc_client_t* m_client = nullptr;
		mongoc_collection_t* m_coll = nullptr;
		bson_error_t m_err;

		void Destroy(void);
		void SetErr(const std::string& str);
	};

	class BulkOperator
	{
		BulkOperator(BulkOperator& rhs) = delete;
		BulkOperator(BulkOperator&& rhs) = delete;
		BulkOperator& operator = (BulkOperator& rhs) = delete;
		BulkOperator& operator = (BulkOperator&& rhs) = delete;

	public:
		explicit BulkOperator(const std::string& collname, bool ordered = false, const mongoc_write_concern_t *write_concern = nullptr);
		~BulkOperator();
		
		bool IsValid(void);

		void Insert(const bson_t* doc);
		void Remove(const bson_t* query, bool bMulti = false);
		void Update(const bson_t* query, const bson_t* update, bool bUpsert = false, bool bMulti = false);
		void ReplaceOne(const bson_t* query, const bson_t* doc, bool bUpsert = false);

		unsigned Execute(void);

		// execute result
		int nInserted(void) const;
		int nMatched(void) const;
		int nModified(void) const;
		int nRemoved(void) const;
		int nUpserted(void) const;
		std::string writeErrors(void) const;
		std::string writeConcernErrors(void) const;

		const std::string Error(void) const;
		std::wstring WError(void) const;

#ifdef __AFXSTR_H__
		CString CError(void) const;
#endif // __AFXSTR_H__

	private:
		AutoPoolColl m_pollColl;
		mongoc_bulk_operation_t* m_bulk = nullptr;
		StackBson m_reply;
		std::string m_errString;
	};


	class Timer
	{
	public:
		void Tick()
		{
			tp = std::chrono::steady_clock::now();
		}

		long long Tock(void)
		{
			auto dur = std::chrono::steady_clock::now() - tp;
			auto microsec = std::chrono::duration_cast<std::chrono::microseconds>(dur);
			return microsec.count();
		}
	protected:
		std::chrono::steady_clock::time_point tp;
	};

	//////////////////////////////////////////////////////////////////////////
	//                             class body                               //
	//////////////////////////////////////////////////////////////////////////


	
} // MongoClib

#endif // MongoAuto_h__

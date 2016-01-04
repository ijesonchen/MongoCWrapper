#ifndef MongoAuto2_h__
#define MongoAuto2_h__

#include <string>
#include <sstream>
#include <vector>
#include <bson.h>
#include <mongoc.h>
#include "MongocHelp.h"


//////////////////////////////////////////////////////////////////////////
// deprecated class & functions

namespace MongoClib
{
	// suggest use AutoPoolClient & AutoPoolColl replace AutoClient & AutoColl
	
	//////////////////////////////////////////////////////////////////////////
	// declare all classes
	class AutoClient; 
	class AutoColl;
	class AutoPoolClient;
	
	//////////////////////////////////////////////////////////////////////////
	// AutoClient
	// wrapper for mongoc_client_t*
	class AutoClient
	{
		AutoClient(AutoClient& rhs) = delete;
		AutoClient(AutoClient&& rhs) = delete;
		AutoClient& operator = (AutoClient& rhs) = delete;
		AutoClient& operator = (AutoClient&& rhs) = delete;

	public:
		AutoClient(mongoc_client_t* p);
		explicit AutoClient(const std::string& srv);
		~AutoClient();

		operator mongoc_client_t* ();

	private:
		mongoc_client_t* m_p = nullptr;
	};

	//////////////////////////////////////////////////////////////////////////
	// AutoColl
	// wrapper for mongoc_collection_t*
	class AutoColl
	{
		AutoColl(AutoColl& rhs) = delete;
		AutoColl(AutoColl&& rhs) = delete;
		AutoColl& operator = (AutoColl& rhs) = delete;
		AutoColl& operator = (AutoColl&& rhs) = delete;

	public:
		AutoColl(mongoc_collection_t* p);
		AutoColl(mongoc_client_t* client, std::string& db, std::string& coll);
		~AutoColl();
		
		operator mongoc_collection_t* ();
		mongoc_collection_t* Release(void);

	private:
		mongoc_collection_t* m_p = nullptr;
	};

	//////////////////////////////////////////////////////////////////////////
	// used with mongoc_client_pool_t
	// wrapper for mongoc_client_t* from client's pool
	class AutoPoolClient
	{
		AutoPoolClient(AutoPoolClient& rhs) = delete;
		AutoPoolClient(AutoPoolClient&& rhs) = delete;
		AutoPoolClient& operator = (AutoPoolClient& rhs) = delete;
		AutoPoolClient& operator = (AutoPoolClient&& rhs) = delete;
	public:
		AutoPoolClient();
		~AutoPoolClient();

		operator mongoc_client_t* () const;
		mongoc_client_t* Release(void);

	private:
		mongoc_client_t* m_p = nullptr;
	};

	//////////////////////////////////////////////////////////////////////////
	// create index. deprecated. use AutoPollColl::CreateIndex
	bool CreateIndex(mongoc_collection_t *collection, const std::string& field, bool bAsc = true, bool bUnique = false, std::string* pError = nullptr);
	bool CreateIndex(mongoc_client_t* client, const std::string& coll, const std::string& field, bool bAsc = true, bool bUnique = false, std::string* pError = nullptr);
	bool CreateIndex(const std::string& coll, const std::string& field, bool bAsc = true, bool bUnique = false, std::string* pError = nullptr);
} // MongoClib


#endif // MongoAuto2_h__

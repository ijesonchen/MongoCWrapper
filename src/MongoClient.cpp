#include "mfcafx.h"
#include "MongoClient.h"

#include <sstream>
#include <mongoc.h>

#include "MongoAuto.h"
#include "MongocHelp.h"

#include "mfcnew.h"

#ifdef _MSC_VER
	#pragma comment(lib, "bson-1.0.lib")
	#pragma comment(lib, "mongoc-1.0.lib")
#endif // _MSC_VER

using namespace std;

namespace MongoClib
{
	const unsigned MongoTimeoutMillisec = 5000;

//////////////////////////////////////////////////////////////////////////
// class MongocDrv

class MongoClient
{
	MongoClient(MongoClient& rhs) = delete;
	MongoClient(MongoClient&& rhs) = delete;
	MongoClient& operator = (MongoClient& rhs) = delete;
	MongoClient& operator = (MongoClient&& rhs) = delete;
public:
	MongoClient();

	~MongoClient();

	int MongoInit(const std::wstring& wsrv, const std::wstring& wdb,
			int toConnectMS = MongoTimeoutMillisec, unsigned short port = 27017,
			int toSelectMS = 0, int toSocketMS = 0);
	void Clear();

	mongoc_client_t* TryPop(void);
	void Push(mongoc_client_t* client);

	const char* db(void) const
	{
		return m_db.c_str();
	}

	const wchar_t* wdb(void) const
	{
		return m_wdb.c_str();
	}
	

protected:
	std::string m_srv;
	unsigned short m_port = 0;
	std::string m_db;
	std::wstring m_wdb;
	mongoc_client_pool_t* m_pool = nullptr;
};

MongoClient::MongoClient()
{
	mongoc_init();
}

MongoClient::~MongoClient()
{
	Clear();
	mongoc_cleanup();
}

int MongoClient::MongoInit(const std::wstring& wsrv, const std::wstring& wdb,
	int toConnectMS /*= MongoTimeoutMillisec*/, unsigned short port /*= 27017*/,
	int toSelectMS /*= 0*/, int toSocketMS /*= 0*/)
{
	if (m_pool)
	{
		return -1;
	}
	string u8Srv = EncUtf8(wsrv);
	string u8db = EncUtf8(wdb);
	if (!u8Srv.length() || !port || !u8db.length())
	{
		return -2;
	}

	stringstream ss;
	ss << "mongodb://" << u8Srv << ":" << port
		<< "?connectTimeoutMS=" << toConnectMS
		<< "&serverSelectionTimeoutMS=" << toSelectMS
		<< "&socketTimeoutMS=" << toSocketMS;
	string struri = ss.str();
	AutoUri uri(ss.str().c_str());
	if (!uri)
	{
		return -2;
	}
	mongoc_client_pool_t* pool = mongoc_client_pool_new(uri);
	if (!pool)
	{
		return -3;
	}
	m_srv = u8Srv;
	m_port = port;
	m_db = u8db;
	m_wdb = wdb;
	m_pool = pool;
	
	// test connection
	AutoPoolColl collection("fake");
	AutoBson query;
	int64_t count = collection.Count(query, 1);

	if (count < 0)
	{
		Clear();
		return -4;
	}

	return 0;
}



void MongoClient::Clear()
{
	m_srv = "";
	m_port = 0;
	m_db = "";
	if (m_pool)
	{
		mongoc_client_pool_destroy(m_pool);
		m_pool = nullptr;
	}
}


mongoc_client_t* MongoClient::TryPop(void)
{
	if (!m_pool)
	{
		return nullptr;
	}
	return mongoc_client_pool_try_pop(m_pool);
}

void MongoClient::Push(mongoc_client_t* client)
{
	if (!client)
	{
		return;
	}
	if (!m_pool)
	{
		mongoc_client_destroy(client);
		return;
	}
	mongoc_client_pool_push(m_pool, client);
}
//////////////////////////////////////////////////////////////////////////
// class instance
// NOT THREAD SAFE. call this once in single-threaded phase
MongoClient& GetMongoClient(void)
{
	static MongoClient mongoClient;
	return mongoClient;
}

//////////////////////////////////////////////////////////////////////////
// functions

// init mongo c driver. not thread safe. to
// locname is used in UNICODE <-> ANSI convention, "" to ignore, but conversion may fail.
// unsigned toXXXMS is time-out milliseconds for xxx, <= 0 is for default
// default: toConnectMS 10secs, toSelectMS 30secs, toSocketMS mins
// remember to create index after init if necessary.
int Init(const std::wstring& srv, const std::wstring& db, const std::string& locname,
	int toConnectMS /*= 0*/, int toSelectMS /*= 0*/, int toSocketMS /*= 0*/,
	unsigned short port /*= 27017*/)
{
	if (locname.length())
	{
		setlocale(LC_CTYPE, locname.c_str());
	}
	
	return GetMongoClient().MongoInit(srv, db, toConnectMS, port, toSelectMS, toSocketMS);
}

void Cleanup(void)
{
	GetMongoClient().Clear();
}

const char* GetDbName(void)
{
	return GetMongoClient().db();
}

// CAUTION: this will remove entire database from server.
bool DropDataBase(void)
{
	mongoc_client_t* client = TryPopClient();
	auto clientdel = [](mongoc_client_t* p) { PushClient(p); };
	std::unique_ptr<mongoc_client_t, decltype(clientdel)> upClient(client, clientdel);

	if (client)
	{
		auto databasedel = [](mongoc_database_t* p) { mongoc_database_destroy(p); };
		std::unique_ptr<mongoc_database_t, decltype(databasedel)> upDatabase(mongoc_client_get_database(client, GetMongoClient().db()), databasedel);
		bson_error_t err;
		if (!mongoc_database_drop(upDatabase.get(), &err))
		{
			return false;
		}
	}
	return true;
}

mongoc_client_t* TryPopClient(void)
{
	return GetMongoClient().TryPop();
}

void PushClient(mongoc_client_t* client)
{
	GetMongoClient().Push(client);
}


} // MongoClib

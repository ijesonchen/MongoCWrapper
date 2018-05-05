#include "mfcafx.h"
#include "MongoClient.h"

#include <sstream>
#include <mongoc.h>

#include "MongoAuto.h"
#include "MongocHelp.h"

#include "mfcnew.h"


//////////////////////////////////////////////////////////////////////////
// place this in main.app to link with lib
/*
#ifdef _MSC_VER
	#ifdef _WIN64 
		#ifdef _DEBUG
			#pragma comment(lib, "..\\..\\winlibmongoc\\lib\\x64Debug\\bson-1.0.lib")
			#pragma comment(lib, "..\\..\\winlibmongoc\\lib\\x64Debug\\mongoc-1.0.lib")
		#else
			#pragma comment(lib, "..\\..\\winlibmongoc\\lib\\x64Release\\bson-1.0.lib")		// x64RelWithDebInfo or x64Release
			#pragma comment(lib, "..\\..\\winlibmongoc\\lib\\x64Release\\mongoc-1.0.lib")	// x64RelWithDebInfo or x64Release
		#endif // _DEBUG
	#else
		#pragma comment(lib, "..\\..\\winlibmongoc\\lib\\x86Debug\\bson-1.0.lib")
		#pragma comment(lib, "..\\..\\winlibmongoc\\lib\\x86Debug\\mongoc-1.0.lib")
	#endif // _WIN64
#endif // _MSC_VER
*/

using namespace std;

namespace MongoClib
{
	const unsigned MongoTimeoutMillisec = 5000;
	const int MongoMaxPoolSize = 500;
	const int MongoMinPoolSize = 100;

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

	int MongoInit(const std::string& u8Srv, const std::string& u8db,
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

int MongoClient::MongoInit(const std::string& u8Srv, const std::string& u8db,
	int toConnectMS /*= MongoTimeoutMillisec*/, unsigned short port /*= 27017*/,
	int toSelectMS /*= 0*/, int toSocketMS /*= 0*/)
{
	if (m_pool)
	{
		return -1;
	}
	auto wdb = DecUtf8(u8db);
	if (!u8Srv.length() || !port || !u8db.length())
	{
		return -2;
	}

	stringstream ss;
	ss << "mongodb://" << u8Srv << ":" << port
		<< "/" << u8db
		<< "?connectTimeoutMS=" << toConnectMS
		<< "&serverSelectionTimeoutMS=" << toSelectMS
		<< "&socketTimeoutMS=" << toSocketMS
		<< "&maxPoolSize=" << MongoMaxPoolSize;
		// minPoolSize since 1.9.0 Deprecated. This option's behavior does not match its name,
		// and its actual behavior will likely hurt performance.
//		<< "&minPoolSize=" << MongoMinPoolSize; 
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
	long long count = 0;
	if (true)
	{
		AutoPoolColl collection("fake");
		AutoBson query;
		count = collection.Count(query, 1);
	}

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

// init mongo c driver. NOT thread safe. call this in single-threaded phase
// unsigned toXXXMS is time-out milliseconds for xxx, <= 0 is for default
// default: toConnectMS 10secs, toSelectMS 30secs, toSocketMS mins
// remember to create index after init if necessary.
// locname is obsoleted and ignored, because it may affect user's setting.

int Init(const std::string& u8Srv, const std::string& u8db,
	int toConnectMS /*= 0*/, int toSelectMS /*= 0*/, int toSocketMS /*= 0*/,
	unsigned short port /*= 27017*/)
{
	return GetMongoClient().MongoInit(u8Srv, u8db, toConnectMS, port, toSelectMS, toSocketMS);
}

int Init(const std::wstring& srv, const std::wstring& db,
	int toConnectMS /*= 0*/, int toSelectMS /*= 0*/, int toSocketMS /*= 0*/,
	unsigned short port /*= 27017*/)
{
	auto u8Srv = EncUtf8(srv);
	auto u8db = EncUtf8(db);
	return Init(u8Srv, u8db, toConnectMS, toSelectMS, toSocketMS, port);
}

int Init(const std::string& u8Srv, unsigned short port, const std::string& u8db)
{
	return Init(u8Srv, u8db, 0, 0, 0, port);
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
		bson_error_t err = {};
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

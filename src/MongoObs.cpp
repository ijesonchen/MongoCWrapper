#include "mfcafx.h"

#ifdef _MSC_VER
#include <codecvt>
#endif // _MSC_VER

#include "MongoObs.h"
#include "MongoAuto.h"
#include "MongoClient.h"

#include "mfcnew.h"

using namespace std;

namespace MongoClib
{
	//////////////////////////////////////////////////////////////////////////
	// AutoClient
	// wrapper for mongoc_client_t*
	AutoClient::AutoClient(mongoc_client_t* p) 
		: m_p(p) 
	{ 
	};

	AutoClient::AutoClient(const std::string& srv)
	{
		m_p = mongoc_client_new(srv.c_str());
	};

	AutoClient::~AutoClient() 
	{ 
		if (m_p) 
			mongoc_client_destroy(m_p); 
	};

	AutoClient::operator mongoc_client_t* ()
	{ 
		return m_p; 
	}

	//////////////////////////////////////////////////////////////////////////
	// AutoColl
	// wrapper for mongoc_collection_t*
	AutoColl::AutoColl(mongoc_collection_t* p) 
		: m_p(p) 
	{ };

	AutoColl::AutoColl(mongoc_client_t* client, std::string& db, std::string& coll)
	{
		m_p = mongoc_client_get_collection(client, db.c_str(), coll.c_str());
	};

	AutoColl::~AutoColl() 
	{ 
		if (m_p) 
			mongoc_collection_destroy(m_p); 
	};

	AutoColl::operator mongoc_collection_t* () 
	{ 
		return m_p; 
	}

	mongoc_collection_t* AutoColl::Release(void)
	{
		mongoc_collection_t* coll = m_p;
		m_p = nullptr;
		return coll;
	}


	//////////////////////////////////////////////////////////////////////////
	// AutoPoolClient
	// wrapper for mongoc_client_t* from client's pool

	AutoPoolClient::AutoPoolClient()
	{
		m_p = TryPopClient();
	}

	AutoPoolClient::~AutoPoolClient()
	{
		// push will check null client
		PushClient(m_p);
	}


	AutoPoolClient::operator mongoc_client_t* () const
	{
		return m_p;
	}

	mongoc_client_t* AutoPoolClient::Release(void)
	{
		mongoc_client_t* client = m_p;
		m_p = nullptr;
		return client;
	}

	//////////////////////////////////////////////////////////////////////////
	// Create Index
	bool CreateIndex(mongoc_collection_t *collection, const std::string& field, bool bAsc /*= true*/, bool bUnique /*= false*/, std::string* pError /*= nullptr*/)
	{
		int nAsc = bAsc ? 1 : -1;
		AutoBson keys(BCON_NEW(field.c_str(), BCON_INT32(nAsc)));
		if (!keys)
		{
			return false;
		}

		bson_error_t error;
		mongoc_index_opt_t opt;
		mongoc_index_opt_init(&opt);
		opt.unique = bUnique;

		if (!mongoc_collection_create_index(collection, keys, &opt, &error))
		{
			if (pError)
			{
				*pError = error.message;
			}
			return false;
		}

		return true;
	}


	bool CreateIndex(mongoc_client_t* client, const std::string& coll, const std::string& field, bool bAsc/* = true*/, bool bUnique /*= false*/, std::string* pError /*= nullptr*/)
	{
		AutoColl collection(mongoc_client_get_collection(client, GetDbName(), coll.c_str()));
		if (!collection)
		{
			return false;
		}
		return CreateIndex(collection, field, bAsc, bUnique, pError);
	}

	bool CreateIndex(const std::string& coll, const std::string& field, bool bAsc /*= true*/, bool bUnique /*= false*/, std::string* pError /*= nullptr*/)
	{
		if (!coll.length() || !field.length())
		{
			return false;
		}
		mongoc_client_t* client = TryPopClient();
		if (!client)
		{
			return false;
		}

		return CreateIndex(client, coll, field, bAsc, bUnique, pError);
	}

#ifdef _MSC_VER
	//////////////////////////////////////////////////////////////////////////
	// UTF-8 <-> UCS convert with STL (C++11).
	// obsoleted: 
	// 1. not supported by Linux (by 2015)
	// 2. slower than MongoClib:: DecUtf8 / EncUtf8
	std::string toU8(const std::wstring& strUnicode)
	{
		std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t > convertor;
		return convertor.to_bytes(strUnicode);
	}

	std::wstring fromU8(const std::string& strUtf8)
	{
		std::wstring_convert < std::codecvt_utf8<wchar_t>, wchar_t > convertor;
		return convertor.from_bytes(strUtf8);
	}
#endif

} // MongoClib
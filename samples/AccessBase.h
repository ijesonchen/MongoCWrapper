#ifndef AccessBase_h__
#define AccessBase_h__

#include <iostream>
#include <deque>
#include "MongoClient.h"
#include "Account.h"

// EmplaceBackCntr is STL container supports emplace_back
// Object is access target class

namespace MongoClib
{
	static const int knAccountPerPeople = 5;

	bool CreateAllIndces(void);
	
template<class Object>
class AccessBase
{

	AccessBase(AccessBase& rhs) = delete;
	AccessBase(AccessBase&& rhs) = delete;
	AccessBase& operator = (AccessBase& rhs) = delete;
	AccessBase& operator = (AccessBase&& rhs) = delete;
public:
	AccessBase(const std::string& collname) 
		: m_collname(collname), m_collnameW(DecAnsi(collname.c_str())) 
	{ };

	virtual ~AccessBase() {};

private:
	template<typename T>
	void Log(std::wstringstream& wss, const T& t)
	{
		wss << t;
	}

	template<typename T, typename... Args>
	void Log(std::wstringstream& wss, const T& t, const Args& ... rest)
	{
		wss << t << L", ";
		Log(wss, rest...);
	}

	template<typename... Args>
	void LogError(const std::wstring& oper, const Args& ... rest)
	{
		std::wstringstream wss;
		wss << m_collnameW << L": " << oper << ": failed, ";
		Log(wss, rest...);
		std::wcout << L"[error]" << wss.str() << std::endl;
	}

	void LogError(const std::wstring& oper)
	{
		std::wstringstream wss;
		wss << m_collnameW << L": " << oper << ": failed.";
		std::wcout << L"[error]" << wss.str() << std::endl;
	}

protected:
	const std::string m_collname;
	const std::wstring m_collnameW;
	
	template<class EmplaceBackCntr>
	bool QueryLoad(EmplaceBackCntr& objects, const bson_t* query, int nMax = 0)
	{
		AutoCursor cursor(m_collname, query, nMax);
		if (!cursor)
		{
			LogError(L"QueryLoad");
			return false;
		}

		while (cursor.Next()) {
			objects.emplace_back();
			if (!ParseBson(objects.back(), cursor))
			{
				objects.pop_back();
				return false;
			}
		}

		if (!objects.size())
		{
			AutoJson json(query);
			std::wstringstream wss;
			wss << m_collnameW << L": QueryLoad no result, query: " << MongoClib::DecUtf8(json);

			std::wcout << L"[warning]" <<  wss.str() << std::endl;
		}

		return true;
	}

	template<class EmplaceBackCntr>
	bool QueryLoad(EmplaceBackCntr& objects, int nMax = 0)
	{
		MongoClib::AutoBson query;
		return QueryLoad(objects, query, nMax);
	}

	bool QueryOne(Object& obj, const bson_t* query)
	{
		std::deque<Object> dqObjects;
		if (!QueryLoad(dqObjects, query, 1))
		{
			return false;
		}
		if (!dqObjects.size())
		{
			LogError(L"QueryOne", L"NoResult");
			return false;
		}
		obj = dqObjects.front();
		return true;
	}
	
public:
	bool Insert(const Object& obj)
	{
		AutoBson doc(BuildBson(obj));
		if (!doc)
		{
			LogError(L"BuildBson");
			return false;
		}

		AutoPoolColl coll(m_collname);
		if (!coll.Insert(doc))
		{
			LogError(L"Insert", coll.WError());
			return false;
		}
		return true;
	}

	bool Remove(const bson_t* query)
	{
		AutoPoolColl coll(m_collname);
		if (!coll.Delete(query))
		{
			LogError(L"Delete", coll.WError());
			return false;
		}
		return true;
	}

	bool MultiUpdate(const bson_t* query, const bson_t* update)
	{
		AutoPoolColl coll(m_collname);
		if (!coll.MultiUpdate(query, update))
		{
			LogError(L"MultiUpdate", coll.WError());
			return false;
		}
		return true;
	}

	bool Modify(const bson_t* query, const bson_t* update)
	{
		AutoPoolColl coll(m_collname);
		if (!coll.Modify(query, update))
		{
			LogError(L"Modify", coll.WError());
			return false;
		}
		return true;
	}

	long long Count(const bson_t* query)
	{
		AutoPoolColl coll(m_collname);
		long long cnt = coll.Count(query);
		if (cnt < 0)
		{
			LogError(L"Count", coll.WError());
		}
		return cnt;
	}

	bool Rename(const std::string newCollName)
	{
		AutoPoolColl coll(m_collname);
		if (!coll.Rename(newCollName))
		{
			LogError(L"Rename", coll.WError());
			return false;
		}
		return true;
	}

	template <typename Container>
	static bool AddAccount(AutoBson& doc, const std::string& key, const Container& val)
	{
		ArrayBuilder docarray;
		if (!docarray)
		{
			return false;
		}
		for (auto it : val)
		{
			AutoBson sub;
			if (!sub)
			{
				return false;
			}
			sub.Add("guid", it.guid);
			sub.Add("name", it.name);
			sub.Add("memo", it.memo);
			sub.Add("inuse", it.inuse);
			docarray.AddSub(sub);
			if (doc.IsFailed())
			{
				return false;
			}
		}
		doc.AddArray(key, docarray);
		if (doc.IsFailed())
		{
			return false;
		}
		return true;
	}

	template <typename Container>
	static bool GetAccount(const MongoClib::AutoCursor& cursor, const std::string& key, Container& val)
	{
		ArrayParser arrayParser(cursor.ResultBson(), key);
		if (!arrayParser.HasElement())
		{
			return false;
		}
		auto inserter = back_inserter(val);
		while (arrayParser.Next())
		{
			if (val.size() > knAccountPerPeople)
			{
				break;
			}
			Account acc;
			acc.guid = arrayParser.Int64("guid");
			acc.name = arrayParser.Str("name");
			acc.memo = arrayParser.Str("memo");
			acc.inuse = arrayParser.Bool("inuse");
			*inserter++ = acc;
		}
		return true;
	}

	std::vector<Account> GetAccount(const MongoClib::AutoCursor& cursor, const std::string& key)
	{
		std::vector<Account> tels;
		if (!GetAccount(cursor, key, tels))
		{
			LogError(L"GetCTelNumber", L"cursor has no result or field not exist.");
		}
		while (tels.size() < knAccountPerPeople)
		{
			tels.push_back(Account());
		}
		return std::move(tels);
	}
	

protected:
	// 接口必须实现
	virtual bool ParseBson(Object&, const MongoClib::AutoCursor&) = 0;
	virtual MongoClib::AutoBson BuildBson(const Object&) = 0;
public:
	virtual bool CreateIndexs(void) = 0;
};


} // MongoClib

#endif // AccessBase_h__

#ifndef MongocHelp_h__
#define MongocHelp_h__

#include <string>
#include <sstream>
#include <vector>
#include <memory>
#include <iterator>
#include <tuple>
#include <bson.h>
#include <mongoc.h>

//////////////////////////////////////////////////////////////////////////
// help help class
// try to used wrapper class instead of parser & build function.

namespace MongoClib
{
	//////////////////////////////////////////////////////////////////////////
	// string conv function
	std::string Timet2String(time_t tm);
	std::string U32toString(const std::uint32_t n);

	//////////////////////////////////////////////////////////////////////////
	// encode / decode functions, UCS2 <-> UTF-8
	std::string EncUtf8(const std::wstring& strUnicode);
	std::wstring DecUtf8(const std::string& strUtf8);


#ifdef __AFXSTR_H__
	inline std::string EncUtf8MS(const CString& strUnicode) 
		{ return EncUtf8(strUnicode.GetString()); };
	inline CString DecUtf8MS(const std::string& strUtf8) 
		{ return DecUtf8(strUtf8).c_str(); };
#endif // __AFXSTR_H__

#ifdef _MSC_VER
	std::string EncAnsi(const std::wstring& strUnicode);
	std::wstring DecAnsi(const std::string& strAnsi);

	inline std::string Utf8ToAnsi(const std::string& strUtf8)
		{ return EncAnsi(DecUtf8(strUtf8));	}
	inline std::string AnsiToUtf8(const std::string& strAnsi)
		{ return EncUtf8(DecAnsi(strAnsi)); }
#endif // _MSC_VER

	//////////////////////////////////////////////////////////////////////////
	// bson parser 
	bool BsonHasKey(const bson_t* doc, const std::string& key, bool& val);
	bool BsonKeyIter(const bson_t* doc, const std::string& key, bson_iter_t& ival);

	bool BsonValue(const bson_t* doc, const std::string& key, bool& val);
	bool BsonValue(const bson_t* doc, const std::string& key, int& val);
	bool BsonValue(const bson_t* doc, const std::string& key, long long& val);
	bool BsonValue(const bson_t* doc, const std::string& key, double& val);
	bool BsonValue(const bson_t* doc, const std::string& key, std::string& val);
	// try not to differ time_t with int64. 
	// mongo use int64 to store date-time in milliseconds since the UNIX epoch, NOT time_t
	bool BsonValueTimet(const bson_t* doc, const std::string& key, time_t& val);
	bool BsonValueBin(const bson_t* doc, const std::string& key, std::string& val);
	// sub document val should be destroyed.
	bool BsonDoc(const bson_t* doc, const std::string& key, bson_t*& val);
	bool BsonJson(const bson_t* doc, const std::string& key, std::string& val);
	bool BsonOid(const bson_t* doc, const std::string& key, std::string& val);
	bool BsonOidTime(const bson_t* doc, const std::string& key, time_t& val);

	//////////////////////////////////////////////////////////////////////////
	// simple bson parser.
	// if failed, return default value
	bool
		BsonValueBool(const bson_t* doc, const std::string& key);
	int
		BsonValueInt32(const bson_t* doc, const std::string& key);
	long long
		BsonValueInt64(const bson_t* doc, const std::string& key);
	double
		BsonValueDouble(const bson_t* doc, const std::string& key);
	std::string
		BsonValStr(const bson_t* doc, const std::string& key);
	std::string
		BsonValBin(const bson_t* doc, const std::string& key);
	bson_t*
		BsonValDoc(const bson_t* doc, const std::string& key);
	std::string
		BsonValJson(const bson_t* doc, const std::string& key);
	std::string
		BsonValOid(const bson_t* doc, const std::string& key);

	//////////////////////////////////////////////////////////////////////////
	// TransFunc, used for bson template function
	const auto str2string = [](const std::string& str)->std::string { return str; };
	const auto str2wstring = [](const std::string& str)->std::wstring { return DecUtf8(str); };
	const auto wstring2str = [](const std::wstring& str)->std::string { return EncUtf8(str); };

#ifdef __AFXSTR_H__
	const auto str2cstring = [](const std::string& str)->CString { return DecUtf8MS(str); };
	const auto cstring2str = [](const CString& str)->std::string { return EncUtf8MS(str); };
#endif // __AFXSTR_H__
	

	//////////////////////////////////////////////////////////////////////////
	// bson template function, operate with container

	// add cntr to bson, cntr must support for_each
	template<class Container, class TransFunc>
	bool BsonAddContainer(bson_t* doc, const std::string& key, Container& cntr, TransFunc transFunc);

	// get key from bson to cntr, cntr must support back_inserter
	template<class Container, class TransFunc>
	bool BsonGetContainer(const bson_t* doc, const std::string& key, Container& cntr, TransFunc transFunc);
		

	//////////////////////////////////////////////////////////////////////////
	// template body
	template<class Container, class TransFunc>
	bool BsonAddContainer(bson_t* doc, const std::string& key, Container& cntr, TransFunc transFunc)
	{
		// if cntr is empty, insert an empty array.
		if (!doc) return false;
		unsigned idx = (unsigned)-1;
		bson_t* arr = bson_new();
		for (auto it : cntr)
		{
			std::string idxvalue = transFunc(it);
			auto idxstr = U32toString(++idx);
			BSON_APPEND_UTF8(arr, idxstr.c_str(), idxvalue.c_str());
		}
		bool bRet = BSON_APPEND_ARRAY(doc, key.c_str(), arr);
		bson_destroy(arr);
		return bRet;
	}

	template<class Container, class TransFunc>
	bool BsonGetContainer(const bson_t* doc, const std::string& key, Container& cntr, TransFunc transFunc)
	{
		if (!doc) return false;

		bson_iter_t ival;
		uint32_t len = 0;

		auto inserter = std::back_inserter(cntr);

		if (BsonKeyIter(doc, key, ival) && BSON_ITER_HOLDS_ARRAY(&ival))
		{
			bson_iter_t ichild;
			if (bson_iter_recurse(&ival, &ichild))
			{
				while (bson_iter_next(&ichild))
				{
					if (!BSON_ITER_HOLDS_UTF8(&ichild))
					{
						return false;
					}
					std::string str = bson_iter_utf8(&ichild, &len);
					*inserter++ = transFunc(str);
				}
				return true;
			}
		}
		return false;
	}

	template<class Container>
	bool BsonBinContainer(const bson_t* doc, const std::string& key, Container& cntr)
	{
		if (!doc) return false;

		bson_iter_t ival;

		auto inserter = std::back_inserter(cntr);

		if (BsonKeyIter(doc, key, ival) && BSON_ITER_HOLDS_ARRAY(&ival))
		{
			bson_iter_t ichild;
			if (bson_iter_recurse(&ival, &ichild))
			{
				std::string val;
				while (bson_iter_next(&ichild))
				{
					if (!BSON_ITER_HOLDS_BINARY(&ichild))
					{
						return false;
					}

					bson_subtype_t  subtype;
					uint32_t        len = 0;
					const uint8_t*	buf = nullptr;
					bson_iter_binary(&ichild, &subtype, &len, &buf);
					if (!buf || !len)
					{
						return false;
					}
					val.assign((const char*)buf, len);

					*inserter++ = val;
				}
				return true;
			}
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	// read json file & convert to bson

	template<class Container>
	bool ReadJsonFile(const std::string& jsonFileName, Container& cntr, std::string& errMsg)
	{
		bson_error_t bsonErr;
		auto pReader = bson_json_reader_new_from_file(jsonFileName.c_str(), &bsonErr);
		if (!pReader)
		{
			errMsg = bsonErr.message;
			return false;
		}
		shared_ptr <bson_json_reader_t> up(pReader, bson_json_reader_destroy);

		bson_t doc = BSON_INITIALIZER;
		auto inserter = std::back_inserter(cntr);
		auto rc = 0;
		while ((rc = bson_json_reader_read(pReader, &doc, &bsonErr)) > 0)
		{
			*inserter++ = doc;
		}

		if (rc < 0)
		{
			errMsg = bsonErr.message;
			return false;
		}
		return true;
	}

	class AutoBson;
	std::tuple<std::vector<AutoBson>, std::string>
		ReadJsonFile(const std::string& jsonFileName);

	std::tuple<AutoBson, std::string> ReadJsonFileFirst(const std::string& jsonFileName);
	std::tuple<AutoBson, std::string> ReadJsonFileLast(const std::string& jsonFileName);
} // MongoClib

#endif // MongocHelp_h__

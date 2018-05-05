#include "mfcafx.h"
#include "MongocHelp.h"

#include <mongoc.h>
#include <iostream>
#include <sstream>

#include "MongoAuto.h"

#include "mfcnew.h"

using namespace std;

namespace MongoClib
{

#ifdef _MSC_VER
	#pragma warning(disable: 4996)
#endif

	std::string Timet2String(time_t tm)
	{
		auto tmStru = localtime(&tm);
		const auto bufLen = 80;
		char buffer[bufLen];
		strftime(buffer, bufLen, "%Y-%m-%d %H:%M:%S", tmStru);
		buffer[bufLen - 1] = 0;
		return buffer;
	}

	std::string U32toString(const std::uint32_t n)
	{
		char str[16];
		const char *res;
		bson_uint32_to_string(n, &res, str, sizeof(str));
		return res;
	}

#ifdef _MSC_VER
	#pragma warning(default :4996)
#endif

	// Return length of string. Must be NULL terminated.
	// If length >= 1G, return -1; terminate NULL not counted
	template <typename T>
	int StringLength1G(const T* psz)
	{
		int nLength = 0;
		while (psz[nLength++])
		{
			if (nLength >= 0x40000000)
			{
				return -1;
			}
		}
		return --nLength;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// UTF-8 <-> UCS2 transform function
	// Return data write count.
	// If pBuffer is NULL, return required buffer length.
	// Whether NULL terminator included lies on input.

	// input must NULL terminated.
	int UnicodeToUtf8(const wchar_t* pszUnicode, char* pUtf8, int nUtf8Len);
	// function does not check NULL terminator.
	int UnicodeToUtf8(const wchar_t* pUnicode, int nUnicodeBufLen, char* pUtf8, int nUtf8Len);
	// input must NULL terminated.
	int Utf8ToUnicode(const char* pszUtf8, wchar_t* pUnicode, int nUnicodeBufLen);
	// function does not check NULL terminator.
	int Utf8ToUnicode(const char* pUtf8, int nUtf8Len, wchar_t* pUnicode, int nUnicodeBufLen);
		
	// input must NULL terminated.
	int UnicodeToUtf8(const wchar_t* pszUnicode, char* pUtf8, int nUtf8Len)
	{
		int nInputLen = StringLength1G(pszUnicode) + 1;
		if (nInputLen <= 0)
		{
			return -1;
		}
		return UnicodeToUtf8(pszUnicode, nInputLen, pUtf8, nUtf8Len);
	}

	// function does not check NULL terminator.
	int UnicodeToUtf8(const wchar_t* pUnicode, int nUnicodeBufLen, char* pUtf8, int nUtf8Len)
	{
		if (!pUnicode)
		{
			return -1;
		}

		int nTotal = 0;
		for (int ii = 0; ii < nUnicodeBufLen; ++ii)
		{
			wchar_t wch = pUnicode[ii];
			if (wch <= 0x007F)
			{
				nTotal += 1;
			}
			else if (wch <= 0x07FF)
			{
				nTotal += 2;
			}
			else if (wch <= 0xFFFF)
			{
				nTotal += 3;
			}
		}

		if (!pUtf8)
		{
			return nTotal;
		}
		*pUtf8 = 0;
		if (nUtf8Len < nTotal)
		{
			return -2;
		}

		// utf8 write index
		int nwUtf8 = 0;
		for (int ii = 0; ii < nUnicodeBufLen; ++ii)
		{
			wchar_t wch = pUnicode[ii];
			if (wch <= 0x007F)
			{
				if (nwUtf8 > nUtf8Len - 1)
				{
					return -3;
				}
				// 0xxxxxxx
				pUtf8[nwUtf8++] = (char)wch;
			}
			else if (wch <= 0x07FF)
			{
				if (nwUtf8 > nUtf8Len - 2)
				{
					return -3;
				}
				// 110xxxxx 10xxxxxx
				pUtf8[nwUtf8++] = (char)((wch >> 6) | 0xC0);
				pUtf8[nwUtf8++] = (char)((wch & 0x3F) | 0x80);
			}
			else if (wch <= 0xFFFF)
			{
				if (nwUtf8 > nUtf8Len - 3)
				{
					return -3;
				}
				// 1110xxxx 10xxxxxx 10xxxxxx
				pUtf8[nwUtf8++] = (char)((wch >> 12) | 0xE0);
				pUtf8[nwUtf8++] = (char)(((wch >> 6) & 0x3F) | 0x80);
				pUtf8[nwUtf8++] = (char)((wch & 0x3F) | 0x80);
			}
		}

		return nwUtf8;
	}

	// input must NULL terminated.
	int Utf8ToUnicode(const char* pszUtf8, wchar_t* pUnicode, int nUnicodeBufLen)
	{
		int nInputLen = StringLength1G(pszUtf8) + 1;
		if (nInputLen <= 0)
		{
			return -1;
		}
		return Utf8ToUnicode(pszUtf8, nInputLen, pUnicode, nUnicodeBufLen);
	}

	int Utf8CharCount(const char* pUtf8)
	{
		if (!pUtf8)
		{
			return -1;
		}
		int nCharCount = 0;
		char ch = *pUtf8;
		while (ch & 0x80)
		{
			++nCharCount;
			ch <<= 1;
		}

		if (nCharCount == 0)
		{
			// <= 0x007F
			return 1;
		}
		else if (nCharCount > 3)
		{
			// not UCS2
			return -2;
		}

		for (int ii = 0; ii < nCharCount - 1; ++ii)
		{
			char CH = pUtf8[1 + ii];
			if ((CH & 0xC0) ^ 0x80)
			{
				// followed not 10xxxxxx
				return -3;
			}
		}

		return nCharCount;
	}

	int Unf8ToWchar(const char* pUtf8, wchar_t& wch)
	{
		int nUtf8Count = Utf8CharCount(pUtf8);
		wch = 0;
		switch (nUtf8Count)
		{
		case 1:
			wch = *pUtf8;
			break;
		case 2:
			wch |= ((pUtf8[0] & 0x1F) << 6);
			wch |= (pUtf8[1] & 0x3F);
			break;
		case 3:
			wch |= ((pUtf8[0] & 0x0F) << 12);
			wch |= ((pUtf8[1] & 0x3F) << 6);
			wch |= (pUtf8[2] & 0x3F);
			break;
		default:
			wch = (wchar_t)-1;
			nUtf8Count -= 100;
			break;
		}

		return nUtf8Count;
	}

	// function does not check NULL terminator.
	int Utf8ToUnicode(const char* pUtf8, int nUtf8Len, wchar_t* pUnicode, int nUnicodeBufLen)
	{
		if (!pUtf8)
		{
			// parameter failed.
			return -1;
		}

		int nTotalUtf = 0;
		int nTotalUni = 0;
		for (nTotalUtf = 0; nTotalUtf < nUtf8Len;)
		{
			int nTemp = Utf8CharCount(pUtf8 + nTotalUtf);
			if (nTemp < 0)
			{
				// utf8 check failed.
				return -2;
			}
			nTotalUtf += nTemp;
			++nTotalUni;
		}
		if (!pUnicode)
		{
			return nTotalUni;
		}
		*pUnicode = 0;
		if (nUnicodeBufLen < nTotalUni)
		{
			return -1;
		}

		int nIdxUtf = 0;
		for (int ii = 0; ii < nTotalUni; ++ii)
		{
			if (nIdxUtf >= nUtf8Len)
			{
				return -2;
			}
			int nUtfCount = Unf8ToWchar(pUtf8 + nIdxUtf, pUnicode[ii]);
			if (nUtfCount <= 0)
			{
				return -3;
			}
			nIdxUtf += nUtfCount;
		}
		return nTotalUni;
	}

	// string (UTF-8) -> wstring (unicode)
	std::string EncUtf8(const std::wstring& strUnicode)
	{
		const wchar_t* pw = strUnicode.c_str();
		int nUnicode = static_cast<int>(strUnicode.length());
		int nUtf8 = UnicodeToUtf8(pw, nUnicode, NULL, 0);
		if (nUtf8 <= 0)
		{
			return "";
		}
		char* p = new char[nUtf8];
		std::auto_ptr<char> ap(p);
		memset(p, 0, nUtf8 * sizeof(char));
		int nRet = UnicodeToUtf8(pw, nUnicode, p, nUtf8);
		if (nRet <= 0)
		{
			return "";
		}
		std::string utf8;
		utf8.assign(p, nUtf8);
		return utf8;
	}

	std::wstring DecUtf8(const std::string& strUtf8)
	{
		const char* p = strUtf8.c_str();
		int nUtf8 = static_cast<int>(strUtf8.length());
		int nUnicode = Utf8ToUnicode(p, nUtf8, NULL, 0);
		if (nUnicode <= 0)
		{
			return L"";
		}
		wchar_t* pw = new wchar_t[nUnicode];
		std::auto_ptr<wchar_t> ap(pw);
		memset(pw, 0, nUnicode * sizeof(wchar_t));
		int nRet = Utf8ToUnicode(p, nUtf8, pw, nUnicode);
		if (nRet <= 0)
		{
			return L"";
		}
		std::wstring unicode;
		unicode.assign(pw, nUnicode);
		return unicode;
	}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996)
	
	// wcstombs/mbstowcs is global locale related. wcstombs_l is system dependent. with Linux, try iconv
	// wstring_convert use codecvt, which g++ not support (should use clang)
	std::string EncAnsi(const std::wstring& strUnicode)
	{
		const wchar_t* pUnicode = strUnicode.c_str();
		auto len = WideCharToMultiByte(CP_ACP, 0, pUnicode, (int)strUnicode.length(), nullptr, 0, nullptr, nullptr);
		if (!len)
		{
			return "";
		}
		char* ap = new char[len + 1];
		std::unique_ptr<char> aup(ap);
		ap[0] = 0;
		ap[len - 1] = 0;
		ap[len] = 0;


		len = WideCharToMultiByte(CP_ACP, 0, pUnicode, (int)strUnicode.length(), ap, len, nullptr, nullptr);

		if (len == (size_t)-1)
		{
			return "";
		}
		return ap;
	}

	std::wstring DecAnsi(const std::string& strAnsi)
	{
		const char* pAnsi = strAnsi.c_str();
		auto len = MultiByteToWideChar(CP_ACP, 0, pAnsi, (int)strAnsi.length(), nullptr, 0);

		if (!len)
		{
			return L"";
		}
		wchar_t* wp = new wchar_t[len + 1];
		std::unique_ptr<wchar_t> wup(wp);
		wp[0] = 0;
		wp[len - 1] = 0;
		wp[len] = 0;

		len = MultiByteToWideChar(CP_ACP, 0, pAnsi, (int)strAnsi.length(), wp, len);

		if (!len)
		{
			return L"";
		}
		return wp;
	}
#endif // _MSC_VER

#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER
	
	//////////////////////////////////////////////////////////////////////////
	// bson parser functions
	bool BsonHasKey(const bson_t* doc, const std::string& key, bool& val)
	{
		if (!doc) return false;

		bson_iter_t iter;
		if (bson_iter_init(&iter, doc))
		{
			val = bson_iter_find_case(&iter, key.c_str());
			return true;
		}
		return false;
	}

	bool BsonKeyIter(const bson_t* doc, const std::string& key, bson_iter_t& ival)
	{
		if (!doc) return false;

		bson_iter_t iter;
		if (bson_iter_init(&iter, doc))
		{
			if (bson_iter_find_descendant(&iter, key.c_str(), &ival))
			{
				return true;
			}
		}
		return false;
	}

	bool BsonDoc(const bson_t* doc, const std::string& key, bson_t*& val)
	{
		if (!doc) return false;

		if (val)
		{
			bson_destroy(val);
			val = nullptr;
		}

		bson_iter_t ival;

		if (BsonKeyIter(doc, key, ival) && BSON_ITER_HOLDS_DOCUMENT(&ival))
		{
			uint8_t* pdoc = nullptr;
			uint32_t ndoc = 0;
			bson_iter_document(&ival, &ndoc, (const uint8_t**)&pdoc);
			val = bson_new_from_data(pdoc, ndoc);
			return true;
		}
		return false;
	}

	bool BsonJson(const bson_t* doc, const std::string& key, std::string& val)
	{
		if (!doc) return false;

		bson_iter_t ival;

		uint8_t* pdoc = nullptr;
		if (BsonKeyIter(doc, key, ival) && BSON_ITER_HOLDS_DOCUMENT(&ival))
		{
			uint32_t ndoc = 0;
			bson_iter_document(&ival, &ndoc, (const uint8_t**)&pdoc);
			AutoBson subdoc(bson_new_from_data(pdoc, ndoc));
			if (subdoc)
			{
				AutoJson json(subdoc);
				val = json;
				return true;
			}
		}
		return false;
	}

	bool BsonOid(const bson_t* doc, const std::string& key, std::string& val)
	{
		if (!doc) return false;

		bson_iter_t ival;

		if (BsonKeyIter(doc, key, ival) && BSON_ITER_HOLDS_OID(&ival))
		{
			char str[25];
			bson_oid_to_string(bson_iter_oid(&ival), str);
			val = str;
			return true;
		}
		return false;
	}

	bool BsonOidTime(const bson_t* doc, const std::string& key, time_t& val)
	{
		if (!doc) return false;

		bson_iter_t ival;

		if (BsonKeyIter(doc, key, ival) && BSON_ITER_HOLDS_OID(&ival))
		{
			val = bson_oid_get_time_t(bson_iter_oid(&ival));
			return true;
		}
		return false;
	}

	bool BsonValue(const bson_t* doc, const std::string& key, bool& val)
	{
		if (!doc) return false;

		bson_iter_t ival;

		if (BsonKeyIter(doc, key, ival) && BSON_ITER_HOLDS_BOOL(&ival))
		{
			val = bson_iter_bool(&ival);
			return true;
		}
		return false;
	}

	bool BsonValue(const bson_t* doc, const std::string& key, int& val)
	{
		if (!doc) return false;

		bson_iter_t ival;

		if (BsonKeyIter(doc, key, ival) && BSON_ITER_HOLDS_INT32(&ival))
		{
			val = bson_iter_int32(&ival);
			return true;
		}
		return false;
	}

	bool BsonValue(const bson_t* doc, const std::string& key, long long& val)
	{
		if (!doc) return false;

		bson_iter_t ival;

		if (BsonKeyIter(doc, key, ival) && BSON_ITER_HOLDS_INT64(&ival))
		{
			val = bson_iter_int64(&ival);
			return true;
		}
		return false;
	}

	bool BsonValue(const bson_t* doc, const std::string& key, double& val)
	{
		if (!doc) return false;

		bson_iter_t ival;

		if (BsonKeyIter(doc, key, ival) && BSON_ITER_HOLDS_DOUBLE(&ival))
		{
			val = bson_iter_double(&ival);
			return true;
		}
		return false;
	}

	bool BsonValue(const bson_t* doc, const std::string& key, std::string& val)
	{
		if (!doc) return false;

		bson_iter_t ival;
		uint32_t len = 0;

		if (BsonKeyIter(doc, key, ival) && BSON_ITER_HOLDS_UTF8(&ival))
		{
			val = bson_iter_utf8(&ival, &len);
			return true;
		}
		return false;
	}

	bool BsonValueTimet(const bson_t* doc, const std::string& key, time_t& val)
	{
		if (!doc) return false;

		bson_iter_t ival;

		if (BsonKeyIter(doc, key, ival) && BSON_ITER_HOLDS_DATE_TIME(&ival))
		{
			val = bson_iter_time_t(&ival);
			return true;
		}
		return false;
	}

	bool BsonValueBin(const bson_t* doc, const std::string& key, std::string& val)
	{
		if (!doc) return false;

		bson_iter_t ival;

		if (BsonKeyIter(doc, key, ival) && BSON_ITER_HOLDS_BINARY(&ival))
		{
			bson_subtype_t  subtype;
			uint32_t        len = 0;
			const uint8_t*	buf = nullptr;
			bson_iter_binary(&ival, &subtype, &len, &buf);
			if (buf && len)
			{
				val.assign((const char*)buf, len);
				return true;
			}
			return false;
		}
		return false;
	}
	
	bool BsonValueBool(const bson_t* doc, const std::string& key)
	{
		bool val = 0;
		BsonValue(doc, key, val);
		return val;
	}

	int BsonValueInt32(const bson_t* doc, const std::string& key)
	{
		int val = 0;
		BsonValue(doc, key, val);
		return val;
	}

	long long BsonValueInt64(const bson_t* doc, const std::string& key)
	{
		long long val = 0;
		BsonValue(doc, key, val);
		return val;
	}

	double BsonValueDouble(const bson_t* doc, const std::string& key)
	{
		double val = 0;
		BsonValue(doc, key, val);
		return val;
	}

	std::string BsonValStr(const bson_t* doc, const std::string& key)
	{
		std::string val;
		BsonValue(doc, key, val);
		return val;
	}

	std::string BsonValBin(const bson_t* doc, const std::string& key)
	{
		std::string val;
		BsonValueBin(doc, key, val);
		return val;
	}

	bson_t* BsonValDoc(const bson_t* doc, const std::string& key)
	{
		bson_t* val = nullptr;
		BsonDoc(doc, key, val);
		return val;
	}

	std::string
		BsonValJson(const bson_t* doc, const std::string& key)
	{
		std::string val;
		BsonJson(doc, key, val);
		return std::move(val);
	}

	std::string
		BsonValOid(const bson_t* doc, const std::string& key)
	{
		std::string val;
		BsonOid(doc, key, val);
		return std::move(val);
	}
	
	std::tuple<std::vector<AutoBson>, std::string>
		ReadJsonFile(const std::string& jsonFileName)
	{
		std::vector<AutoBson> v;
		string s;
		ReadJsonFile(jsonFileName, v, s);
		return std::make_tuple(std::move(v), s);
	}
	
	std::tuple<AutoBson, std::string>
		ReadJsonFileFirst(const std::string& jsonFileName)
	{
		std::vector<AutoBson> v;
		std::string s;
		std::tie(v, s) = ReadJsonFile(jsonFileName);
		if (v.empty())
		{
			return make_tuple(std::move(AutoBson()), s);
		}
		return make_tuple(std::move(v.front()), s);
	}

	std::tuple<AutoBson, std::string>
		ReadJsonFileLast(const std::string& jsonFileName)
	{
		std::vector<AutoBson> v;
		std::string s;
		std::tie(v, s) = ReadJsonFile(jsonFileName);
		if (v.empty())
		{
			return make_tuple(std::move(AutoBson()), s);
		}
		return make_tuple(std::move(v.back()), s);
	}
} // MongoClib
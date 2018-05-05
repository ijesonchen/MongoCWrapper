#ifndef MongoLog_h__
#define MongoLog_h__

#include <string>
#include <sstream>
#include <iostream>
#include "MongoClient.h"

class MongoLogDef;

extern MongoLogDef mlDef;

static const auto mlMaxRecordCount = 100 * 10000;
static const std::string mlFieldLevel = "l";
static const std::string mlFieldCategory = "k";
static const std::string mlFieldValue = "v";
static const std::string mlFieldTimeStamp = "t";

enum MongoLogLevel
{
	MLLUndef = 0,
	MLLDebug = 1,
	MLLDetail = 2,
	MLLInfo = 3,
	MLLWarn = 4,
	MLLError = 5,
	MLLFatal = 6,
};

inline bool MLog(const std::string& coll, MongoLogLevel level, const std::string& key, const std::string& value)
{
	MongoClib::AutoPoolColl c(coll);
	if (c.Count(MongoClib::AutoBson()) > mlMaxRecordCount)
	{
		std::stringstream ss;
		ss << coll << "_" << time(nullptr);
		c.Rename(ss.str());
	}
	if (!c.CreateIndex(mlFieldCategory))
	{
		return false;
	}

	MongoClib::AutoBson b;
	b.AddTime(mlFieldTimeStamp);
	b.Add(mlFieldLevel, level);
	b.Add(mlFieldCategory, key);
	b.Add(mlFieldValue, value);

	if (b.IsFailed())
	{
		return false;
	}

	return c.Insert(b);
}

//////////////////////////////////////////////////////////////////////////
//
class MongoLogDef
{
private:
	class Teacher
	{
		std::string coll = "TeacherLog";
	public:
		operator std::string() { return coll; };
		// categories
		std::string enroll = "enroll";
		std::string out = "out";
		std::string tStart = "testStart";
		std::string tEnd = "testEnd";

		bool Enroll(const std::string& v, MongoLogLevel lv = MLLWarn)
		{
			return MLog(coll, lv, enroll, v);
		}

		bool Out(const std::string& v)
		{
			return MLog(coll, MLLWarn, out, v);
		}

		bool Start(const std::string& v)
		{
			return MLog(coll, MLLInfo, tStart, v);
		}

		bool End(const std::string& v)
		{
			return MLog(coll, MLLInfo, tEnd, v);
		}

		bool WillAutoGenKeyByFuncName(const std::string& v)
		{
			// should use with macro
			std::string fn = __FUNCTION__;
			auto s = fn.substr(fn.rfind(":") + 1, std::string::npos);
			return MLog(coll, MLLInfo, s, v);
		}
	};

	class Student
	{
		std::string coll = "LogStudent";
	public:
		operator std::string() { return coll; };
		// categories
		std::string income = "income";
		std::string out = "out";
		std::string take = "take";
	};
public:
	Teacher teacher;
	Student student;
};

#endif // LogAccess_h__

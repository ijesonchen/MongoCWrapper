#ifndef OidTimeAccess_h__
#define OidTimeAccess_h__

#include <string>
#include <deque>
#include <cstdint>

//#include "..\src\MongoClient.h"
#include "AccessBase.h"

class OidTime
{
public:
	std::string oid = "";
	std::int64_t idx = 0;
	time_t tmGen = 0;
	time_t tmOid = 0; // read from oid, do not write
	std::string str = "";
	std::string tmHex = "";

	inline std::string ToString(void)
	{
		std::stringstream ss;
		ss << oid << " " << idx << " " << std::hex << tmGen << " " << tmOid << " " << str << " " << tmHex;
		return ss.str();
	}
};

class OidTimeAccess : public MongoClib::AccessBase<OidTime>
{
	struct FieldName
	{
		std::string _id = "_id";
		std::string oid = "oid";
		std::string idx = "idx";
		std::string tmGen = "tmGen";
		std::string tmOid = "tmOid"; // do not write db, read only
		std::string str = "str";
		std::string tmHex = "tmHex";
	};
	const FieldName field;
public:
	OidTimeAccess();
	~OidTimeAccess();

	bool Delete(const std::int64_t idx);
	bool Update(const OidTime& obj);
	bool LoadOne(const std::int64_t idx, OidTime& obj);
	bool LoadAll(std::deque<OidTime>& dqObjects);
	bool LoadCond(std::deque<OidTime>& dqObjects, time_t tmGen);
	bool LoadCond(std::deque<OidTime>& dqObjects, bson_oid_t& o1, bson_oid_t& o2);
	bool LoadCond(std::deque<OidTime>& dqObjects, time_t tm1, time_t tm2);
	time_t  LoadTmGen(const std::int64_t idx);

	bool CreateIndexs(void);
private:
	MongoClib::AutoBson BuildBson(const OidTime& obj);
	bool ParseBson(OidTime& obj, const MongoClib::AutoCursor& cursor);

	bool QueryUpdate(const std::int64_t idx, const bson_t* update);
};

#endif // OidTimeAccess_h__

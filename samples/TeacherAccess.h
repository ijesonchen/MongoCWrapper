#ifndef TeacherAccess_h__
#define TeacherAccess_h__

#include "AccessBase.h"
#include "Teacher.h"
#include <deque>
#include <vector>
#include <list>

class TeacherAccess : public MongoClib::AccessBase<Teacher>
{
public:
	TeacherAccess();
	~TeacherAccess();

	using AccessBase::Insert;

	//	bool Insert(const Object& obj);
	bool Delete(const int idx);
	bool Update(const Teacher& obj);
	bool UpdateBin(const int idx, const std::string& bin);
	bool LoadOne(const int idx, Teacher& obj);
	bool LoadAll(std::deque<Teacher>& dqObjects);
	bool LoadCond(std::deque<Teacher>& dqObjects, const int group);
	std::string LoadBinData(const int idx);

	template<typename Container>
	bool Insert(const Container& cntr);

	template<typename Container>
	bool Insert2(const Container& cntr);

	bool Update(const std::deque<Teacher>& dqObjects);
	bool Replace(const std::deque<Teacher>& dqObjects);
	bool Delete(const std::deque<Teacher>& dqObjects);

	bool DeleteAll(void);
	// reset all name, test purpose only
	bool ResetName(void);

	bool CreateIndexs(void);
	bool CreateIndexsNoUnique(void);
	bool CreateComplexIndexs(void);
private:
	MongoClib::AutoBson BuildBson(const Teacher& obj);
	bool ParseBson(Teacher& obj, const MongoClib::AutoCursor& cursor);

	bool QueryUpdate(const int idx, const bson_t* update);
};

extern template
bool TeacherAccess::Insert(const std::deque<Teacher>& cntr);

extern template
bool TeacherAccess::Insert(const std::vector<Teacher>& cntr);

extern template
bool TeacherAccess::Insert(const std::list<Teacher>& cntr);

#endif // TeacherAccess_h__

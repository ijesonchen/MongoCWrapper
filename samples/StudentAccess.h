#ifndef StudentAccess_h__
#define StudentAccess_h__

// this should test with mfc

#include "Student.h"
#include <string>
#include <deque>

#include "..\src\MongoClient.h"
#include "AccessBase.h"

class StudentAccess : public MongoClib::AccessBase<Student>
{
public:
	StudentAccess();
	~StudentAccess();

	//	bool Insert(const Object& obj);
	bool Delete(const CString& name);
	bool Update(const Student& obj);
	bool UpdateBool(const CString& name, bool b);
	bool LoadOne(const CString& name, Student& obj);
	bool LoadAll(std::deque<Student>& dqObjects);
	bool LoadCond(std::deque<Student>& dqObjects, const int n);
	int  LoadState(const CString& name);

	bool CreateIndexs(void);
private:
	MongoClib::AutoBson BuildBson(const Student& obj);
	bool ParseBson(Student& obj, const MongoClib::AutoCursor& cursor);

	bool QueryUpdate(const CString& name, const bson_t* update);
};

#endif // StudentAccess_h__

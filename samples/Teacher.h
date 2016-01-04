#ifndef Teacher_h__
#define Teacher_h__

#include <string>
#include <vector>
#include "Account.h"

class Teacher
{
public:
	Teacher();
	~Teacher();
	int idx = 0;
	int group = 0;
	std::string name;
	std::wstring wname;
	std::vector<std::string> students;
	std::vector<Account> accounts;
	std::string binData;

	bool operator==(const Teacher& rhs) const
	{
		if (this == &rhs)
		{
			return true;
		}
		if (idx == rhs.idx &&
			name == rhs.name &&
			wname == rhs.wname &&
			students == rhs.students &&
			accounts == rhs.accounts &&
			binData == rhs.binData)
		{
			return true;
		}
		return false;
	}

	bool operator!=(const Teacher& rhs) const
	{
		return !operator==(rhs);
	}
};

#endif // Teacher_h__

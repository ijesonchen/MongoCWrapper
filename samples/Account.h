#ifndef Account_h__
#define Account_h__

#include <string>

class Account
{
public:
	long long  guid = 0;
	std::string name;
	std::string memo;
	bool inuse = true;

	bool operator==(const Account& rhs) const
	{
		if (guid == rhs.guid &&
			name == rhs.name &&
			memo == rhs.memo)
		{
			return true;
		}
		return false;
	}
};

#endif // Account_h__
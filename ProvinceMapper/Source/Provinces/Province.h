#ifndef PROVINCE_H
#define PROVINCE_H
#include <string>

struct Province
{
	Province() = default;
	explicit Province(int theID);
	bool operator==(const Province& rhs) const;
	bool operator<(const Province& rhs) const;
	bool operator!=(const Province& rhs) const;
	bool operator!() const;

	int ID = 0;
	mutable int r = 0;
	mutable int g = 0;
	mutable int b = 0;
	mutable std::string name;
};

#endif // PROVINCE_H

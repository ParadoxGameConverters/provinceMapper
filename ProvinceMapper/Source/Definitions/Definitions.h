#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <map>
#include <memory>
#include <string>

struct Province;
class Definitions
{
public:
	void loadDefinitions(const std::string& fileName);
	void loadDefinitions(std::istream& theStream);

	[[nodiscard]] const auto& getProvinces() const { return provinces; }

private:
	void parseStream(std::istream& theStream);

	std::map<int, std::shared_ptr<Province>> provinces;
};

#endif // DEFINITIONS_H

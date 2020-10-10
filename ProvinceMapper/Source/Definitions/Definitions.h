#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <map>
#include <string>
#include <optional>

struct ProvinceDefinition
{
	int provinceID = 0;
	int r = 0;
	int g = 0;
	int b = 0;
	std::string mapDataName;
};

class Definitions
{
public:
	void loadDefinitions(const std::string& fileName);
	void loadDefinitions(std::istream& theStream);

	[[nodiscard]] std::optional<std::tuple<int, int, int>> getColorForProvinceID(int provinceID) const;
	[[nodiscard]] const auto& getDefinitions() const { return definitions; }

private:
	void parseStream(std::istream& theStream);

	std::map<int, ProvinceDefinition> definitions;
};

#endif // DEFINITIONS_H

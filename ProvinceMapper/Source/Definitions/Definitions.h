#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#include <map>
#include <memory>
#include <optional>
#include <string>

struct Province;
class Definitions
{
  public:
	void loadDefinitions(const std::string& fileName);
	void loadDefinitions(std::istream& theStream);

	void registerPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
	void registerBorderPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b);
	[[nodiscard]] const auto& getProvinces() const { return provinces; }

  private:
	void parseStream(std::istream& theStream);
	[[nodiscard]] std::optional<std::tuple<int, unsigned char, unsigned char, unsigned char, std::string>> parseLine(const std::string& line);

	std::map<int, std::shared_ptr<Province>> provinces;				// ID, province
	std::map<unsigned int, std::shared_ptr<Province>> chromaCache; // color, province
};

#endif // DEFINITIONS_H

#include "PointMapper.h"
#include "ParserHelpers.h"
#include <fstream>

PointMapper::PointMapper(std::istream& theStream)
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

PointMapper::PointMapper()
{
	coPoints = std::make_shared<std::vector<std::shared_ptr<CoPoint>>>();
	registerKeys();
	parseFile("test_mappings.txt");
	clearRegisteredKeywords();
}

void PointMapper::registerKeys()
{
	registerKeyword("link", [this](const std::string& unused, std::istream& theStream) {
		const auto coPoint = std::make_shared<CoPoint>(theStream);
		coPoints->push_back(coPoint);
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

void PointMapper::exportPoints() const
{
	std::ofstream pointFile("test_mappings.txt");
	for (const auto& coPoint: *coPoints)
		pointFile << *coPoint;
	pointFile.close();
}

void PointMapper::exportAdjustedMap() const
{
}

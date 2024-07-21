#include "CommonRegexes.h"
#include "TriangulationPointPair.h"
#include "Log.h"
#include "ParserHelpers.h"
#include "Provinces/Province.h"

TriangulationPointPair::TriangulationPointPair(std::istream& theStream,
	 const int theID):
	 ID(theID)
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

TriangulationPointPair::TriangulationPointPair(int theID): ID(theID)
{
}

void TriangulationPointPair::registerKeys()
{
	registerKeyword("srcX", [this](std::istream& theStream) {
		sourcePoint.x = commonItems::getDouble(theStream);
	});
	registerKeyword("srcY", [this](std::istream& theStream) {
		sourcePoint.y = commonItems::getDouble(theStream);
	});
	registerKeyword("dstX", [this](std::istream& theStream) {
		targetPoint.x = commonItems::getDouble(theStream);
	});
	registerKeyword("dstY", [this](std::istream& theStream) {
		targetPoint.y = commonItems::getDouble(theStream);
	});
	registerKeyword("comment", [this](std::istream& theStream) {
		comment = commonItems::getString(theStream);
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

std::ostream& operator<<(std::ostream& output, const TriangulationPointPair& pointPair)
{
	output << "\ttriangulation_pair = { ";

	// Dump point coordinates
	output << "srcX = " << pointPair.sourcePoint.x << " ";
	output << "srcY = " << pointPair.sourcePoint.y << " ";
	output << "dstX = " << pointPair.targetPoint.x << " ";
	output << "dstY = " << pointPair.targetPoint.y << " ";

	if (pointPair.comment)
	{
		output << "comment = \"" << *pointPair.comment << "\" ";
		return output;
	}

	output << "}\n";
	return output;
}

bool TriangulationPointPair::operator==(const TriangulationPointPair& rhs) const
{
	return ID == rhs.ID;
}

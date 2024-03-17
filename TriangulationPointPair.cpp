#include "CommonRegexes.h"
#include "TriangulationPointPair.h"
#include "Log.h"
#include "ParserHelpers.h"
#include "Provinces/Province.h"

TriangulationPointPair::TriangulationPointPair(std::istream& theStream,
	 std::string theSourceToken,
	 std::string theTargetToken,
	 const int theID):
	 ID(theID),
	 sourceToken(std::move(theSourceToken)), targetToken(std::move(theTargetToken))
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

TriangulationPointPair::TriangulationPointPair(std::string theSourceToken,
	 std::string theTargetToken,
	 int theID):
	 ID(theID),
	 sourceToken(std::move(theSourceToken)), targetToken(std::move(theTargetToken))
{
}

void TriangulationPointPair::registerKeys()
{
	registerKeyword(sourceToken, [this](std::istream& theStream) {
		auto id = commonItems::getString(theStream);
		if (id.substr(0, 2) == "0x")
		{
			id = id.substr(1, id.length());
		}
		const auto& provinces = sourceDefs->getProvinces();
		if (const auto& provItr = provinces.find(id); provItr != provinces.end())
			sources.emplace_back(provItr->second);
	});
	registerKeyword(targetToken, [this](std::istream& theStream) {
		auto id = commonItems::getString(theStream);
		if (id.substr(0, 2) == "0x")
		{
			id = id.substr(1, id.length());
		}
		const auto& provinces = targetDefs->getProvinces();
		if (const auto& provItr = provinces.find(id); provItr != provinces.end())
			targets.emplace_back(provItr->second);
	});
	registerKeyword("comment", [this](std::istream& theStream) {
		comment = commonItems::getString(theStream);
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

std::ostream& operator<<(std::ostream& output, const TriangulationPointPair& pointPair)
{
	output << "\tlink = { ";
	// If this is a comment only output the comment.
	if (pointPair.comment)
	{
		output << "comment = \"" << *pointPair.comment << "\" }\n";
		return output;
	}

	// Dump point coordinates
	output << "srcX = " << pointPair.sourcePoint.x << " ";
	output << "srcY = " << pointPair.sourcePoint.y << " ";
	output << "dstX = " << pointPair.targetPoint.x << " ";
	output << "dstY = " << pointPair.targetPoint.y << " ";
	output << "}";

	output << "\n";
	return output;
}

bool TriangulationPointPair::operator==(const TriangulationPointPair& rhs) const
{
	return ID == rhs.ID;
}

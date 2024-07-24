#include "Parser.h"
#include "CommonRegexes.h"
#include "TriangulationPointPair.h"
#include "Log.h"
#include "ParserHelpers.h"
#include "Provinces/Province.h"

TriangulationPointPair::TriangulationPointPair(std::istream& theStream,
	 const int theID): LinkBase(theID)
{
	auto parser = commonItems::parser();
	registerKeys(parser);
	parser.parseStream(theStream);
	parser.clearRegisteredKeywords();
}

TriangulationPointPair::TriangulationPointPair(int theID): LinkBase(theID)
{
}

const std::string TriangulationPointPair::toRowString()
{
	std::string name;

	const auto& sourcePoint = getSourcePoint();
	if (sourcePoint)
	{
		wxString sourcePointStr = wxString::Format("(%d, %d)", sourcePoint->x, sourcePoint->y);
		name += std::string(sourcePointStr.mb_str());
	}

	name += " -> ";

	const auto& targetPoint = getTargetPoint();
	if (targetPoint)
	{
		wxString targetPointStr = wxString::Format("(%d, %d)", targetPoint->x, targetPoint->y);
		name += std::string(targetPointStr.mb_str());
	}

	if (comment)
		name += " " + *comment;

	return name;
}

const wxColour TriangulationPointPair::getBaseRowColour()
{
	return wxColour(240, 240, 240);
}

const wxColour TriangulationPointPair::getActiveRowColour()
{
	return wxColour(150, 250, 150); // Bright green for selected pairs.
}

void TriangulationPointPair::registerKeys(commonItems::parser parser)
{
	parser.registerKeyword("srcX", [this](std::istream& theStream) {
		if (!sourcePoint)
			sourcePoint = wxPoint();
		(*sourcePoint).x = commonItems::getDouble(theStream);
	});
	parser.registerKeyword("srcY", [this](std::istream& theStream) {
		if (!sourcePoint)
			sourcePoint = wxPoint();
		(*sourcePoint).y = commonItems::getDouble(theStream);
	});
	parser.registerKeyword("dstX", [this](std::istream& theStream) {
		if (!targetPoint)
			targetPoint = wxPoint();
		(*targetPoint).x = commonItems::getDouble(theStream);
	});
	parser.registerKeyword("dstY", [this](std::istream& theStream) {
		if (!targetPoint)
			targetPoint = wxPoint();
		(*targetPoint).y = commonItems::getDouble(theStream);
	});
	parser.registerKeyword("comment", [this](std::istream& theStream) {
		comment = commonItems::getString(theStream);
	});
	parser.registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

std::ostream& operator<<(std::ostream& output, const TriangulationPointPair& pointPair)
{
	output << "\ttriangulation_pair = { ";

	// Dump point coordinates
	if (pointPair.sourcePoint)
	{
		output << "srcX = " << pointPair.sourcePoint->x << " ";
		output << "srcY = " << pointPair.sourcePoint->y << " ";
	}
	if (pointPair.targetPoint)
	{
		output << "dstX = " << pointPair.targetPoint->x << " ";
		output << "dstY = " << pointPair.targetPoint->y << " ";
	}

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

#include "CoPoint.h"
#include "Log.h"
#include "ParserHelpers.h"

CoPoint::CoPoint(std::istream& theStream)
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

void CoPoint::registerKeys()
{
	registerKeyword("name", [this](const std::string& unused, std::istream& theStream) {
		name = commonItems::singleString(theStream).getString();
	});
	registerKeyword("source", [this](const std::string& unused, std::istream& theStream) {
		const auto coords = commonItems::intList(theStream).getInts();
		if (coords.size() == 2)
		{
			source = Point(coords[0], coords[1]);
		}
	});
	registerKeyword("target", [this](const std::string& unused, std::istream& theStream) {
		const auto coords = commonItems::intList(theStream).getInts();
		if (coords.size() == 2)
		{
			target = Point(coords[0], coords[1]);
		}
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

bool CoPoint::operator==(const CoPoint& lhs) const
{
	if (lhs.source && !source)
		return false;
	if (!lhs.source && source)
		return false;
	if (lhs.target && !target)
		return false;
	if (!lhs.target && target)
		return false;
	if (lhs.name.empty() && !name.empty())
		return false;
	if (!lhs.name.empty() && name.empty())
		return false;
	if (source && *source != *lhs.source)
		return false;
	if (target && *target != *lhs.target)
		return false;
	if (name != lhs.name)
		return false;
	return true;
}

std::ostream& operator<<(std::ostream& output, const CoPoint& coPoint)
{
	output << "link = { ";
	if (!coPoint.name.empty())
		output << "name = \"" << coPoint.name << "\" ";
	if (coPoint.getSource())
		output << "source = { " << coPoint.source->x << " " << coPoint.source->y << "} ";
	if (coPoint.getTarget())
		output << "target = { " << coPoint.target->x << " " << coPoint.target->y << "} ";
	output << "}\n";
	return output;
}

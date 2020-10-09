#include "LinkMapping.h"
#include "Log.h"
#include "ParserHelpers.h"

LinkMapping::LinkMapping(std::istream& theStream)
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

void LinkMapping::registerKeys()
{
	registerKeyword("name", [this](const std::string& unused, std::istream& theStream) {
		name = commonItems::singleString(theStream).getString();
	});
	registerKeyword("source", [this](const std::string& unused, std::istream& theStream) {
		sources.insert(Province(commonItems::singleInt(theStream).getInt()));
	});
	registerKeyword("target", [this](const std::string& unused, std::istream& theStream) {
		targets.insert(Province(commonItems::singleInt(theStream).getInt()));
	});
	registerKeyword("comment", [this](const std::string& unused, std::istream& theStream) {
		comment = commonItems::singleString(theStream).getString();
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

std::ostream& operator<<(std::ostream& output, const LinkMapping& linkMapping)
{
	output << "link = { ";
	if (!linkMapping.comment.empty())
	{
		output << "comment = \"" << linkMapping.comment << "\" }\n";
		return output;
	}
	if (!linkMapping.name.empty())
		output << "name = \"" << linkMapping.name << "\" ";
	for (const auto& province: linkMapping.sources)
	{
		output << "ck3 = " << province.ID << " ";
	}
	for (const auto& province: linkMapping.targets)
	{
		output << "eu4 = " << province.ID << " ";
	}
	output << "} # ";
	bool first = true;
	for (const auto& province: linkMapping.sources)
	{
		if (first)
		{
			output << province.name;
			first = false;
		}
		else
		{
			output << ", " << province.name;
		}
	}
	output << " -> ";
	first = true;
	for (const auto& province: linkMapping.targets)
	{
		if (first)
		{
			output << province.name;
			first = false;
		}
		else
		{
			output << ", " << province.name;
		}
	}
	output << "\n";
	return output;
}

void LinkMapping::toggleSource(const Province& theSource)
{
	if (sources.count(theSource))
		sources.erase(theSource);
	else
		sources.insert(theSource);
}

void LinkMapping::toggleTarget(const Province& theTarget)
{
	if (sources.count(theTarget))
		sources.erase(theTarget);
	else
		sources.insert(theTarget);
}

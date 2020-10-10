#include "LinkMapping.h"
#include "ParserHelpers.h"
#include "Log.h"
#include "../Provinces/Province.h"
#include "../Definitions/Definitions.h"

LinkMapping::LinkMapping(std::istream& theStream, const Definitions& sourceDefs, const Definitions& targetDefs)
{
	registerKeys(sourceDefs, targetDefs);
	parseStream(theStream);
	clearRegisteredKeywords();
}

void LinkMapping::registerKeys(const Definitions& sourceDefs, const Definitions& targetDefs)
{
	registerKeyword("name", [this](const std::string& unused, std::istream& theStream) {
		name = commonItems::singleString(theStream).getString();
	});
	registerKeyword("source", [this, sourceDefs](const std::string& unused, std::istream& theStream) {
		const auto id = commonItems::singleInt(theStream).getInt();
		if (sourceDefs.getProvinces().count(id))
			sources.insert(sourceDefs.getProvinces().find(id)->second);
	});
	registerKeyword("target", [this, targetDefs](const std::string& unused, std::istream& theStream) {
		const auto id = commonItems::singleInt(theStream).getInt();
		if (targetDefs.getProvinces().count(id))
			sources.insert(targetDefs.getProvinces().find(id)->second);
	});
	registerKeyword("comment", [this](const std::string& unused, std::istream& theStream) {
		comment = commonItems::singleString(theStream).getString();
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

std::ostream& operator<<(std::ostream& output, const LinkMapping& linkMapping)
{
	output << "\tlink = { ";
	if (!linkMapping.comment.empty())
	{
		output << "comment = \"" << linkMapping.comment << "\" }\n";
		return output;
	}
	if (!linkMapping.name.empty())
		output << "name = \"" << linkMapping.name << "\" ";
	for (const auto& province: linkMapping.sources)
	{
		output << "ck3 = " << province << " ";
	}
	for (const auto& province: linkMapping.targets)
	{
		output << "eu4 = " << province << " ";
	}
	output << "} # ";
	if (linkMapping.sources.size() > 1 && linkMapping.targets.size() > 1)
		output << "MANY-TO-MANY: ";
	std::string comma;
	if (!linkMapping.sources.empty())
		for (const auto& province: linkMapping.sources)
		{
			output << comma;
			output << province;
			comma = ", ";
		}
	else
		output << "NOTHING";
	output << " -> ";
	comma.clear();
	if (!linkMapping.targets.empty())
		for (const auto& province: linkMapping.targets)
		{
			output << comma;
			output << province;
			comma = ", ";
		}
	else
		output << "DROPPED";
	output << "\n";
	return output;
}

void LinkMapping::toggleSource(const std::shared_ptr<Province>& theSource)
{
	if (sources.count(theSource))
		sources.erase(theSource);
	else
		sources.insert(theSource);
}

void LinkMapping::toggleTarget(const std::shared_ptr<Province>& theTarget)
{
	if (sources.count(theTarget))
		sources.erase(theTarget);
	else
		sources.insert(theTarget);
}

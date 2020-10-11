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
	registerKeyword("ck3", [this, sourceDefs](const std::string& unused, std::istream& theStream) {
		const auto id = commonItems::singleInt(theStream).getInt();
		const auto& provinces = sourceDefs.getProvinces();
		const auto& provItr = provinces.find(id);
		if (provItr != provinces.end())
			sources.emplace_back(provItr->second);
	});
	registerKeyword("eu4", [this, targetDefs](const std::string& unused, std::istream& theStream) {
		const auto id = commonItems::singleInt(theStream).getInt();
		const auto& provinces = targetDefs.getProvinces();
		const auto& provItr = provinces.find(id);
		if (provItr != provinces.end())
			targets.emplace_back(provItr->second);
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
	std::vector<std::shared_ptr<Province>> replacement;
	for (const auto& province: sources)
		if (*province != *theSource)
			replacement.emplace_back(province);
	if (replacement.size() == sources.size())
		sources.emplace_back(theSource);
	else
		sources = replacement;
}

void LinkMapping::toggleTarget(const std::shared_ptr<Province>& theTarget)
{
	std::vector<std::shared_ptr<Province>> replacement;
	for (const auto& province: targets)
		if (*province != *theTarget)
			replacement.emplace_back(province);
	if (replacement.size() == targets.size())
		targets.emplace_back(theTarget);
	else
		targets = replacement;
}

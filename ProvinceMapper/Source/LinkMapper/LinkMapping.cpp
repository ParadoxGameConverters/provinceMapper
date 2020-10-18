#include "LinkMapping.h"
#include "Definitions/Definitions.h"
#include "Log.h"
#include "ParserHelpers.h"
#include "Provinces/Province.h"

LinkMapping::LinkMapping(std::istream& theStream,
	 std::shared_ptr<Definitions> theSourceDefs,
	 std::shared_ptr<Definitions> theTargetDefs,
	 std::string theSourceToken,
	 std::string theTargetToken,
	 const int theID):
	 ID(theID),
	 sourceDefs(std::move(theSourceDefs)), targetDefs(std::move(theTargetDefs)), sourceToken(std::move(theSourceToken)), targetToken(std::move(theTargetToken))
{
	registerKeys();
	parseStream(theStream);
	clearRegisteredKeywords();
}

LinkMapping::LinkMapping(std::shared_ptr<Definitions> theSourceDefs,
	 std::shared_ptr<Definitions> theTargetDefs,
	 std::string theSourceToken,
	 std::string theTargetToken,
	 int theID):
	 ID(theID),
	 sourceDefs(std::move(theSourceDefs)), targetDefs(std::move(theTargetDefs)), sourceToken(std::move(theSourceToken)), targetToken(std::move(theTargetToken))
{
}

void LinkMapping::registerKeys()
{
	registerKeyword(sourceToken, [this](const std::string& unused, std::istream& theStream) {
		const auto id = commonItems::singleInt(theStream).getInt();
		const auto& provinces = sourceDefs->getProvinces();
		if (const auto& provItr = provinces.find(id); provItr != provinces.end())
			sources.emplace_back(provItr->second);
	});
	registerKeyword(targetToken, [this](const std::string& unused, std::istream& theStream) {
		const auto id = commonItems::singleInt(theStream).getInt();
		const auto& provinces = targetDefs->getProvinces();
		if (const auto& provItr = provinces.find(id); provItr != provinces.end())
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
	// If this is a comment only output the comment.
	if (linkMapping.comment)
	{
		output << "comment = \"" << *linkMapping.comment << "\" }\n";
		return output;
	}

	// Dump numbers
	for (const auto& province: linkMapping.sources)
	{
		output << linkMapping.sourceToken << " = " << province->ID << " ";
	}
	for (const auto& province: linkMapping.targets)
	{
		output << linkMapping.targetToken << " = " << province->ID << " ";
	}
	output << "} # ";

	// Comment section, N-to-N
	if (linkMapping.sources.size() > 1 && linkMapping.targets.size() > 1)
		output << "MANY-TO-MANY: ";

	// List sources
	std::string comma;
	if (!linkMapping.sources.empty())
		for (const auto& province: linkMapping.sources)
		{
			output << comma;
			output << province->mapDataName;
			comma = ", ";
		}
	else
		output << "NOTHING";

	// List targets
	output << " -> ";
	comma.clear();
	if (!linkMapping.targets.empty())
		for (const auto& province: linkMapping.targets)
		{
			output << comma;
			output << province->mapDataName;
			comma = ", ";
		}
	else
		output << "DROPPED";

	output << "\n";
	return output;
}

void LinkMapping::toggleSource(const int sourceID)
{
	std::vector<std::shared_ptr<Province>> replacement;
	for (const auto& province: sources)
		if (province->ID != sourceID)
			replacement.emplace_back(province);
	if (replacement.size() == sources.size())
	{
		// We need to find the province in sourceDefs and import it.
		auto province = sourceDefs->getProvinceForID(sourceID);
		if (province)
			sources.emplace_back(province);
	}
	else
		sources = replacement;
}

void LinkMapping::toggleTarget(const int targetID)
{
	std::vector<std::shared_ptr<Province>> replacement;
	for (const auto& province: targets)
		if (province->ID != targetID)
			replacement.emplace_back(province);
	if (replacement.size() == targets.size())
	{
		// We need to find the province in targetDefs and import it.
		auto province = targetDefs->getProvinceForID(targetID);
		if (province)
			targets.emplace_back(province);
	}
	else
		targets = replacement;
}

bool LinkMapping::operator==(const LinkMapping& rhs) const
{
	return ID == rhs.ID;
}

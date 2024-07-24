#include "LinkMapping.h"
#include "CommonRegexes.h"
#include "Log.h"
#include "ParserHelpers.h"
#include "Provinces/Province.h"

LinkMapping::LinkMapping(std::istream& theStream,
	 std::shared_ptr<DefinitionsInterface> theSourceDefs,
	 std::shared_ptr<DefinitionsInterface> theTargetDefs,
	 std::string theSourceToken,
	 std::string theTargetToken,
	 const int theID):
	 LinkBase(theID),
	 sourceDefs(std::move(theSourceDefs)), targetDefs(std::move(theTargetDefs)), sourceToken(std::move(theSourceToken)), targetToken(std::move(theTargetToken))
{
	auto parser = commonItems::parser();
	registerKeys(parser);
	parser.parseStream(theStream);
	parser.clearRegisteredKeywords();
}

LinkMapping::LinkMapping(std::shared_ptr<DefinitionsInterface> theSourceDefs,
	 std::shared_ptr<DefinitionsInterface> theTargetDefs,
	 std::string theSourceToken,
	 std::string theTargetToken,
	 int theID):
	 LinkBase(theID),
	 sourceDefs(std::move(theSourceDefs)), targetDefs(std::move(theTargetDefs)), sourceToken(std::move(theSourceToken)), targetToken(std::move(theTargetToken))
{
}

void LinkMapping::registerKeys(commonItems::parser parser)
{
	parser.registerKeyword(sourceToken, [this](std::istream& theStream) {
		auto id = commonItems::getString(theStream);
		if (id.substr(0, 2) == "0x")
		{
			id = id.substr(1, id.length());
		}
		const auto& provinces = sourceDefs->getProvinces();
		if (const auto& provItr = provinces.find(id); provItr != provinces.end())
			sources.emplace_back(provItr->second);
	});
	parser.registerKeyword(targetToken, [this](std::istream& theStream) {
		auto id = commonItems::getString(theStream);
		if (id.substr(0, 2) == "0x")
		{
			id = id.substr(1, id.length());
		}
		const auto& provinces = targetDefs->getProvinces();
		if (const auto& provItr = provinces.find(id); provItr != provinces.end())
			targets.emplace_back(provItr->second);
	});
	parser.registerKeyword("comment", [this](std::istream& theStream) {
		comment = commonItems::getString(theStream);
	});
	parser.registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
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
			if (province->locName)
				output << *province->locName;
			else if (!province->mapDataName.empty())
				output << province->mapDataName;
			else
				output << "(Unknown)";
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
			if (province->locName)
				output << *province->locName;
			else if (!province->mapDataName.empty())
				output << province->mapDataName;
			else
				output << "(Unknown)";
			comma = ", ";
		}
	else
		output << "DROPPED";

	output << "\n";
	return output;
}

Mapping LinkMapping::toggleSource(const std::string& sourceID)
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
		{
			sources.emplace_back(province);
			return Mapping::MAPPED;
		}
		else
		{
			return Mapping::FAIL;
		}
	}
	else
	{
		sources = replacement;
		return Mapping::UNMAPPED;
	}
}

Mapping LinkMapping::toggleTarget(const std::string& targetID)
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
		{
			targets.emplace_back(province);
			return Mapping::MAPPED;
		}
		else
		{
			return Mapping::FAIL;
		}
	}
	else
	{
		targets = replacement;
		return Mapping::UNMAPPED;
	}
}

bool LinkMapping::operator==(const LinkMapping& rhs) const
{
	return ID == rhs.ID;
}

#include "Configuration.h"
#include "OSCompatibilityLayer.h"
#include "ParserHelpers.h"
#include <fstream>

void Configuration::registerKeys()
{
	registerKeyword("sourceDir", [this](const std::string& unused, std::istream& theStream) {
		sourceDir = commonItems::singleString(theStream).getString();
	});
	registerKeyword("targetDir", [this](const std::string& unused, std::istream& theStream) {
		targetDir = commonItems::singleString(theStream).getString();
	});
	registerKeyword("sourceToken", [this](const std::string& unused, std::istream& theStream) {
		sourceToken = commonItems::singleString(theStream).getString();
	});
	registerKeyword("targetToken", [this](const std::string& unused, std::istream& theStream) {
		targetToken = commonItems::singleString(theStream).getString();
	});
	registerKeyword("linkFile", [this](const std::string& unused, std::istream& theStream) {
		linkFile = commonItems::singleString(theStream).getString();
	});
	registerKeyword("reverseSource", [this](const std::string& unused, std::istream& theStream) {
		reverseSource = commonItems::singleString(theStream).getString() == "true";
	});
	registerKeyword("reverseTarget", [this](const std::string& unused, std::istream& theStream) {
		reverseTarget = commonItems::singleString(theStream).getString() == "true";
	});
	registerRegex(commonItems::catchallRegex, commonItems::ignoreItem);
}

std::ostream& operator<<(std::ostream& output, const Configuration& configuration)
{
	if (configuration.sourceDir)
		output << "sourceDir = \"" << *configuration.sourceDir << "\"\n";
	if (configuration.reverseSource)
		output << "reverseSource = true\n";
	if (configuration.targetDir)
		output << "targetDir = \"" << *configuration.targetDir << "\"\n";
	if (configuration.reverseTarget)
		output << "reverseTarget = true\n";
	if (configuration.sourceToken)
		output << "sourceToken = \"" << *configuration.sourceToken << "\"\n";
	if (configuration.targetToken)
		output << "targetToken = \"" << *configuration.targetToken << "\"\n";
	if (configuration.linkFile)
		output << "linkFile = \"" << *configuration.linkFile << "\"\n";

	return output;
}

void Configuration::save() const
{
	std::ofstream confFile("configuration.txt");
	confFile << *this;
	confFile.close();
}

void Configuration::load()
{
	registerKeys();
	if (commonItems::DoesFileExist("configuration.txt"))
		parseFile("configuration.txt");
	clearRegisteredKeywords();
}

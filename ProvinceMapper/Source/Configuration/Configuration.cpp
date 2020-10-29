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
	registerKeyword("imageFramePos", [this](const std::string& unused, std::istream& theStream) {
		const auto& theInts = commonItems::intList(theStream).getInts();
		if (theInts.size() == 2)
			imageFramePos = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("imageFrameSize", [this](const std::string& unused, std::istream& theStream) {
		const auto& theInts = commonItems::intList(theStream).getInts();
		if (theInts.size() == 2)
			imageFrameSize = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("imageFrameMaximized", [this](const std::string& unused, std::istream& theStream) {
		imageFrameMaximized = commonItems::singleString(theStream).getString() == "true";
	});
	registerKeyword("linksFramePos", [this](const std::string& unused, std::istream& theStream) {
		const auto& theInts = commonItems::intList(theStream).getInts();
		if (theInts.size() == 2)
			linksFramePos = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("linksFrameSize", [this](const std::string& unused, std::istream& theStream) {
		const auto& theInts = commonItems::intList(theStream).getInts();
		if (theInts.size() == 2)
			linksFrameSize = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("linksFrameMaximized", [this](const std::string& unused, std::istream& theStream) {
		linksFrameMaximized = commonItems::singleString(theStream).getString() == "true";
	});
	registerKeyword("unmappedFramePos", [this](const std::string& unused, std::istream& theStream) {
		const auto& theInts = commonItems::intList(theStream).getInts();
		if (theInts.size() == 2)
			unmappedFramePos = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("unmappedFrameSize", [this](const std::string& unused, std::istream& theStream) {
		const auto& theInts = commonItems::intList(theStream).getInts();
		if (theInts.size() == 2)
			unmappedFrameSize = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("unmappedFrameMaximized", [this](const std::string& unused, std::istream& theStream) {
		unmappedFrameMaximized = commonItems::singleString(theStream).getString() == "true";
	});
	registerKeyword("unmappedFrameOn", [this](const std::string& unused, std::istream& theStream) {
		unmappedFrameOn = commonItems::singleString(theStream).getString() == "true";
	});
	registerKeyword("statusBarPos", [this](const std::string& unused, std::istream& theStream) {
		const auto& theInts = commonItems::intList(theStream).getInts();
		if (theInts.size() == 2)
			statusBarPos = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("statusBarOn", [this](const std::string& unused, std::istream& theStream) {
		statusBarOn = commonItems::singleString(theStream).getString() == "true";
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
	if (configuration.imageFramePos)
		output << "imageFramePos = { " << configuration.imageFramePos->x << " " << configuration.imageFramePos->y << " }\n";
	if (configuration.imageFrameSize)
		output << "imageFrameSize = { " << configuration.imageFrameSize->x << " " << configuration.imageFrameSize->y << " }\n";
	if (configuration.imageFrameMaximized)
		output << "imageFrameMaximized = true\n";
	else
		output << "imageFrameMaximized = false\n";
	if (configuration.linksFramePos)
		output << "linksFramePos = { " << configuration.linksFramePos->x << " " << configuration.linksFramePos->y << " }\n";
	if (configuration.linksFrameSize)
		output << "linksFrameSize = { " << configuration.linksFrameSize->x << " " << configuration.linksFrameSize->y << " }\n";
	if (configuration.linksFrameMaximized)
		output << "linksFrameMaximized = true\n";
	else
		output << "linksFrameMaximized = false\n";
	if (configuration.linksFramePos)
		output << "unmappedFramePos = { " << configuration.unmappedFramePos->x << " " << configuration.unmappedFramePos->y << " }\n";
	if (configuration.unmappedFrameSize)
		output << "unmappedFrameSize = { " << configuration.unmappedFrameSize->x << " " << configuration.unmappedFrameSize->y << " }\n";
	if (configuration.unmappedFrameMaximized)
		output << "unmappedFrameMaximized = true\n";
	else
		output << "unmappedFrameMaximized = false\n";
	if (configuration.unmappedFrameOn)
		output << "unmappedFrameOn = true\n";
	else
		output << "unmappedFrameOn = false\n";
	if (configuration.statusBarPos)
		output << "statusBarPos = { " << configuration.statusBarPos->x << " " << configuration.statusBarPos->y << " }\n";
	if (configuration.statusBarOn)
		output << "statusBarOn = true\n";
	else
		output << "statusBarOn = false\n";

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

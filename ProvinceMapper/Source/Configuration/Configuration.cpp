#include "Configuration.h"
#include "OSCompatibilityLayer.h"
#include "ParserHelpers.h"
#include "CommonRegexes.h"
#include <fstream>

void Configuration::registerKeys()
{
	registerKeyword("sourceDir", [this](std::istream& theStream) {
		sourceDir = commonItems::getString(theStream);
	});
	registerKeyword("targetDir", [this](std::istream& theStream) {
		targetDir = commonItems::getString(theStream);
	});
	registerKeyword("sourceToken", [this](std::istream& theStream) {
		sourceToken = commonItems::getString(theStream);
	});
	registerKeyword("targetToken", [this](std::istream& theStream) {
		targetToken = commonItems::getString(theStream);
	});
	registerKeyword("linkFile", [this](std::istream& theStream) {
		linkFile = commonItems::getString(theStream);
	});
	registerKeyword("reverseSource", [this](std::istream& theStream) {
		reverseSource = commonItems::getString(theStream) == "true";
	});
	registerKeyword("reverseTarget", [this](std::istream& theStream) {
		reverseTarget = commonItems::getString(theStream) == "true";
	});
	registerKeyword("imageFramePos", [this](std::istream& theStream) {
		const auto& theInts = commonItems::getInts(theStream);
		if (theInts.size() == 2)
			imageFramePos = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("imageFrameSize", [this](std::istream& theStream) {
		const auto& theInts = commonItems::getInts(theStream);
		if (theInts.size() == 2)
			imageFrameSize = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("imageFrameMaximized", [this](std::istream& theStream) {
		imageFrameMaximized = commonItems::getString(theStream) == "true";
	});
	registerKeyword("linksFramePos", [this](std::istream& theStream) {
		const auto& theInts = commonItems::getInts(theStream);
		if (theInts.size() == 2)
			linksFramePos = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("linksFrameSize", [this](std::istream& theStream) {
		const auto& theInts = commonItems::getInts(theStream);
		if (theInts.size() == 2)
			linksFrameSize = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("linksFrameMaximized", [this](std::istream& theStream) {
		linksFrameMaximized = commonItems::getString(theStream) == "true";
	});
	registerKeyword("unmappedFramePos", [this](std::istream& theStream) {
		const auto& theInts = commonItems::getInts(theStream);
		if (theInts.size() == 2)
			unmappedFramePos = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("unmappedFrameSize", [this](std::istream& theStream) {
		const auto& theInts = commonItems::getInts(theStream);
		if (theInts.size() == 2)
			unmappedFrameSize = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("unmappedFrameMaximized", [this](std::istream& theStream) {
		unmappedFrameMaximized = commonItems::getString(theStream) == "true";
	});
	registerKeyword("unmappedFrameOn", [this](std::istream& theStream) {
		unmappedFrameOn = commonItems::getString(theStream) == "true";
	});
	registerKeyword("searchFramePos", [this](std::istream& theStream) {
		const auto& theInts = commonItems::getInts(theStream);
		if (theInts.size() == 2)
			searchFramePos = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("searchFrameSize", [this](std::istream& theStream) {
		const auto& theInts = commonItems::getInts(theStream);
		if (theInts.size() == 2)
			searchFrameSize = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("searchFrameMaximized", [this](std::istream& theStream) {
		searchFrameMaximized = commonItems::getString(theStream) == "true";
	});
	registerKeyword("searchFrameOn", [this](std::istream& theStream) {
		searchFrameOn = commonItems::getString(theStream) == "true";
	});
	registerKeyword("statusBarPos", [this](std::istream& theStream) {
		const auto& theInts = commonItems::getInts(theStream);
		if (theInts.size() == 2)
			statusBarPos = Rect(theInts[0], theInts[1]);
	});
	registerKeyword("statusBarOn", [this](std::istream& theStream) {
		statusBarOn = commonItems::getString(theStream) == "true";
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

	if (configuration.unmappedFramePos)
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

	if (configuration.searchFramePos)
		output << "searchFramePos = { " << configuration.searchFramePos->x << " " << configuration.searchFramePos->y << " }\n";
	if (configuration.searchFrameSize)
		output << "searchFrameSize = { " << configuration.searchFrameSize->x << " " << configuration.searchFrameSize->y << " }\n";
	if (configuration.searchFrameMaximized)
		output << "searchFrameMaximized = true\n";
	else
		output << "searchFrameMaximized = false\n";
	if (configuration.searchFrameOn)
		output << "searchFrameOn = true\n";
	else
		output << "searchFrameOn = false\n";

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

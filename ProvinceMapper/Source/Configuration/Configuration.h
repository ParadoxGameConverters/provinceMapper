#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "Parser.h"

class Configuration: commonItems::parser
{
  public:
	Configuration() = default;

	[[nodiscard]] const auto& getSourceDir() const { return sourceDir; }
	[[nodiscard]] const auto& getTargetDir() const { return targetDir; }
	[[nodiscard]] const auto& getSourceToken() const { return sourceToken; }
	[[nodiscard]] const auto& getTargetToken() const { return targetToken; }
	[[nodiscard]] const auto& getLinkFile() const { return linkFile; }

	void setSourceDir(const std::string& dir) { sourceDir = dir; }
	void setTargetDir(const std::string& dir) { targetDir = dir; }
	void setSourceToken(const std::string& token) { sourceToken = token; }
	void setTargetToken(const std::string& token) { targetToken = token; }
	void setLinkFile(const std::string& file) { linkFile = file; }

	void save() const;
	void load();
	
	friend std::ostream& operator<<(std::ostream& output, const Configuration& configuration);

  private:
	void registerKeys();

	std::optional<std::string> sourceDir;
	std::optional<std::string> targetDir;
	std::optional<std::string> sourceToken;
	std::optional<std::string> targetToken;
	std::optional<std::string> linkFile;
};
std::ostream& operator<<(std::ostream& output, const Configuration& configuration);
#endif // CONFIGURATION_H

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "Parser.h"

struct Rect
{
	Rect(const int ix, const int iy)
	{
		x = ix;
		y = iy;
	}
	int x = 0;
	int y = 0;
};

class Configuration: commonItems::parser
{
  public:
	Configuration() = default;

	[[nodiscard]] const auto& getSourceDir() const { return sourceDir; }
	[[nodiscard]] const auto& getTargetDir() const { return targetDir; }
	[[nodiscard]] const auto& getSourceToken() const { return sourceToken; }
	[[nodiscard]] const auto& getTargetToken() const { return targetToken; }
	[[nodiscard]] const auto& getLinkFile() const { return linkFile; }
	[[nodiscard]] const auto& isSourceReversed() const { return reverseSource; }
	[[nodiscard]] const auto& isTargetReversed() const { return reverseTarget; }
	[[nodiscard]] const auto& getImageFramePos() const { return imageFramePos; }
	[[nodiscard]] const auto& getImageFrameSize() const { return imageFrameSize; }
	[[nodiscard]] const auto& getLinksFramePos() const { return linksFramePos; }
	[[nodiscard]] const auto& getLinksFrameSize() const { return linksFrameSize; }
	[[nodiscard]] const auto& getStatusBarPos() const { return statusBarPos; }
	[[nodiscard]] const auto& isStatusBarOn() const { return statusBarOn; }

	void setSourceDir(const std::string& dir) { sourceDir = dir; }
	void setTargetDir(const std::string& dir) { targetDir = dir; }
	void setSourceToken(const std::string& token) { sourceToken = token; }
	void setTargetToken(const std::string& token) { targetToken = token; }
	void setLinkFile(const std::string& file) { linkFile = file; }
	void setSourceReversed(bool reversed) { reverseSource = reversed; }
	void setTargetReversed(bool reversed) { reverseTarget = reversed; }
	void setImageFramePos(const int x, const int y) { imageFramePos = Rect(x, y); }
	void setImageFrameSize(const int x, const int y) { imageFrameSize = Rect(x, y); }
	void setLinksFramePos(const int x, const int y) { linksFramePos = Rect(x, y); }
	void setLinksFrameSize(const int x, const int y) { linksFrameSize = Rect(x, y); }
	void setStatusBarPos(const int x, const int y) { statusBarPos = Rect(x, y); }
	void setStatusBarOn(const bool on) { statusBarOn = on; }

	void save() const;
	void load();

	friend std::ostream& operator<<(std::ostream& output, const Configuration& configuration);

  private:
	void registerKeys();

	bool reverseSource = false;
	bool reverseTarget = false;
	std::optional<std::string> sourceDir;
	std::optional<std::string> targetDir;
	std::optional<std::string> sourceToken;
	std::optional<std::string> targetToken;
	std::optional<std::string> linkFile;
	std::optional<Rect> imageFramePos;
	std::optional<Rect> imageFrameSize;
	std::optional<Rect> linksFramePos;
	std::optional<Rect> linksFrameSize;
	std::optional<Rect> statusBarPos;
	bool statusBarOn = true;
};
std::ostream& operator<<(std::ostream& output, const Configuration& configuration);
#endif // CONFIGURATION_H

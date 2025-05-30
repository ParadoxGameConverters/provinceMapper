#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Parser.h>

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
	[[nodiscard]] auto isSourceReversed() const { return reverseSource; }
	[[nodiscard]] auto isTargetReversed() const { return reverseTarget; }
	[[nodiscard]] auto isDitchAdjacencies() const { return ditchAdjacencies; }
	[[nodiscard]] const auto& getImageFramePos() const { return imageFramePos; }
	[[nodiscard]] const auto& getImageFrameSize() const { return imageFrameSize; }
	[[nodiscard]] auto isImageFrameMaximized() const { return imageFrameMaximized; }
	[[nodiscard]] const auto& getLinksFramePos() const { return linksFramePos; }
	[[nodiscard]] const auto& getLinksFrameSize() const { return linksFrameSize; }
	[[nodiscard]] auto isLinksFrameMaximized() const { return linksFrameMaximized; }
	[[nodiscard]] const auto& getUnmappedFramePos() const { return unmappedFramePos; }
	[[nodiscard]] const auto& getUnmappedFrameSize() const { return unmappedFrameSize; }
	[[nodiscard]] auto isUnmappedFrameMaximized() const { return unmappedFrameMaximized; }
	[[nodiscard]] auto isUnmappedFrameOn() const { return unmappedFrameOn; }
	[[nodiscard]] const auto& getSearchFramePos() const { return searchFramePos; }
	[[nodiscard]] const auto& getSearchFrameSize() const { return searchFrameSize; }
	[[nodiscard]] auto isSearchFrameMaximized() const { return searchFrameMaximized; }
	[[nodiscard]] auto isSearchFrameOn() const { return searchFrameOn; }
	[[nodiscard]] const auto& getStatusBarPos() const { return statusBarPos; }
	[[nodiscard]] auto isStatusBarOn() const { return statusBarOn; }

	void setSourceDir(const std::filesystem::path& dir) { sourceDir = dir; }
	void setTargetDir(const std::filesystem::path& dir) { targetDir = dir; }
	void setSourceToken(const std::string& token) { sourceToken = token; }
	void setTargetToken(const std::string& token) { targetToken = token; }
	void setLinkFile(const std::filesystem::path& file) { linkFile = file; }
	void setSourceReversed(const bool reversed) { reverseSource = reversed; }
	void setTargetReversed(const bool reversed) { reverseTarget = reversed; }
	void setDitchAdjacencies(const bool ditch) { ditchAdjacencies = ditch; }
	void setImageFramePos(const int x, const int y) { imageFramePos = Rect(x, y); }
	void setImageFrameSize(const int x, const int y) { imageFrameSize = Rect(x, y); }
	void setImageFrameMaximized(const bool on) { imageFrameMaximized = on; }
	void setLinksFramePos(const int x, const int y) { linksFramePos = Rect(x, y); }
	void setLinksFrameSize(const int x, const int y) { linksFrameSize = Rect(x, y); }
	void setLinksFrameMaximized(const bool on) { linksFrameMaximized = on; }
	void setUnmappedFramePos(const int x, const int y) { unmappedFramePos = Rect(x, y); }
	void setUnmappedFrameSize(const int x, const int y) { unmappedFrameSize = Rect(x, y); }
	void setUnmappedFrameMaximized(const bool on) { unmappedFrameMaximized = on; }
	void setUnmappedFrameOn(const bool on) { unmappedFrameOn = on; }
	void setSearchFramePos(const int x, const int y) { searchFramePos = Rect(x, y); }
	void setSearchFrameSize(const int x, const int y) { searchFrameSize = Rect(x, y); }
	void setSearchFrameMaximized(const bool on) { searchFrameMaximized = on; }
	void setSearchFrameOn(const bool on) { searchFrameOn = on; }
	void setStatusBarPos(const int x, const int y) { statusBarPos = Rect(x, y); }
	void setStatusBarOn(const bool on) { statusBarOn = on; }

	void save() const;
	void load();

	friend std::ostream& operator<<(std::ostream& output, const Configuration& configuration);

  private:
	void registerKeys();

	bool reverseSource = false;
	bool reverseTarget = false;
	bool ditchAdjacencies = false;
	std::optional<std::filesystem::path> sourceDir;
	std::optional<std::filesystem::path> targetDir;
	std::optional<std::string> sourceToken;
	std::optional<std::string> targetToken;
	std::optional<std::filesystem::path> linkFile;
	std::optional<Rect> imageFramePos;
	std::optional<Rect> imageFrameSize;
	bool imageFrameMaximized = false;
	std::optional<Rect> linksFramePos;
	std::optional<Rect> linksFrameSize;
	bool linksFrameMaximized = false;
	std::optional<Rect> unmappedFramePos;
	std::optional<Rect> unmappedFrameSize;
	bool unmappedFrameMaximized = false;
	bool unmappedFrameOn = false;
	std::optional<Rect> searchFramePos;
	std::optional<Rect> searchFrameSize;
	bool searchFrameMaximized = false;
	bool searchFrameOn = false;
	std::optional<Rect> statusBarPos;
	bool statusBarOn = true;
};
std::ostream& operator<<(std::ostream& output, const Configuration& configuration);
#endif // CONFIGURATION_H

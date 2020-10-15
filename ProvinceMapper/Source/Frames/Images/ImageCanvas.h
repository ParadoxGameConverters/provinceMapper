#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "Provinces/Province.h"

enum class ImageTabSelector
{
	SOURCE,
	TARGET
};

class LinkMappingVersion;
class ImageCanvas: public wxScrolledCanvas
{
  public:
	ImageCanvas(wxWindow* parent, ImageTabSelector theSelector, const std::shared_ptr<LinkMappingVersion>& theActiveVersion, wxImage* theImage);

	[[nodiscard]] auto getHeight() const { return height; }
	[[nodiscard]] auto getWidth() const { return width; }
	[[nodiscard]] const auto& getImageData() const { return imageData; }

	void clearShadedPixels() { shadedPixels.clear(); }
	void generateShadedPixels();
	void applyShadedPixels();
	void restoreImageData();
	
  private:
	wxImage* image;
	unsigned char* imageData;
	size_t imageDataSize = 0;
	int height = 0;
	int width = 0;
	std::vector<Pixel> shadedPixels;
	std::shared_ptr<LinkMappingVersion> activeVersion;
	ImageTabSelector selector;
};

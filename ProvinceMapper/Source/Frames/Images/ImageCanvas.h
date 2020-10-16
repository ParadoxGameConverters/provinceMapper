#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "Provinces/Pixel.h"

wxDECLARE_EVENT(wxEVT_SELECT_PROVINCE, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_DESELECT_PROVINCE, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SELECT_LINK_BY_ID, wxCommandEvent);

class wxTipWindow;
struct Province;
enum class ImageTabSelector
{
	SOURCE,
	TARGET
};

class LinkMappingVersion;
class LinkMapping;
class Definitions;
class ImageCanvas: public wxScrolledCanvas
{
  public:
	ImageCanvas(wxWindow* parent,
		 ImageTabSelector theSelector,
		 const std::shared_ptr<LinkMappingVersion>& theActiveVersion,
		 wxImage* theImage,
		 std::shared_ptr<Definitions> theDefinitions);

	[[nodiscard]] auto getHeight() const { return height; }
	[[nodiscard]] auto getWidth() const { return width; }
	[[nodiscard]] const auto& getImageData() const { return imageData; }

	void onMouseOver(wxMouseEvent& event);
	void leftUp(wxMouseEvent& event);

	void clearShadedPixels() { shadedPixels.clear(); }
	void generateShadedPixels();
	void applyShadedPixels();
	void applyStrafedPixels();
	void restoreImageData();
	void setBlack() { black = true; }
	void clearBlack() { black = false; }

	void activateLinkByIndex(int row);
	void activateLinkByID(int ID);
	void deactivateLink();

  private:
	void deselectProvince(std::shared_ptr<Province> province);
	void selectProvince(std::shared_ptr<Province> province);
	void selectLink(int linkID);

	void strafeProvinces();

	bool black = false;
	ImageTabSelector selector;

	wxImage* image;
	unsigned char* imageData;
	size_t imageDataSize = 0;
	int height = 0;
	int width = 0;

	std::vector<Pixel> shadedPixels;
	std::vector<Pixel> strafedPixels;

	std::shared_ptr<LinkMappingVersion> activeVersion;
	std::shared_ptr<Definitions> definitions;
	std::shared_ptr<LinkMapping> activeLink;

	std::pair<unsigned int, std::string> tooltipCache;

  protected:
	wxEvtHandler* eventListener = nullptr;
};

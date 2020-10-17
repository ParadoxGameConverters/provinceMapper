#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "Provinces/Pixel.h"

wxDECLARE_EVENT(wxEVT_TOGGLE_PROVINCE, wxCommandEvent);
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
	[[nodiscard]] wxPoint locateLinkCoordinates(int ID) const;

	void clearShadedPixels() { shadedPixels.clear(); }
	void generateShadedPixels();
	void applyShadedPixels();
	void applyStrafedPixels();
	void restoreImageData() const;
	void setBlack() { black = true; }
	void clearBlack() { black = false; }

	void activateLinkByIndex(int row);
	void activateLinkByID(int ID);
	void deactivateLink();
	void toggleProvinceByID(int ID);
	void shadeProvinceByID(int ID);

  private:
	void onMouseOver(wxMouseEvent& event);
	void leftUp(wxMouseEvent& event);
	void rightUp(wxMouseEvent& event);

	void stageToggleProvinceByID(int provinceID) const;
	void strafeProvinces();
	void strafeProvince(const std::shared_ptr<Province>& province);
	void dismarkProvince(const std::shared_ptr<Province>& province) const;
	void markProvince(const std::shared_ptr<Province>& province);
	void selectLink(int linkID) const;
	
	[[nodiscard]] const std::vector<std::shared_ptr<Province>>& getRelevantProvinces(const std::shared_ptr<LinkMapping>& link) const;

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

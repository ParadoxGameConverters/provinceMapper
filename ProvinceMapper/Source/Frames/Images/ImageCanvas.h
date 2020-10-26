#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "Provinces/Pixel.h"

wxDECLARE_EVENT(wxEVT_TOGGLE_PROVINCE, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SELECT_LINK_BY_ID, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_REFRESH, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_POINT_PLACED, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOUSE_AT, wxCommandEvent);

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
	[[nodiscard]] auto getScale() const { return scaleFactor; }
	[[nodiscard]] auto getOldScale() const { return oldScaleFactor; }
	[[nodiscard]] const auto& getImageData() const { return imageData; }
	[[nodiscard]] wxPoint locateLinkCoordinates(int ID) const;
	[[nodiscard]] const auto& getPoints() const { return points; }
	[[nodiscard]] std::string nameAtCoords(const wxPoint& point);

	void clearShadedPixels() { shadedPixels.clear(); }
	void clearStrafedPixels() { strafedPixels.clear(); }
	void generateShadedPixels();
	void applyShadedPixels();
	void applyStrafedPixels();
	void restoreImageData() const;
	void setBlack() { black = true; }
	void clearBlack() { black = false; }
	void clearScale() { oldScaleFactor = scaleFactor; }

	void activateLinkByIndex(int row);
	void activateLinkByID(int ID);
	void deactivateLink();
	void toggleProvinceByID(int ID);
	void shadeProvinceByID(int ID);
	void deleteActiveLink();
	void setVersion(const std::shared_ptr<LinkMappingVersion>& version) { activeVersion = version; }
	void pushZoomLevel(int zoomLevel);
	void toggleTriangulate();

  private:
	void onMouseOver(wxMouseEvent& event);
	void leftUp(wxMouseEvent& event);
	void rightUp(wxMouseEvent& event);
	void onKeyDown(wxKeyEvent& event);
	void onMouseWheel(wxMouseEvent& event);

	void stageAddComment();
	void stageDeleteLink() const;
	void stageToggleProvinceByID(int provinceID) const;
	void strafeProvinces();
	void strafeProvince(const std::shared_ptr<Province>& province);
	void dismarkProvince(const std::shared_ptr<Province>& province) const;
	void markProvince(const std::shared_ptr<Province>& province);
	void selectLink(int linkID) const;
	void zoomIn();
	void zoomOut();
	void stageRefresh() const;
	void stageMoveUp() const;
	void stageMoveDown() const;
	void stageSave() const;
	void stageAddLink() const;
	void stageMoveVersionLeft() const;
	void stageMoveVersionRight() const;
	void stagePointPlaced() const;

	[[nodiscard]] const std::vector<std::shared_ptr<Province>>& getRelevantProvinces(const std::shared_ptr<LinkMapping>& link) const;

	bool black = false;
	ImageTabSelector selector;
	int lastClickedRow = 0;

	bool triangulate = false;
	std::vector<wxPoint> points;

	wxImage* image;
	unsigned char* imageData;
	size_t imageDataSize = 0;
	int height = 0;
	int width = 0;
	double oldScaleFactor = 1.0;
	double scaleFactor = 1.0;
	int rotationDelta = 0;

	std::vector<Pixel> shadedPixels;
	std::vector<Pixel> strafedPixels;

	std::shared_ptr<LinkMappingVersion> activeVersion;
	std::shared_ptr<Definitions> definitions;
	std::shared_ptr<LinkMapping> activeLink;

	std::pair<unsigned int, std::string> tooltipCache;

  protected:
	wxEvtHandler* eventHandler = nullptr;
};

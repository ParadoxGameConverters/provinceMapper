#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "Definitions/DefinitionsInterface.h"
#include "Provinces/Pixel.h"
#include "LinkMapper/LinkMappingVersion.h"

wxDECLARE_EVENT(wxEVT_TOGGLE_PROVINCE, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SELECT_LINK_BY_ID, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SELECT_TRIANGULATION_PAIR_BY_ID, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_REFRESH, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_POINT_PLACED, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_MOUSE_AT, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SCROLL_RELEASE_H, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_SCROLL_RELEASE_V, wxCommandEvent);
wxDECLARE_EVENT(wxEVT_DELAUNAY_TRIANGULATE, wxCommandEvent);

class wxTipWindow;
class Province;
enum class ImageTabSelector
{
	SOURCE,
	TARGET
};

class LinkMapping;
class Definitions;
class ImageCanvas final: public wxScrolledCanvas
{
  public:
	ImageCanvas(wxWindow* parent,
		 ImageTabSelector theSelector,
		 const std::shared_ptr<LinkMappingVersion>& theActiveVersion,
		 wxImage* theImage,
		 std::shared_ptr<DefinitionsInterface> theDefinitions);

	[[nodiscard]] auto getHeight() const { return height; }
	[[nodiscard]] auto getWidth() const { return width; }
	[[nodiscard]] auto getScale() const { return scaleFactor; }
	[[nodiscard]] auto getOldScale() const { return oldScaleFactor; }
	[[nodiscard]] const auto& getImageData() const { return imageData; }
	[[nodiscard]] wxPoint locateLinkCoordinates(int ID) const;
	[[nodiscard]] wxPoint locateProvinceCoordinates(const std::string& ID) const;
	[[nodiscard]] const auto& getPoints() const { return points; }
	[[nodiscard]] std::string nameAtCoords(const wxPoint& point);
	[[nodiscard]] std::shared_ptr<Province> provinceAtCoords(const wxPoint& point) const;
	[[nodiscard]] auto getOldScrollH() const { return oldScrollPositionH; }
	[[nodiscard]] auto getOldScrollV() const { return oldScrollPositionV; }
	[[nodiscard]] const auto& getActiveVersion() const { return activeVersion; }
	[[nodiscard]] const auto& getActiveTriangulationPair() const { return activeVersion->getActiveTriangulationPair(); }
	[[nodiscard]] const auto& getTriangulationPairs() const { return activeVersion->getTriangulationPairs(); }
	[[nodiscard]] const auto& getDefinitions() const { return definitions; }
 
	void clearShadedPixels() { shadedPixels.clear(); }
	void clearStrafedPixels() { strafedPixels.clear(); }
	void generateShadedPixels();
	void applyShadedPixels();
	void applyStrafedPixels();
	void restoreImageData() const;
	void setBlack() { black = true; }
	void clearBlack() { black = false; }
	void clearScale() { oldScaleFactor = scaleFactor; }
	void clearOldScrollH() { oldScrollPositionH = GetScrollPos(wxHORIZONTAL); }
	void clearOldScrollV() { oldScrollPositionV = GetScrollPos(wxVERTICAL); }

	void activateLinkByIndex(int row);
	void activateLinkByID(int ID);
	void deactivateLink();
	void toggleProvinceByID(const std::string& ID);
	void shadeProvinceByID(const std::string& ID);
	void deleteActiveLink();
	void setVersion(const std::shared_ptr<LinkMappingVersion>& version) { activeVersion = version; }
	void pushZoomLevel(int zoomLevel);
	void toggleTriangulate();
	void toggleTriangulationMesh() { showTriangulationMesh = !showTriangulationMesh; }

  private:
	void onMouseOver(wxMouseEvent& event);
	void leftUp(const wxMouseEvent& event);
	void rightUp(wxMouseEvent& event);
	void onKeyDown(wxKeyEvent& event);
	void onMouseWheel(const wxMouseEvent& event);
	void onScrollRelease(wxScrollWinEvent& event);

	void stageAddComment();
	void stageDeleteLinkOrTriangulationPair() const;
	void stageToggleProvinceByID(const std::string& provinceID) const;
	void strafeProvinces();
	void strafeProvince(const std::shared_ptr<Province>& province);
	void dismarkProvince(const std::shared_ptr<Province>& province) const;
	void markProvince(const std::shared_ptr<Province>& province);
	void selectLink(int linkID) const;
	void zoomIn();
	void zoomOut();
	void stageRefresh() const;
	void stageTriangulationPairPointPlaced() const;
	void stageMoveUp() const;
	void stageMoveDown() const;
	void stageSave() const;
	void stageAddLink() const;
	void stageAddTriangulationPair() const;
	void stageAutogenerateMappings() const;
	void stageMoveVersionLeft() const;
	void stageMoveVersionRight() const;
	void stagePointPlaced() const;
	void stageDelaunayTriangulate() const;
	void stageActivateTriangulationPairByID(int ID) const;

	[[nodiscard]] const std::vector<std::shared_ptr<Province>>& getRelevantProvinces(const std::shared_ptr<LinkMapping>& link) const;

	bool black = false;
	ImageTabSelector selector;
	int lastClickedRow = 0;

	bool showTriangulationMesh = false;

	bool triangulate = false;
	std::vector<wxPoint> points; // points used for the old triangulation version

	wxImage* image;
	unsigned char* imageData;
	size_t imageDataSize = 0;
	int height = 0;
	int width = 0;
	double oldScaleFactor = 1.0;
	double scaleFactor = 1.0;
	int rotationDelta = 0;
	int oldScrollPositionV = 0;
	int oldScrollPositionH = 0;

	std::vector<Pixel> shadedPixels;
	std::vector<Pixel> strafedPixels;

	std::shared_ptr<LinkMappingVersion> activeVersion;
	std::shared_ptr<DefinitionsInterface> definitions;

	std::pair<unsigned int, std::string> tooltipCache;

  protected:
	wxEvtHandler* eventHandler = nullptr;
};

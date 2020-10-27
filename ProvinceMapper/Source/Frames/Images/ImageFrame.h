#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <optional>

class StatusBar;
class LinkMappingVersion;
class wxSplitterWindow;
class ImageCanvas;
enum class ImageTabSelector;
class Definitions;
class ImageFrame: public wxFrame
{
  public:
	ImageFrame(wxWindow* parent,
		 const std::shared_ptr<LinkMappingVersion>& theActiveVersion,
		 wxImage* sourceImg,
		 wxImage* targetImg,
		 const std::shared_ptr<Definitions>& sourceDefs,
		 const std::shared_ptr<Definitions>& targetDefs);

	void activateLinkByIndex(int row);
	void activateLinkByID(int ID);
	void deactivateLink();
	void toggleProvinceByID(int ID, bool sourceImage);
	void shadeProvinceByID(int ID, bool sourceImage);
	void centerMap(int ID);
	void deleteActiveLink();
	void setVersion(const std::shared_ptr<LinkMappingVersion>& version);
	void showToolbar() const;

  private:
	void onScrollPaint(wxPaintEvent& event);
	void onToggleOrientation(wxCommandEvent& event);
	void onToggleBlack(wxCommandEvent& event);
	void onClose(wxCloseEvent& event);
	void onRefresh(wxCommandEvent& event);
	void onTriangulate(wxCommandEvent& event);
	void onPointPlaced(wxCommandEvent& event);
	void triangulateAtPoint(wxCommandEvent& event);

	void render() const;
	void renderSource() const;
	void renderTarget() const;

	void determineTriangulationSanity();
	void buildBounds();
	wxRect sourceRect;
	wxRect targetRect;
	std::optional<wxPoint> sourcePointer;
	std::optional<wxPoint> targetPointer;

	ImageCanvas* sourceCanvas = nullptr;
	ImageCanvas* targetCanvas = nullptr;
	wxSplitterWindow* splitter = nullptr;
	StatusBar* statusBar = nullptr;

	bool black = false;
	bool triangulationIsSane = false;

	static double pointDistance(const wxPoint& point1, const wxPoint& point2);
	static wxPoint triangulate(const std::vector<wxPoint>& sources, const std::vector<wxPoint>& targets, const wxPoint& sourcePoint);

  protected:
	wxEvtHandler* eventHandler;
};

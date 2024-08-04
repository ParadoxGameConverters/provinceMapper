#pragma once
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "Definitions/DefinitionsInterface.h"
#include <optional>

class Triangle;
class StatusBar;
class LinkMappingVersion;
class wxSplitterWindow;
class ImageCanvas;
enum class ImageTabSelector;
class Configuration;
class wxAutoBufferedPaintDC;
class ImageFrame: public wxFrame
{
  public:
	ImageFrame(wxWindow* parent,
		 const wxPoint& position,
		 const wxSize& size,
		 const std::shared_ptr<LinkMappingVersion>& theActiveVersion,
		 wxImage* sourceImg,
		 wxImage* targetImg,
		 const std::shared_ptr<DefinitionsInterface>& sourceDefs,
		 const std::shared_ptr<DefinitionsInterface>& targetDefs,
		 std::shared_ptr<Configuration> theConfiguration);

	void activateLinkByIndex(int row);
	void activateTriangulationPairByIndex(int row);
	void activateLinkByID(int ID);
	void deactivateLink();
	void deactivateTriangulationPair();
	void toggleProvinceByID(const std::string& ID, bool sourceImage);
	void shadeProvinceByID(const std::string& ID, bool sourceImage);
	void centerMap(int ID);
	void centerProvince(ImageTabSelector selector, const std::string& ID);
	void deleteActiveLink();
	void deleteActiveTriangulationPair();
	void activateTriangulationPairByID(const int ID);
	void setVersion(const std::shared_ptr<LinkMappingVersion>& version);
	void showToolbar() const;
	void delaunayTriangulate();

  private:
	void onScrollPaint(wxPaintEvent& event);
	void onToggleOrientation(wxCommandEvent& event);
	void onToggleBlack(wxCommandEvent& event);
	void onClose(const wxCloseEvent& event);
	void onRefresh(const wxCommandEvent& event);
	void onTriangulate(wxCommandEvent& event);
	void onPointPlaced(const wxCommandEvent& event);
	void triangulateAtPoint(const wxCommandEvent& event);
	void onResize(wxSizeEvent& event);
	void onMove(wxMoveEvent& event);
	void onLock(const wxCommandEvent& event);
	void onScrollReleaseH(const wxCommandEvent& event);
	void onScrollReleaseV(const wxCommandEvent& event);

	void render() const;
	void renderSource() const;
	void renderTarget() const;
	void renderTriangulationMesh(wxAutoBufferedPaintDC& paintDC, bool isSourceMap) const;
	[[nodiscard]] const auto& getTriangles() { return triangles; }
	std::vector<Triangle> triangles;

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
	bool lock = false;

	static wxPoint triangulate(const std::vector<wxPoint>& sources, const std::vector<wxPoint>& targets, const wxPoint& sourcePoint);

	std::shared_ptr<Configuration> configuration;

  protected:
	wxEvtHandler* eventHandler;
};

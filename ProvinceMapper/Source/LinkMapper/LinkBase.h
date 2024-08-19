#pragma once
#include "Parser.h"

class wxColour;
class LinkBase
{
  public:
	virtual ~LinkBase() = default;
	explicit LinkBase(int theID);

	[[nodiscard]] auto getID() const { return ID; }
	[[nodiscard]] const auto& getComment() const { return comment; }
	[[nodiscard]] virtual const std::string toRowString() = 0;
	[[nodiscard]] virtual const wxColour getBaseRowColour() = 0;
	[[nodiscard]] virtual const wxColour getActiveRowColour() = 0;

	void setComment(const std::string& theComment) { comment = theComment; }

	bool operator==(const LinkBase& rhs) const;

  protected:
	int ID = 0;
	std::optional<std::string> comment;
};
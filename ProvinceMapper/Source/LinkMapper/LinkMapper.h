#ifndef LINK_MAPPER_H
#define LINK_MAPPER_H

#include "LinkMapper.h"
#include "LinkMapping.h"
#include "Parser.h"

class LinkMapper: commonItems::parser
{
  public:
	LinkMapper();
	explicit LinkMapper(std::istream& theStream);

	void exportLinks() const;
	[[nodiscard]] const auto& getLinks() const { return links; }

  private:
	void registerKeys();
	std::shared_ptr<std::vector<std::shared_ptr<LinkMapping>>> links;
};

#endif // LINK_MAPPER_H
#include "Automapper.h"

#include "Log.h"
#include "Provinces/Province.h"

#include <ranges>

void Automapper::registerMatch(const std::shared_ptr<Province>& srcProvince, const std::shared_ptr<Province>& targetProvince)
{
	const auto& srcProvinceID = srcProvince->ID;
	const auto& targetProvinceID = targetProvince->ID;

	if (sourceProvinceShares.contains(srcProvinceID))
	{
		if (sourceProvinceShares[srcProvinceID].contains(targetProvinceID))
		{
			sourceProvinceShares[srcProvinceID][targetProvinceID]++;
		}
		else
		{
			sourceProvinceShares[srcProvinceID][targetProvinceID] = 1;
		}
	}
	else
	{
		sourceProvinceShares[srcProvinceID][targetProvinceID] = 1;

		// When registering a new source province, update the cache of impassable source provinces.
		if (srcProvince->isImpassable())
		{
			srcImpassablesCache.insert(srcProvinceID);
		}
	}

	if (targetProvinceShares.contains(targetProvinceID))
	{
		if (targetProvinceShares[targetProvinceID].contains(srcProvinceID))
		{
			targetProvinceShares[targetProvinceID][srcProvinceID]++;
		}
		else
		{
			targetProvinceShares[targetProvinceID][srcProvinceID] = 1;
		}
	}
	else
	{
		targetProvinceShares[targetProvinceID][srcProvinceID] = 1;

		// When registering a new target province, update the cache of impassable target provinces.
		if (targetProvince->isImpassable())
		{
			tgtImpassablesCache.insert(targetProvinceID);
		}
	}
}

static std::map<int, std::string, std::greater<>> getHighestMatches(const std::map<std::string, int>& shares)
{
	// Flip the shares map to get the shares as keys, with descending order.
	std::map<int, std::string, std::greater<>> sharesMap;
	for (const auto& [provID, share]: shares)
	{
		sharesMap[share] = provID;
	}

	return sharesMap;
}

bool Automapper::canProvincesBeMapped(const std::string& srcProvID, const std::string& tgtProvID, const bool allowAddingToExistingLink) const
{
	// Avoid many-to-many mappings.
	if (alreadyMappedSrcProvincesCache.contains(srcProvID) && alreadyMappedTgtProvincesCache.contains(tgtProvID))
	{
		return false;
	}

	if (const auto& tgtLink = activeVersion->getLinkForTargetProvince(tgtProvID))
	{
		if (!allowAddingToExistingLink)
		{
			return false;
		}

		// If link already has multiple targets and a different source, we can't add another source.
		if (tgtLink->getTargets().size() > 1)
		{
			for (const auto& srcProv: tgtLink->getSources())
			{
				if (srcProv->ID != srcProvID)
				{
					return false;
				}
			}
		}
	}

	if (const auto& srcLink = activeVersion->getLinkForSourceProvince(srcProvID))
	{
		if (!allowAddingToExistingLink)
		{
			return false;
		}

		// If link already has multiple sources and a different target, we can't add another target.
		if (srcLink->getSources().size() > 1)
		{
			for (const auto& tgtProv: srcLink->getTargets())
			{
				if (tgtProv->ID != tgtProvID)
				{
					return false;
				}
			}
		}
	}

	return true;
}

void Automapper::mapProvinces(const std::string& srcProvID, const std::string& tgtProvID)
{
	activeVersion->addAutogeneratedLink(srcProvID, tgtProvID);

	sourceProvinceShares[srcProvID].erase(tgtProvID);
	targetProvinceShares[tgtProvID].erase(srcProvID);

	alreadyMappedSrcProvincesCache.insert(srcProvID);
	alreadyMappedTgtProvincesCache.insert(tgtProvID);
}

void Automapper::cleanUpSourceProvinceShares()
{
	for (const auto& key: srcProvincesToRemove)
	{
		sourceProvinceShares.erase(key);
	}
}

void Automapper::cleanUpTargetProvinceShares()
{
	for (const auto& key: tgtProvincesToRemove)
	{
		targetProvinceShares.erase(key);
	}
}

void Automapper::generateLinks()
{
	// General rules for the automapping:
	// - We don't modify the hand-made links.
	// - Provinces belonging to hand-made links are excluded from the automapping.
	// - We don't create many-to-many mappings.

	activeVersion->deactivateLink();

	// 1. For all non-impassable target provinces:
	//	   If the most matching source province is available and not impassable, map them.
	Log(LogLevel::Debug) << "Link generation step 1...";
	for (const auto& [tgtProvID, srcProvMatches]: targetProvinceShares)
	{
		if (tgtImpassablesCache.contains(tgtProvID))
		{
			continue;
		}

		auto highestSrcMatches = getHighestMatches(srcProvMatches);

		const auto& srcProvID = highestSrcMatches.begin()->second;
		if (srcImpassablesCache.contains(srcProvID))
		{
			continue;
		}
		if (!canProvincesBeMapped(srcProvID, tgtProvID, true))
		{
			continue;
		}

		mapProvinces(srcProvID, tgtProvID);

		// tgtProvID can be removed from targetProvinceShares because it's already mapped.
		// Schedule it for removal after the loop to avoid iterator invalidation.
		tgtProvincesToRemove.insert(tgtProvID);
	}
	cleanUpTargetProvinceShares();

	// 2. For all yet unmapped non-impassable source provinces:
	//	   If the most matching target province is available and not impassable, map them.
	Log(LogLevel::Debug) << "Link generation step 2...";
	for (const auto& [srcProvID, tgtProvMatches]: sourceProvinceShares)
	{
		if (srcImpassablesCache.contains(srcProvID))
		{
			continue;
		}
		if (tgtProvMatches.empty())
		{
			continue;
		}

		auto highestTgtMatches = getHighestMatches(tgtProvMatches);

		const auto& tgtProvID = highestTgtMatches.begin()->second;
		if (tgtImpassablesCache.contains(tgtProvID))
		{
			continue;
		}
		if (!canProvincesBeMapped(srcProvID, tgtProvID, true))
		{
			continue;
		}

		mapProvinces(srcProvID, tgtProvID);

		srcProvincesToRemove.insert(srcProvID);
	}
	cleanUpSourceProvinceShares();

	// 3. For all yet unmapped non-impassable target provinces:
	//	   Try to use the most matching available non-impassable source province to map them.
	Log(LogLevel::Debug) << "Link generation step 3...";
	for (const auto& [tgtProvID, srcProvMatches]: targetProvinceShares)
	{
		if (tgtImpassablesCache.contains(tgtProvID))
		{
			continue;
		}
		if (srcProvMatches.empty())
		{
			continue;
		}

		auto highestSrcMatches = getHighestMatches(srcProvMatches);
		for (const auto& srcProvID: highestSrcMatches | std::views::values)
		{
			if (srcImpassablesCache.contains(srcProvID))
			{
				continue;
			}
			if (!canProvincesBeMapped(srcProvID, tgtProvID, true))
			{
				continue;
			}

			mapProvinces(srcProvID, tgtProvID);

			tgtProvincesToRemove.insert(tgtProvID);
			break;
		}
	}
	cleanUpTargetProvinceShares();

	// 4. For all yet unmapped non-impassable source provinces:target
	//	   Try to use the most matching available non-impassable target province to map them.
	Log(LogLevel::Debug) << "Link generation step 4...";
	for (const auto& [srcProvID, tgtProvMatches]: sourceProvinceShares)
	{
		if (srcImpassablesCache.contains(srcProvID))
		{
			continue;
		}
		if (tgtProvMatches.empty())
		{
			continue;
		}

		auto highestTgtMatches = getHighestMatches(tgtProvMatches);
		for (const auto& tgtProvID: highestTgtMatches | std::views::values)
		{
			if (tgtImpassablesCache.contains(tgtProvID))
			{
				continue;
			}
			if (!canProvincesBeMapped(srcProvID, tgtProvID, true))
			{
				continue;
			}

			mapProvinces(srcProvID, tgtProvID);

			srcProvincesToRemove.insert(srcProvID);
			break;
		}
	}
	cleanUpSourceProvinceShares();

	// 5. For all yet unmapped target provinces:
	//    Try to use the most matching available source province to map them.
	Log(LogLevel::Debug) << "Link generation step 5...";
	for (const auto& [tgtProvID, srcProvMatches]: targetProvinceShares)
	{
		auto highestSrcMatches = getHighestMatches(srcProvMatches);
		for (const auto& srcProvID: highestSrcMatches | std::views::values)
		{
			if (!canProvincesBeMapped(srcProvID, tgtProvID, true))
			{
				continue;
			}

			mapProvinces(srcProvID, tgtProvID);

			tgtProvincesToRemove.insert(tgtProvID);
			break;
		}
	}
	cleanUpTargetProvinceShares();

	// 6. For all yet unmapped source provinces:
	//    Try to use the most matching available target province to map them.
	Log(LogLevel::Debug) << "Link generation step 6...";
	for (const auto& [srcProvID, tgtProvMatches]: sourceProvinceShares)
	{
		auto highestTgtMatches = getHighestMatches(tgtProvMatches);
		for (const auto& tgtProvID: highestTgtMatches | std::views::values)
		{
			if (!canProvincesBeMapped(srcProvID, tgtProvID, true))
			{
				continue;
			}

			mapProvinces(srcProvID, tgtProvID);

			srcProvincesToRemove.insert(srcProvID);
			break;
		}
	}
	cleanUpSourceProvinceShares();

	Log(LogLevel::Info) << "<> Automapping complete.";
}

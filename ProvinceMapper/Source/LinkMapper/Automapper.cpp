#include "Automapper.h"

#include "Frames/Images/ImageFrame.h"
#include "LinkMappingVersion.h"
#include "Provinces/Province.h"
#include "Triangle.h"
#include <Log.h>

#include <ranges>
#include <wx/taskbarbutton.h>


wxPoint triangulate(const std::vector<wxPoint>& sources, const std::vector<wxPoint>& targets, const wxPoint& sourcePoint)
{
	// move the source point in reference to the source origin
	const auto movedSource = sourcePoint - sources[0];

	// construct a basis matrix for the source triangle:
	// ( A B ) = ( x1 x2 )
	// ( C D ) = ( y1 y2 )
	const auto sourceA = static_cast<float>(sources[1].x) - static_cast<float>(sources[0].x);
	const auto sourceB = static_cast<float>(sources[2].x) - static_cast<float>(sources[0].x);
	const auto sourceC = static_cast<float>(sources[1].y) - static_cast<float>(sources[0].y);
	const auto sourceD = static_cast<float>(sources[2].y) - static_cast<float>(sources[0].y);

	// construct the inverse of the source basis matrix:
	// ___1___ ( d -b )
	// ad - bc (-c  a )
	const auto sourceDeterminant = 1 / (sourceA * sourceD - sourceB * sourceC);
	const auto inverseA = sourceDeterminant * sourceD;
	const auto inverseB = sourceDeterminant * -sourceB;
	const auto inverseC = sourceDeterminant * -sourceC;
	const auto inverseD = sourceDeterminant * sourceA;

	// transform the source point into the source triangle basis
	const auto sourceU = static_cast<float>(movedSource.x) * inverseA + static_cast<float>(movedSource.y) * inverseB;
	const auto sourceV = static_cast<float>(movedSource.x) * inverseC + static_cast<float>(movedSource.y) * inverseD;

	// silently move from source triangle basis to destination triangle basis
	const auto targetU = sourceU;
	const auto targetV = sourceV;

	// construct a basis matrix for the target triangle:
	// ( A B ) = ( x1 x2 )
	// ( C D ) = ( y1 y2 )
	const auto targetA = static_cast<float>(targets[1].x) - static_cast<float>(targets[0].x);
	const auto targetB = static_cast<float>(targets[2].x) - static_cast<float>(targets[0].x);
	const auto targetC = static_cast<float>(targets[1].y) - static_cast<float>(targets[0].y);
	const auto targetD = static_cast<float>(targets[2].y) - static_cast<float>(targets[0].y);

	// transform the target point from the destination triangle basis
	wxPoint target;
	target.x = static_cast<int>(std::round(targetU * targetA + targetV * targetB));
	target.y = static_cast<int>(std::round(targetU * targetC + targetV * targetD));

	// move the target point in reference to the source origin
	return target + targets[0];
}

static void increaseShare(std::map<std::string, int>& shares, const std::string& provID, const int amount)
{
	if (shares.contains(provID))
	{
		shares[provID] += amount;
	}
	else
	{
		shares[provID] = amount;
	}
}

inline void Automapper::determineTargetProvinceForSourcePixels(const std::shared_ptr<Province>& sourceProvince,
	 const std::vector<Pixel>& sourcePixels,
	 const PointToTriangleMap& srcPointToTriangleMap,
	 const PointToProvinceMap& tgtPointToProvinceMap,
	 const gtl::flat_hash_set<std::string>& excludedTgtProvinceIDs,
	 const int targetMapWidth,
	 const int targetMapHeight)
{
	std::map<std::shared_ptr<Province>, int> shares; // src prov, <target prov, shares>

	for (const auto& sourcePixel: sourcePixels)
	{
		const auto sourcePoint = wxPoint(sourcePixel.x, sourcePixel.y);
		const auto& triangleItr = srcPointToTriangleMap.find(sourcePoint);
		if (triangleItr == srcPointToTriangleMap.end())
		{
			// The point is outside any triangle; skip it.
			continue;
		}
		const auto& triangle = triangleItr->second;

		const auto& tgtPoint = triangulate(triangle->getSourcePoints(), triangle->getTargetPoints(), sourcePoint);

		// Skip if tgtPoint is outside the target map.
		if (tgtPoint.x < 0 || tgtPoint.x >= targetMapWidth || tgtPoint.y < 0 || tgtPoint.y >= targetMapHeight)
			continue;

		// Get tgtProvince from the packed map, or skip if not found.
		const auto& tgtProvinceItr = tgtPointToProvinceMap.find(tgtPoint);
		if (tgtProvinceItr == tgtPointToProvinceMap.end())
			continue;

		const auto& tgtProvince = tgtProvinceItr->second;
		if (excludedTgtProvinceIDs.contains(tgtProvince->ID))
			continue;

		shares[tgtProvince]++;
	}

	for (const auto& [tgtProv, amount]: shares)
	{
		registerMatch(sourceProvince, tgtProv, amount);
	}
}

void Automapper::matchTargetProvsToSourceProvs(const std::vector<std::shared_ptr<Province>>& sourceProvinces,
	 const PointToTriangleMap& srcPointToTriangleMap,
	 const PointToProvinceMap& tgtPointToLandProvinceMap,
	 const PointToProvinceMap& tgtPointToWaterProvinceMap,
	 const gtl::flat_hash_set<std::string>& excludedTgtProvinceIDs,
	 const int targetMapWidth,
	 const int targetMapHeight)
{
	// Split source provinces into chunks for parallel processing
	const size_t numThreads = std::thread::hardware_concurrency();
	const size_t chunkSize = (sourceProvinces.size() + numThreads - 1) / numThreads;

	std::vector<std::future<void>> futures;

	for (size_t i = 0; i < numThreads; ++i)
	{
		auto startIt = sourceProvinces.begin() + i * chunkSize;
		auto endIt = (i == numThreads - 1) ? sourceProvinces.end() : startIt + chunkSize;

		futures.push_back(std::async(std::launch::async, [&, startIt, endIt] {
			for (auto it = startIt; it != endIt; ++it)
			{
				const auto& srcProv = *it;

				// Skip if the source province is already mapped (implying a hand-made mapping).
				if (activeVersion->isProvinceMapped(srcProv->ID, true) == Mapping::MAPPED)
					continue;

				const auto& tgtProvsMapToUse = srcProv->isWater() ? tgtPointToWaterProvinceMap : tgtPointToLandProvinceMap;

				// Determine which target province every pixel of the source province corresponds to.
				determineTargetProvinceForSourcePixels(srcProv,
					 srcProv->innerPixels,
					 srcPointToTriangleMap,
					 tgtProvsMapToUse,
					 excludedTgtProvinceIDs,
					 targetMapWidth,
					 targetMapHeight);
				determineTargetProvinceForSourcePixels(srcProv,
					 srcProv->borderPixels,
					 srcPointToTriangleMap,
					 tgtProvsMapToUse,
					 excludedTgtProvinceIDs,
					 targetMapWidth,
					 targetMapHeight);
			}
		}));
	}

	// Wait for all tasks to complete
	for (auto& future: futures)
	{
		future.get();
	}
}

void Automapper::registerMatch(const std::shared_ptr<Province>& srcProvince, const std::shared_ptr<Province>& targetProvince, const int amount)
{
	const auto& srcProvinceID = srcProvince->ID;
	const auto& targetProvinceID = targetProvince->ID;

	std::lock_guard lock(automapperMutex); // Lock the mutex

	if (sourceProvinceShares.contains(srcProvinceID))
	{
		increaseShare(sourceProvinceShares[srcProvinceID], targetProvinceID, amount);
	}
	else
	{
		sourceProvinceShares[srcProvinceID][targetProvinceID] = amount;

		// When registering a new source province, update the cache of impassable source provinces.
		if (srcProvince->isImpassable())
		{
			srcImpassablesCache.insert(srcProvinceID);
		}
	}

	if (targetProvinceShares.contains(targetProvinceID))
	{
		increaseShare(targetProvinceShares[targetProvinceID], srcProvinceID, amount);
	}
	else
	{
		targetProvinceShares[targetProvinceID][srcProvinceID] = amount;

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
	const bool srcAlreadyMapped = alreadyMappedSrcProvincesCache.contains(srcProvID);
	const bool tgtAlreadyMapped = alreadyMappedTgtProvincesCache.contains(tgtProvID);

	if (!allowAddingToExistingLink && (srcAlreadyMapped || tgtAlreadyMapped))
	{
		return false;
	}

	// Avoid many-to-many mappings.
	if (srcAlreadyMapped && tgtAlreadyMapped)
	{
		return false;
	}

	if (const auto& tgtLink = activeVersion->getLinkForTargetProvince(tgtProvID))
	{
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
	for (const auto& ID: alreadyMappedSrcProvincesCache)
	{
		sourceProvinceShares.erase(ID);
	}
}

void Automapper::cleanUpTargetProvinceShares()
{
	for (const auto& ID: alreadyMappedTgtProvincesCache)
	{
		targetProvinceShares.erase(ID);
	}
}

void Automapper::forUnmappedTargetsMapUnmappedSources()
{
	for (const auto& [tgtProvID, srcProvMatches]: targetProvinceShares)
	{
		if (tgtImpassablesCache.contains(tgtProvID))
			continue;

		auto highestSrcMatches = getHighestMatches(srcProvMatches);
		if (highestSrcMatches.empty())
			continue;

		const auto& srcProvID = highestSrcMatches.begin()->second;
		if (srcImpassablesCache.contains(srcProvID))
			continue;
		if (!canProvincesBeMapped(srcProvID, tgtProvID, false))
			continue;

		mapProvinces(srcProvID, tgtProvID);
	}
	cleanUpTargetProvinceShares();
}

void Automapper::forUnmappedSourcesMapUnmappedTargets()
{
	for (const auto& [srcProvID, tgtProvMatches]: sourceProvinceShares)
	{
		if (srcImpassablesCache.contains(srcProvID))
			continue;
		if (tgtProvMatches.empty())
			continue;

		auto highestTgtMatches = getHighestMatches(tgtProvMatches);
		if (highestTgtMatches.empty())
			continue;

		const auto& tgtProvID = highestTgtMatches.begin()->second;
		if (tgtImpassablesCache.contains(tgtProvID))
			continue;
		if (!canProvincesBeMapped(srcProvID, tgtProvID, false))
			continue;

		mapProvinces(srcProvID, tgtProvID);
	}
	cleanUpSourceProvinceShares();
}

void Automapper::forNonImpassableTargetsMapTryToMapFirstChoices()
{
	for (const auto& [tgtProvID, srcProvMatches]: targetProvinceShares)
	{
		if (tgtImpassablesCache.contains(tgtProvID))
			continue;

		auto highestSrcMatches = getHighestMatches(srcProvMatches);
		if (highestSrcMatches.empty())
			continue;

		const auto& srcProvID = highestSrcMatches.begin()->second;
		if (srcImpassablesCache.contains(srcProvID))
			continue;
		if (!canProvincesBeMapped(srcProvID, tgtProvID, true))
			continue;

		mapProvinces(srcProvID, tgtProvID);
	}
	cleanUpTargetProvinceShares();
}

void Automapper::forNonImpassableSourcesMapTryToMapFirstChoices()
{
	for (const auto& [srcProvID, tgtProvMatches]: sourceProvinceShares)
	{
		if (srcImpassablesCache.contains(srcProvID))
			continue;
		if (tgtProvMatches.empty())
			continue;

		auto highestTgtMatches = getHighestMatches(tgtProvMatches);
		if (highestTgtMatches.empty())
			continue;

		const auto& tgtProvID = highestTgtMatches.begin()->second;
		if (tgtImpassablesCache.contains(tgtProvID))
			continue;
		if (!canProvincesBeMapped(srcProvID, tgtProvID, true))
			continue;

		mapProvinces(srcProvID, tgtProvID);
	}
	cleanUpSourceProvinceShares();
}

void Automapper::forUnmappedTargetsEvaluateAllNonImpassableMatches()
{
	for (const auto& [tgtProvID, srcProvMatches]: targetProvinceShares)
	{
		if (tgtImpassablesCache.contains(tgtProvID))
			continue;
		if (srcProvMatches.empty())
			continue;

		auto highestSrcMatches = getHighestMatches(srcProvMatches);
		for (const auto& srcProvID: highestSrcMatches | std::views::values)
		{
			if (srcImpassablesCache.contains(srcProvID))
				continue;
			if (!canProvincesBeMapped(srcProvID, tgtProvID, true))
				continue;

			mapProvinces(srcProvID, tgtProvID);
			break;
		}
	}
	cleanUpTargetProvinceShares();
}

void Automapper::forUnmappedSourcesEvaluateAllNonImpassableMatches()
{
	for (const auto& [srcProvID, tgtProvMatches]: sourceProvinceShares)
	{
		if (srcImpassablesCache.contains(srcProvID))
			continue;
		if (tgtProvMatches.empty())
			continue;

		auto highestTgtMatches = getHighestMatches(tgtProvMatches);
		for (const auto& tgtProvID: highestTgtMatches | std::views::values)
		{
			if (tgtImpassablesCache.contains(tgtProvID))
				continue;
			if (!canProvincesBeMapped(srcProvID, tgtProvID, true))
				continue;

			mapProvinces(srcProvID, tgtProvID);
			break;
		}
	}
	cleanUpSourceProvinceShares();
}

void Automapper::forUnmappedTargetsEvaluateAllMatches()
{
	for (const auto& [tgtProvID, srcProvMatches]: targetProvinceShares)
	{
		auto highestSrcMatches = getHighestMatches(srcProvMatches);
		for (const auto& srcProvID: highestSrcMatches | std::views::values)
		{
			if (!canProvincesBeMapped(srcProvID, tgtProvID, true))
				continue;

			mapProvinces(srcProvID, tgtProvID);
			break;
		}
	}
	cleanUpTargetProvinceShares();
}

void Automapper::forUnmappedSourcesEvaluateAllMatches()
{
	for (const auto& [srcProvID, tgtProvMatches]: sourceProvinceShares)
	{
		auto highestTgtMatches = getHighestMatches(tgtProvMatches);
		for (const auto& tgtProvID: highestTgtMatches | std::views::values)
		{
			if (!canProvincesBeMapped(srcProvID, tgtProvID, true))
				continue;

			mapProvinces(srcProvID, tgtProvID);
			break;
		}
	}
	cleanUpSourceProvinceShares();
}

void Automapper::forUnmappedTargetsTryToStealSourcesFromExistingLinks()
{
	for (const auto& [tgtProvID, srcProvMatches]: targetProvinceShares)
	{
		if (tgtImpassablesCache.contains(tgtProvID))
			continue;

		auto highestSrcMatches = getHighestMatches(srcProvMatches);
		for (const auto& srcProvID: highestSrcMatches | std::views::values)
		{
			if (srcImpassablesCache.contains(srcProvID))
				continue;

			const auto& srcLinkToBeMugged = activeVersion->getLinkForSourceProvince(srcProvID);
			if (!srcLinkToBeMugged || srcLinkToBeMugged->getSources().size() <= 1)
				continue;

			Log(LogLevel::Debug) << "Stealing source province " << srcProvID << " from existing mapping that contains target province "
										<< srcLinkToBeMugged->getTargets()[0]->ID << " in order to map target province " << tgtProvID;
			activeVersion->activateLinkByID(srcLinkToBeMugged->getID());
			if (activeVersion->toggleProvinceByID(srcProvID, true) != std::nullopt)
			{
				// The function should have returned nullopt because we're not creating a new link.
				Log(LogLevel::Error) << "Failed to correctly remove province " << srcProvID << " from a link.";
			}
			mapProvinces(srcProvID, tgtProvID);
			break;
		}
	}
	cleanUpTargetProvinceShares();
}

void Automapper::forUnmappedSourcesTryToStealTargetsFromExistingLinks()
{
	for (const auto& [srcProvID, tgtProvMatches]: sourceProvinceShares)
	{
		if (srcImpassablesCache.contains(srcProvID))
			continue;

		auto highestTgtMatches = getHighestMatches(tgtProvMatches);
		for (const auto& tgtProvID: highestTgtMatches | std::views::values)
		{
			if (tgtImpassablesCache.contains(tgtProvID))
				continue;

			const auto& tgtLinkToBeMugged = activeVersion->getLinkForTargetProvince(tgtProvID);
			if (!tgtLinkToBeMugged || tgtLinkToBeMugged->getTargets().size() <= 1)
				continue;

			Log(LogLevel::Debug) << "Stealing target province " << tgtProvID << " from existing mapping that contains source province "
										<< tgtLinkToBeMugged->getSources()[0]->ID << " in order to map source province " << srcProvID;
			activeVersion->activateLinkByID(tgtLinkToBeMugged->getID());
			if (activeVersion->toggleProvinceByID(tgtProvID, false) != std::nullopt)
			{
				// The function should have returned nullopt because we're not creating a new link.
				Log(LogLevel::Error) << "Failed to correctly remove province " << tgtProvID << " from a link.";
			}
			mapProvinces(srcProvID, tgtProvID);
			break;
		}
	}
	cleanUpSourceProvinceShares();
}

void Automapper::generateLinks(wxTaskBarButton* taskBarBtn)
{
	// General rules for the automapping:
	// - We don't modify the hand-made links.
	// - Provinces belonging to hand-made links are excluded from the automapping.
	// - We don't create many-to-many mappings.

	activeVersion->deactivateLink();
	int currentProgress = 3;

	// 1. For all non-impassable target provinces:
	//	   If the most matching source province is available and not impassable, map them.
	//	   Require both sides to be currently unmapped.
	Log(LogLevel::Debug) << "Link generation step 1...";
	forUnmappedTargetsMapUnmappedSources();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);

	// 2. For all yet unmapped non-impassable source provinces:
	//	   If the most matching target province is available and not impassable, map them.
	//	   Require both sides to be currently unmapped.
	Log(LogLevel::Debug) << "Link generation step 2...";
	forUnmappedSourcesMapUnmappedTargets();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);

	// 3. For all non-impassable target provinces:
	//	   If the most matching source province is available and not impassable, map them.
	Log(LogLevel::Debug) << "Link generation step 3...";
	forNonImpassableTargetsMapTryToMapFirstChoices();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);

	// 4. For all yet unmapped non-impassable source provinces:
	//	   If the most matching target province is available and not impassable, map them.
	Log(LogLevel::Debug) << "Link generation step 4...";
	forNonImpassableSourcesMapTryToMapFirstChoices();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);

	// 5. For all yet unmapped non-impassable target provinces:
	//	   Try to use the most matching available non-impassable source province to map them.
	Log(LogLevel::Debug) << "Link generation step 5...";
	forUnmappedTargetsEvaluateAllNonImpassableMatches();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);

	// 6. For all yet unmapped non-impassable source provinces:target
	//	   Try to use the most matching available non-impassable target province to map them.
	Log(LogLevel::Debug) << "Link generation step 6...";
	forUnmappedSourcesEvaluateAllNonImpassableMatches();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);

	// 7. For all yet unmapped target provinces:
	//    Try to use the most matching available source province to map them.
	Log(LogLevel::Debug) << "Link generation step 7...";
	forUnmappedTargetsEvaluateAllMatches();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);

	// 8. For all yet unmapped source provinces:
	//    Try to use the most matching available target province to map them.
	Log(LogLevel::Debug) << "Link generation step 8...";
	forUnmappedSourcesEvaluateAllMatches();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);

	// 9. For all yet unmapped non-impassable target provinces, we're running out of options, so we turn to theft.
	//    Check if we can steal a source province from an existing many-to-one link.
	Log(LogLevel::Debug) << "Link generation step 9...";
	forUnmappedTargetsTryToStealSourcesFromExistingLinks();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);

	// 10. For all yet unmapped non-impassable source provinces, we're running out of options, so we turn to theft.
	//     Check if we can steal a target province from an existing one-to-many link.
	Log(LogLevel::Debug) << "Link generation step 10...";
	forUnmappedSourcesTryToStealTargetsFromExistingLinks();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);

	// After steps 9 and 10, it may be possible to map some provinces that were previously impossible to map.
	// Repeat steps 3 to 8.
	Log(LogLevel::Debug) << "Link generation step 11...";
	forNonImpassableTargetsMapTryToMapFirstChoices();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);
	Log(LogLevel::Debug) << "Link generation step 12...";
	forNonImpassableSourcesMapTryToMapFirstChoices();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);
	Log(LogLevel::Debug) << "Link generation step 13...";
	forUnmappedTargetsEvaluateAllNonImpassableMatches();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);
	Log(LogLevel::Debug) << "Link generation step 14...";
	forUnmappedSourcesEvaluateAllNonImpassableMatches();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);
	Log(LogLevel::Debug) << "Link generation step 15...";
	forUnmappedTargetsEvaluateAllMatches();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);
	Log(LogLevel::Debug) << "Link generation step 16...";
	forUnmappedSourcesEvaluateAllMatches();
	if (taskBarBtn)
		taskBarBtn->SetProgressValue(++currentProgress);

	Log(LogLevel::Info) << "<> Automapping complete.";
}

#include "Vic3Definitions.h"
#include "OSCompatibilityLayer.h"
#include "Provinces/Pixel.h"
#include "Provinces/Province.h"
#include <fstream>
#include <iomanip>



void Vic3Definitions::registerPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	Pixel pixel(x, y, r, g, b);
	const auto& chroma_itr = chromaCache.find(pixelPack(r, g, b));
	if (chroma_itr != chromaCache.end())
	{
		chroma_itr->second->innerPixels.emplace_back(pixel);
	}
	else
	{
		std::stringstream id;
		id << "x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(r) << std::setw(2) << std::setfill('0')
			<< static_cast<int>(g) << std::setw(2) << std::setfill('0') << static_cast<int>(b);
		auto new_province = std::make_shared<Province>(id.str(), r, g, b, id.str());
		new_province->innerPixels.emplace_back(pixel);
		chromaCache.emplace(pixelPack(r, g, b), new_province);
		provinces.emplace(id.str(), new_province);
	}
}


void Vic3Definitions::registerBorderPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b)
{
	Pixel pixel(x, y, r, g, b);
	const auto& chroma_itr = chromaCache.find(pixelPack(r, g, b));
	if (chroma_itr != chromaCache.end())
	{
		chroma_itr->second->borderPixels.emplace_back(pixel);
	}
	else
	{
		std::stringstream id;
		id << "x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(r) << std::setw(2) << std::setfill('0')
			<< static_cast<int>(g) << std::setw(2) << std::setfill('0') << static_cast<int>(b);
		auto new_province = std::make_shared<Province>(id.str(), r, g, b, id.str());
		new_province->borderPixels.emplace_back(pixel);
		chromaCache.emplace(pixelPack(r, g, b), new_province);
		provinces.emplace(id.str(), new_province);
	}
}


std::optional<std::string> Vic3Definitions::getNameForChroma(const unsigned int chroma)
{
	if (const auto& chroma_cache_itr = chromaCache.find(chroma); chroma_cache_itr != chromaCache.end())
	{
		return chroma_cache_itr->second->bespokeName();
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<std::string> Vic3Definitions::getMiscForChroma(const unsigned int chroma)
{
	if (const auto& chroma_cache_itr = chromaCache.find(chroma); chroma_cache_itr != chromaCache.end())
	{
		return chroma_cache_itr->second->miscName();
	}
	else
	{
		return std::nullopt;
	}
}

std::optional<std::string> Vic3Definitions::getIDForChroma(const unsigned int chroma)
{
	if (const auto& chroma_cache_itr = chromaCache.find(chroma); chroma_cache_itr != chromaCache.end())
	{
		return chroma_cache_itr->second->ID;
	}
	else
	{
		return std::nullopt;
	}
}


std::shared_ptr<Province> Vic3Definitions::getProvinceForChroma(const unsigned int chroma)
{
	if (const auto& chroma_cache_itr = chromaCache.find(chroma); chroma_cache_itr != chromaCache.end())
	{
		return chroma_cache_itr->second;
	}
	else
	{
		return nullptr;
	}
}


std::shared_ptr<Province> Vic3Definitions::getProvinceForID(const std::string& ID)
{
	if (const auto& province_itr = provinces.find(ID); province_itr != provinces.end())
	{
		return province_itr->second;
	}
	else if (ID.starts_with('0'))
	{
		if (const auto& province_itr2 = provinces.find(ID.substr(1, ID.length())); province_itr != provinces.end())
		{
			return province_itr2->second;
		}
	}

	return nullptr;
}

void Vic3Definitions::loadLocalizations(const LocalizationMapper& localizationMapper, LocalizationMapper::LocType locType)
{
	for (const auto& [id, province]: provinces)
	{
		if (locType == LocalizationMapper::LocType::SOURCE && localizationMapper.getLocForSourceKey(id))
		{
			auto stateName = *localizationMapper.getLocForSourceKey(id);
			province->areaName = stateName;

			if (const auto& cmatch = localizationMapper.getLocForSourceKey(id + "_city"); cmatch)
			{
				if (const auto& cmatch2 = localizationMapper.getLocForSourceKey(*cmatch))
				{
					if (!province->locName)
						province->locName = *cmatch2 + " [city]";
					else
						*province->locName += "[city]";
				}
			}
			if (const auto& pmatch = localizationMapper.getLocForSourceKey(id + "_port"); pmatch)
			{
				if (const auto& pmatch2 = localizationMapper.getLocForSourceKey(*pmatch))
				{
					if (!province->locName)
						province->locName = *pmatch2 + " [port]";
					else
						*province->locName += "[port]";
				}
			}
			if (const auto& wmatch = localizationMapper.getLocForSourceKey(id + "_wood"); wmatch)
			{
				if (const auto& wmatch2 = localizationMapper.getLocForSourceKey(*wmatch))
				{
					if (!province->locName)
						province->locName = *wmatch2 + " [wood]";
					else
						*province->locName += "[wood]";
				}
			}
			if (const auto& fmatch = localizationMapper.getLocForSourceKey(id + "_farm"); fmatch)
			{
				if (const auto& fmatch2 = localizationMapper.getLocForSourceKey(*fmatch))
				{
					if (!province->locName)
						province->locName = *fmatch2 + " [farm]";
					else
						*province->locName += "[farm]";
				}
			}
			if (const auto& mmatch = localizationMapper.getLocForSourceKey(id + "_mine"); mmatch)
			{
				if (const auto& mmatch2 = localizationMapper.getLocForSourceKey(*mmatch))
				{
					if (!province->locName)
						province->locName = *mmatch2 + " [mine]";
					else
						*province->locName += "[mine]";
				}
			}
			if (!province->locName)
			{
				province->locName = province->areaName;
			}

			if (const auto& match = localizationMapper.getLocForSourceKey(stateName); match)
				province->areaName = *match;
			if (const auto& regionName = vic3regions.getParentRegionName(stateName); regionName)
			{
				if (const auto& match = localizationMapper.getLocForSourceKey(*regionName); match)
					province->regionName = *match;
				else
					province->regionName = *regionName;
			}
			if (const auto& regionName = vic3regions.getParentSuperRegionName(stateName); regionName)
			{
				if (const auto& match = localizationMapper.getLocForSourceKey(*regionName); match)
					province->superRegionName = *match;
				else
					province->superRegionName = *regionName;
			}
		}

		if (locType == LocalizationMapper::LocType::TARGET && localizationMapper.getLocForTargetKey(id))
		{
			auto stateName = *localizationMapper.getLocForTargetKey(id);
			province->areaName = stateName;

			if (const auto& cmatch = localizationMapper.getLocForTargetKey(id + "_city"); cmatch)
			{
				if (const auto& cmatch2 = localizationMapper.getLocForTargetKey(*cmatch))
				{
					if (!province->locName)
						province->locName = *cmatch2 + " [city]";
					else
						*province->locName += "[city]";
				}
			}
			if (const auto& pmatch = localizationMapper.getLocForTargetKey(id + "_port"); pmatch)
			{
				if (const auto& pmatch2 = localizationMapper.getLocForTargetKey(*pmatch))
				{
					if (!province->locName)
						province->locName = *pmatch2 + " [port]";
					else
						*province->locName += "[port]";
				}
			}
			if (const auto& wmatch = localizationMapper.getLocForTargetKey(id + "_wood"); wmatch)
			{
				if (const auto& wmatch2 = localizationMapper.getLocForTargetKey(*wmatch))
				{
					if (!province->locName)
						province->locName = *wmatch2 + " [wood]";
					else
						*province->locName += "[wood]";
				}
			}
			if (const auto& fmatch = localizationMapper.getLocForTargetKey(id + "_farm"); fmatch)
			{
				if (const auto& fmatch2 = localizationMapper.getLocForTargetKey(*fmatch))
				{
					if (!province->locName)
						province->locName = *fmatch2 + " [farm]";
					else
						*province->locName += "[farm]";
				}
			}
			if (const auto& mmatch = localizationMapper.getLocForTargetKey(id + "_mine"); mmatch)
			{
				if (const auto& mmatch2 = localizationMapper.getLocForTargetKey(*mmatch))
				{
					if (!province->locName)
						province->locName = *mmatch2 + " [mine]";
					else
						*province->locName += "[mine]";
				}
			}
			if (!province->locName)
			{
				province->locName = province->areaName;
			}

			if (const auto& match = localizationMapper.getLocForTargetKey(stateName); match)
				province->areaName = *match;
			if (const auto& regionName = vic3regions.getParentRegionName(stateName); regionName)
			{
				if (const auto& match = localizationMapper.getLocForTargetKey(*regionName); match)
					province->regionName = *match;
				else
					province->regionName = *regionName;
			}
			if (const auto& regionName = vic3regions.getParentSuperRegionName(stateName); regionName)
			{
				if (const auto& match = localizationMapper.getLocForTargetKey(*regionName); match)
					province->superRegionName = *match;
				else
					province->superRegionName = *regionName;
			}
		}
	}
}

void Vic3Definitions::loadVic3Regions(const std::string& folderPath)
{
	vic3regions.loadSuperRegions(folderPath);
}

void Vic3Definitions::registerNeighbor(unsigned int provinceChroma, unsigned int neighborChroma)
{
	if (!neighborChromas.contains(provinceChroma))
		neighborChromas.emplace(provinceChroma, std::set<unsigned int>{});
	neighborChromas.at(provinceChroma).emplace(neighborChroma);
}

std::map<unsigned int, std::set<unsigned int>> Vic3Definitions::getNeighborChromas() const
{
	return neighborChromas;
}

void Vic3Definitions::ditchAdjacencies(const std::string& fileName)
{
	std::map<std::string, std::set<std::string>> adjacencies;
	for (const auto& [sourceChroma, targetChromas]: neighborChromas)
	{
		if (const auto& sourceProvince = getIDForChroma(sourceChroma); sourceChroma)
		{
			adjacencies.emplace(*sourceProvince, std::set<std::string>{});
			for (const auto& targetChroma: targetChromas)
			{
				if (const auto& targetProvince = getIDForChroma(targetChroma); targetProvince)
					adjacencies.at(*sourceProvince).emplace(*targetProvince);
			}
		}
	}
	std::ofstream adjacenciesFile(fileName);
	for (const auto& [sourceProvince, targetProvinces]: adjacencies)
	{
		if (targetProvinces.empty())
			continue;
		adjacenciesFile << sourceProvince << " = { ";
		for (const auto& targetProvince: targetProvinces)
			adjacenciesFile << targetProvince << " ";
		adjacenciesFile << "}\n";
	}
	adjacenciesFile.close();
}

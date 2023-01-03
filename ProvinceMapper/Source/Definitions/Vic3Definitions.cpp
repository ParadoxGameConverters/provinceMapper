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


std::optional<std::string> Vic3Definitions::getNameForChroma(const int chroma)
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

std::optional<std::string> Vic3Definitions::getMiscForChroma(const int chroma)
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

std::optional<std::string> Vic3Definitions::getIDForChroma(const int chroma)
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


std::shared_ptr<Province> Vic3Definitions::getProvinceForChroma(const int chroma)
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
		if (const auto& province_itr = provinces.find(ID.substr(1, ID.length())); province_itr != provinces.end())
		{
			return province_itr->second;
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
			province->locName = stateName;
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
			province->locName = stateName;
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

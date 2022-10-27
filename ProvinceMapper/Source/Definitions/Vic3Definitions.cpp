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
		id << "0x" << std::hex << std::setw(2) << std::setfill('0') << r << g << b;
		auto new_province = std::make_shared<Province>(id.str(), r, g, b, "");
		new_province->innerPixels.emplace_back(pixel);
		chromaCache.emplace(pixelPack(r, g, b), new_province);
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
		id << "0x" << std::hex << std::setw(2) << std::setfill('0') << r << g << b;
		auto new_province = std::make_shared<Province>(id.str(), r, g, b, "");
		new_province->borderPixels.emplace_back(pixel);
		chromaCache.emplace(pixelPack(r, g, b), new_province);
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
	else
	{
		return nullptr;
	}
}

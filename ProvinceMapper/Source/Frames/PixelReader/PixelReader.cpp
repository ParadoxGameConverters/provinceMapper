#include "PixelReader.h"
#include "Definitions/Definitions.h"

void* PixelReader::Entry()
{
	unsigned char* rgb = image->GetData();
	for (auto y = 0; y < image->GetSize().GetY(); y++)
	{
		for (auto x = 0; x < image->GetSize().GetX(); x++)
		{
			auto border = true;
			// border or regular pixel?
			if (isSameColorAtCoords(x, y, x - 1, y) && isSameColorAtCoords(x, y, x + 1, y) && isSameColorAtCoords(x, y, x, y - 1) &&
				 isSameColorAtCoords(x, y, x, y + 1))
				border = false;
			const auto offs = coordsToOffset(x, y, image->GetSize().GetX());

			if (border == true)
				definitions->registerBorderPixel(x, y, rgb[offs], rgb[offs + 1], rgb[offs + 2]);
			else
				definitions->registerPixel(x, y, rgb[offs], rgb[offs + 1], rgb[offs + 2]);
		}
	}
	return nullptr;
}

bool PixelReader::isSameColorAtCoords(const int ax, const int ay, const int bx, const int by) const
{
	const auto height = image->GetSize().GetY();
	const auto width = image->GetSize().GetX();
	if (ax > width - 1 || ax < 0 || bx > width - 1 || bx < 0)
		return false;
	if (ay > height - 1 || ay < 0 || by > height - 1 || by < 0)
		return false;
	const auto offsetA = coordsToOffset(ax, ay, width);
	const auto offsetB = coordsToOffset(bx, by, width);
	unsigned char* rgb = image->GetData();

	// Override for river colors which are hardcoded at 200/200/200. They are always true so adjacent pixels are not border pixels.
	if (rgb[offsetA] == 200 && rgb[offsetA + 1] == 200 && rgb[offsetA + 2] == 200)
		return true;
	if (rgb[offsetB] == 200 && rgb[offsetB + 1] == 200 && rgb[offsetB + 2] == 200)
		return true;

	// Otherwise compare them normally.
	if (rgb[offsetA] == rgb[offsetB] && rgb[offsetA + 1] == rgb[offsetB + 1] && rgb[offsetA + 2] == rgb[offsetB + 2])
		return true;
	else
		return false;
}

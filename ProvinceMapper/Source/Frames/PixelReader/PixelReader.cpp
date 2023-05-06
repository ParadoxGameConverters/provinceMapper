#include "PixelReader.h"
#include "Log.h"

void* PixelReader::Entry()
{
	const unsigned char* rgb = image->GetData();
	for (auto y = 0; y < image->GetSize().GetY(); y++)
	{
		for (auto x = 0; x < image->GetSize().GetX(); x++)
		{
			auto border = false;
			const auto offs = coordsToOffset(x, y, image->GetSize().GetX());
			// border or regular pixel?
			if (!isSameColorAtCoords(x, y, x - 1, y))
			{
				if (x > 0)
				{
					const auto borderOffs = coordsToOffset(x - 1, y, image->GetSize().GetX());
					definitions->registerNeighbor(pixelPack(rgb[offs], rgb[offs + 1], rgb[offs + 2]),
						 pixelPack(rgb[borderOffs], rgb[borderOffs + 1], rgb[borderOffs + 2]));
				}
				border = true;
			}
			if (!isSameColorAtCoords(x, y, x + 1, y))
			{
				if (x < image->GetSize().GetX() - 1)
				{
					const auto borderOffs = coordsToOffset(x + 1, y, image->GetSize().GetX());
					definitions->registerNeighbor(pixelPack(rgb[offs], rgb[offs + 1], rgb[offs + 2]),
						 pixelPack(rgb[borderOffs], rgb[borderOffs + 1], rgb[borderOffs + 2]));
				}
				border = true;
			}
			if (!isSameColorAtCoords(x, y, x, y - 1))
			{
				if (y > 0)
				{
					const auto borderOffs = coordsToOffset(x, y - 1, image->GetSize().GetX());
					definitions->registerNeighbor(pixelPack(rgb[offs], rgb[offs + 1], rgb[offs + 2]),
						 pixelPack(rgb[borderOffs], rgb[borderOffs + 1], rgb[borderOffs + 2]));
				}
				border = true;
			}
			if (!isSameColorAtCoords(x, y, x, y + 1))
			{
				if (y < image->GetSize().GetY() - 1)
				{
					const auto borderOffs = coordsToOffset(x, y + 1, image->GetSize().GetX());
					definitions->registerNeighbor(pixelPack(rgb[offs], rgb[offs + 1], rgb[offs + 2]),
						 pixelPack(rgb[borderOffs], rgb[borderOffs + 1], rgb[borderOffs + 2]));
				}
				border = true;
			}

			if (border == true)
				definitions->registerBorderPixel(x, y, rgb[offs], rgb[offs + 1], rgb[offs + 2]);
			else
				definitions->registerPixel(x, y, rgb[offs], rgb[offs + 1], rgb[offs + 2]);
		}
	}
	Log(LogLevel::Info) << "Parsed " << image->GetSize().GetX() << "x" << image->GetSize().GetY() << " source pixels.";
	Log(LogLevel::Info) << "Neighbor chroma cache has " << definitions->getNeighborChromas().size() << " entries.";
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
	const unsigned char* rgb = image->GetData();

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

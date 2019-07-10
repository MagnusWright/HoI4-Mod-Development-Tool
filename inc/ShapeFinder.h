#ifndef SHAPEFINDER_H
#define SHAPEFINDER_H

#include "Types.h" // Pixel
#include "BitMap.h" // BitMap

namespace MapNormalizer {
    bool isAdjacent(const Pixel&, size_t, size_t);
    bool isBoundaryPixel(Pixel);
    bool doColorsMatch(Color, Color);
    bool isInImage(BitMap*, uint32_t, uint32_t);
    uint32_t xyToIndex(BitMap*, uint32_t, uint32_t);
    uint32_t xyToIndex(uint32_t, uint32_t, uint32_t);
    Pixel getAsPixel(BitMap*, uint32_t, uint32_t);

    extern std::uint32_t error_count;
    extern std::vector<Pixel> problematic_pixels;
    PolygonList findAllShapes(BitMap*, unsigned char* = nullptr,
                              std::vector<Pixel>& = problematic_pixels);
}

#endif


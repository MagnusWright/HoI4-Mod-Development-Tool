
#include "ProvinceMapBuilder.h"

#include "UniqueColorGenerator.h"

MapNormalizer::ProvinceType MapNormalizer::getProvinceType(const Color& color) {
    // TODO
    return ProvinceType::UNKNOWN; 
}

bool MapNormalizer::isCoastal(const Polygon& shape) {
    // TODO
    return false;
}

MapNormalizer::Terrain MapNormalizer::getTerrainType(const Color& color) {
    // TODO
    return Terrain::UNKNOWN;
}

size_t MapNormalizer::getContinent(const Color& color) {
    // TODO
    return 1;
}

MapNormalizer::ProvinceList MapNormalizer::createProvinceList(const PolygonList& shape_list)
{
    ProvinceList provinces;

    for(size_t i = 0; i < shape_list.size(); ++i) {
        auto&& shape = shape_list[i];

        provinces.push_back(Province{
            i,
            generateUniqueColor(i),
            getProvinceType(shape.color),
            isCoastal(shape),
            getTerrainType(shape.color),
            getContinent(shape.color)
        });
    }

    return provinces;
}


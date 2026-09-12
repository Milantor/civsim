#pragma once

#include "settings.hpp"

#include <cstdint>
#include <vector>

// tile: tile kinds, tile metadata, and the tile name lookup.
namespace civsim::tile
{
    // TILE TYPES:
    // the map is a row-major tile field and each saved byte is one tile kind.

    /**
     * The kind of a single map tile; also used as the tile id (index into TileInfos).
     */
    enum class TileType : std::uint8_t
    {
        Dirt,
        Water,
        Sand,
        Rock,
        Ore
    };

    /**
     * UV rectangle of one tile sprite inside the texture atlas.
     */
    struct SpriteInfo
    {
        float u0, v0, u1, v1;   // UV coords inside the atlas
        const char *spriteFile; // file path (used to build the atlas)
    };

    /**
     * Human-readable metadata for one TileType: name, sprite file, and atlas UVs.
     *
     * TODO: maybe there should be a TileType field here used as an id, but that is
     * not so easy — think about it.
     */
    struct TileInfo
    {
        const char *name;
        const char *spriteFile; // TODO: rework, that thing deprecated
        SpriteInfo sprite;
    };

    // inline constexpr std::array<TileInfo, 5> TileInfos{{
    //     {"dirt", settings::DirtFile},
    //     {"water", settings::WaterFile},
    //     {"sand", settings::SandFile},
    //     {"rock", settings::RockFile},
    //     {"ore", settings::OreFile},
    // }};

    /**
     * Tile metadata registry, indexed by TileType order (Dirt..Ore).
     *
     * TODO: load this from json parsing or something like that instead of hardcoding.
     */
    inline std::vector<TileInfo> TileInfos = {
        {"dirt", settings::DirtFile, {0, 0, 0, 0, settings::DirtFile}},
        {"water", settings::WaterFile, {0, 0, 0, 0, settings::WaterFile}},
        {"sand", settings::SandFile, {0, 0, 0, 0, settings::SandFile}},
        {"rock", settings::RockFile, {0, 0, 0, 0, settings::RockFile}},
        {"ore", settings::OreFile, {0, 0, 0, 0, settings::OreFile}}};

    /**
     * Turn a TileType enum into the UI-friendly tile word.
     *
     * @param tile tile kind to name
     * @return the tile name, or "unknown" when the index is out of range
     */
    inline const char *GetTileName(TileType tile)
    {
        const auto index = static_cast<std::size_t>(tile);
        if (index >= TileInfos.size())
            return "unknown";

        return TileInfos[index].name;
    }
}

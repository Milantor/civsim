#pragma once

#include "raylib-cpp.hpp"
#include "settings.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace civsim
{
    struct RiverPoint
    {
        int x;
        int y;
    };

    enum class RiverGeneratorMode : std::uint8_t
    {
        RandomEndpoints,
        FixedStartRandomEnd,
        FixedStartFixedEnd
    };

    // TILE TYPES:
    // the map is a row-major tile field and each saved byte is one tile kind.
    enum class TileType : std::uint8_t
    {
        Dirt,
        Water,
        Sand,
        Rock,
        Ore
    };

    struct TileInfo
    {
        const char *name;
        const char *spriteFile;
    };

    inline constexpr std::array<TileInfo, 5> TileInfos{{
        {"dirt", settings::DirtFile},
        {"water", settings::WaterFile},
        {"sand", settings::SandFile},
        {"rock", settings::RockFile},
        {"ore", settings::OreFile},
    }};

    // ROCK CLUSTER:
    // real connected rock component stored as the flat row-major tile indexes it owns.
    struct RockCluster
    {
        std::vector<int> tileIndexes;
    };
    // LEVEL:
    // whole generated map object in memory, with map size from settings and tile rows stored in the vector.
    struct Level
    {
        int width;
        int height;
        std::uint64_t seed;

        Level(int width = settings::MapWidth,
              int height = settings::MapHeight,
              std::optional<std::uint64_t> seed = std::nullopt);

        std::vector<TileType> tiles;
        std::vector<RockCluster> rockClusters;

        // Level::At(x, y) writes a tile by row-major map index.
        TileType &At(int x, int y);

        // Level::At(x, y) reads a tile by row-major map index.
        const TileType &At(int x, int y) const;

        // GenerateRiver() paints the water body and sand bank onto the dirt field.
        bool GenerateRiver(int riverWidth, int bankWidth);

        // GenerateRiver() paints the river from a border point list and endpoint mode.
        bool GenerateRiver(int riverWidth,
                           int bankWidth,
                           RiverGeneratorMode mode,
                           std::optional<int> startPointIndex = std::nullopt,
                           std::optional<int> endPointIndex = std::nullopt);

        // GenerateRocks() grows rock clusters on the dirt field using the tune values.
        bool GenerateRocks(float volume, float spacing, float density);
    };

        const RockCluster *findClusterByTile(const Level &level, int tileIndex);

    // CURSOR TILE:
    // tiny data box for the tile currently under the pointer.
    struct CursorTile
    {
        int x;
        int y;
        TileType type;
    };

    /**
     * Make a binary byte stream from the Level vector.
     *
     * @param level current map object
     * @param bytes output byte vector for generatedlevel.dat
     * @return true when the byte vector is filled
     */
    bool SaveLevelAsBinary(const Level &level,
                           std::vector<std::uint8_t> &bytes);

    /**
     * Build a texture-backed PNG image from the Level object.
     *
     * @param level current map object
     * @param textures tile texture registry, indexed by TileType order
     * @param pngImage output image buffer for the PNG file
     * @return true when the Image buffer is filled
     */
    bool SaveLevelAsPng(const Level &level,
                        const std::array<raylib::Texture2D, 5> &textures,
                        Image &pngImage);

    /**
     * Find the tile currently below the camera pointer.
     *
     * @param level current map object
     * @param camera current 2D camera view
     * @param origin map origin for world-space drawing
     * @param tileSize tile square size in pixels
     * @return optional cursor tile info when the pointer is inside the map
     */
    std::optional<CursorTile> GetTileUnderCursor(const Level &level,
                                                 const raylib::Camera2D &camera,
                                                 raylib::Vector2 origin,
                                                 int tileSize);

    // GetTileName() turns a TileType enum into the UI-friendly tile word.
    const char *GetTileName(TileType tile);

    /**
     * Draw the map by walking the Level array and blitting textures per tile.
     *
     * @param level current map object
     * @param textures tile texture registry, indexed by TileType order
     * @param origin map origin used for sprite placement
     */
    void DrawLevel(const Level &level,
                   const std::array<raylib::Texture2D, 5> &textures,
                   raylib::Vector2 origin);

    // Rock2OreClusterPenetratorTEST() samples one stored rock cluster with at least 50 real tiles and
    // changes a random 40% of that cluster into the new Ore tile type.
    bool Rock2OreClusterPenetratorTEST(Level &level, const RockCluster &cluster);

    // levelgen: map factory and level recipe.
    namespace levelgen
    {
        /**
         * Generate the level tile field with a plain dirt base.
         *
         * @param seed optional explicit map seed; if omitted, a fresh random seed is created.
         * @return a new Level object with a dirt field ready for other generators
         */
        Level GenerateLevel(std::optional<std::uint64_t> seed = std::nullopt);

        /**
         * Generate the default playable map recipe from the game settings knobs.
         *
         * @param seed optional explicit map seed; if omitted, a fresh random seed is created.
         * @return a new Level object with river, sand bank, and rock clusters
         */
        Level GenerateDefaultLevel(std::optional<std::uint64_t> seed = std::nullopt);
    }
}

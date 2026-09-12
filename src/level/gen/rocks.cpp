#include "level/gen/rocks.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <queue>
#include <random>
#include <vector>

namespace civsim::level::gen::rocks
{
    namespace
    {
        // integer grid coordinate used while growing rock clusters.
        struct TileCoord
        {
            int x;
            int y;
        };
    }

    bool GenerateRocks(Level &level,
                       float volume,
                       float spacing,
                       float density)
    {
        const std::uint64_t rockSeed = seed::MixSeed(level.seed, 0x524f434bULL);
        std::mt19937_64 randomEngine(rockSeed);
        std::vector<TileCoord> rockCenters;
        std::vector<TileCoord> rockFrontier;
        std::vector<TileCoord> dirtTiles;

        for (int y = 0; y < level.height; ++y)
        {
            for (int x = 0; x < level.width; ++x)
            {
                if (level.At(x, y) == tile::TileType::Dirt)
                    dirtTiles.push_back({x, y});
            }
        }

        if (dirtTiles.empty())
            return false;

        // shuffle the dirt tile list so the
        // first rock center does not always start same place.
        std::shuffle(dirtTiles.begin(), dirtTiles.end(), randomEngine);

        const float safeVolume = std::clamp(volume, 0.f, 1.f);
        const float safeSpacing = std::clamp(spacing, 0.f, 1.f);
        const float safeDensity = std::clamp(density, 0.f, 1.f);

        const int targetRockTiles = std::min(
            static_cast<int>(std::lround(dirtTiles.size() * safeVolume)),
            static_cast<int>(dirtTiles.size()));

        // density changes how many rock clusters we try.
        // the real final count is clamped to the rules.
        const int rockCount = std::clamp(
            static_cast<int>(std::lround(
                settings::RockMaxCount - safeDensity * (settings::RockMaxCount - settings::RockMinCount))),
            settings::RockMinCount,
            settings::RockMaxCount);

        if (targetRockTiles == 0)
            return false;

        // the radius estimate is a rough tile budget guess.
        const float averageRadius = std::sqrt(
            static_cast<float>(targetRockTiles) / (rockCount * std::numbers::pi_v<float>));
        const float centerGap = averageRadius * safeSpacing;

        std::uniform_int_distribution<int> tilePick(0,
                                                    static_cast<int>(dirtTiles.size()) - 1);
        int placedRockTiles = 0;

        for (int rock = 0;
             rock < rockCount && placedRockTiles < targetRockTiles;
             ++rock)
        {
            TileCoord center = dirtTiles.front();
            bool foundCenter = false;

            for (int attempt = 0;
                 attempt < settings::RockPlaceTries;
                 ++attempt)
            {
                const TileCoord candidate = dirtTiles[tilePick(randomEngine)];
                bool farEnough = true;

                for (const TileCoord &oldCenter : rockCenters)
                {
                    const float distance = std::hypot(candidate.x - oldCenter.x,
                                                      candidate.y - oldCenter.y);
                    if (distance < centerGap)
                    {
                        farEnough = false;
                        break;
                    }
                }

                if (farEnough)
                {
                    center = candidate;
                    foundCenter = true;
                    break;
                }
            }

            if (!foundCenter && !rockCenters.empty())
                continue;

            // a new rock group starts from one settled center.
            rockCenters.push_back(center);
            rockFrontier.push_back(center);

            // this cluster gets only a slice of the total rock budget.
            const int targetForRock = std::max(
                1,
                (targetRockTiles - placedRockTiles) / (rockCount - rock));
            int grownRockTiles = 0;

            while (!rockFrontier.empty() && grownRockTiles < targetForRock)
            {
                std::uniform_int_distribution<int> frontierPick(0,
                                                                static_cast<int>(rockFrontier.size()) - 1);
                const int frontierIndex = frontierPick(randomEngine);
                const TileCoord tile = rockFrontier[frontierIndex];
                rockFrontier.erase(rockFrontier.begin() + frontierIndex);
                const int tileX = tile.x;
                const int tileY = tile.y;

                if (level.At(tileX, tileY) != tile::TileType::Dirt)
                    continue;

                // turn one dirt tile into rock, then grow neighbors.
                level.At(tileX, tileY) = tile::TileType::Rock;
                ++grownRockTiles;
                ++placedRockTiles;

                const std::array<TileCoord, 4> neighbors{
                    TileCoord{tile.x - 1, tile.y},
                    TileCoord{tile.x + 1, tile.y},
                    TileCoord{tile.x, tile.y - 1},
                    TileCoord{tile.x, tile.y + 1}};

                for (const TileCoord &neighbor : neighbors)
                {
                    const int neighborX = neighbor.x;
                    const int neighborY = neighbor.y;

                    if (neighborX >= 0 && neighborX < level.width && neighborY >= 0 && neighborY < level.height && level.At(neighborX, neighborY) == tile::TileType::Dirt)
                        rockFrontier.push_back(neighbor);
                }
            }
        }

        // the hole rule checks nearby rocks, then
        // allows dirt cells to turn into rocks close to them.
        const int holeRule = std::clamp(settings::RockFillNeighborNeed,
                                        0,
                                        8);
        std::queue<std::pair<int, int>> dirtyCheck;
        std::vector<std::vector<bool>> inQueue(level.height, std::vector<bool>(level.width, false));

        const auto pushDirty = [&](int x, int y)
        {
            if (x < 0 || x >= level.width || y < 0 || y >= level.height)
                return;
            if (level.At(x, y) != tile::TileType::Dirt || inQueue[y][x])
                return;

            inQueue[y][x] = true;
            dirtyCheck.push({x, y});
        };

        const auto countRocksAround = [&](int centerX, int centerY)
        {
            int rockNear = 0;
            for (int dy = -1; dy <= 1; ++dy)
            {
                for (int dx = -1; dx <= 1; ++dx)
                {
                    if (dx == 0 && dy == 0)
                        continue;

                    const int nx = centerX + dx;
                    const int ny = centerY + dy;
                    if (nx < 0 || nx >= level.width || ny < 0 || ny >= level.height)
                        continue;

                    if (level.At(nx, ny) == tile::TileType::Rock)
                        ++rockNear;
                }
            }
            return rockNear;
        };

        for (int y = 0; y < level.height; ++y)
        {
            for (int x = 0; x < level.width; ++x)
            {
                if (level.At(x, y) != tile::TileType::Dirt)
                    continue;

                for (int dy = -1; dy <= 1; ++dy)
                {
                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        if (dx == 0 && dy == 0)
                            continue;

                        const int nx = x + dx;
                        const int ny = y + dy;
                        if (nx < 0 || nx >= level.width || ny < 0 || ny >= level.height)
                            continue;

                        if (level.At(nx, ny) == tile::TileType::Rock)
                        {
                            pushDirty(x, y);
                            break;
                        }
                    }
                }
            }
        }

        while (!dirtyCheck.empty())
        {
            const auto [x, y] = dirtyCheck.front();
            dirtyCheck.pop();

            inQueue[y][x] = false;

            if (level.At(x, y) != tile::TileType::Dirt)
                continue;

            if (countRocksAround(x, y) >= holeRule)
            {
                level.At(x, y) = tile::TileType::Rock;

                for (int dy = -1; dy <= 1; ++dy)
                {
                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        if (dx == 0 && dy == 0)
                            continue;

                        pushDirty(x + dx, y + dy);
                    }
                }
            }
        }

        level.rockClusters = level::cluster::MakeRockClusters(level);
        return true;
    }
}

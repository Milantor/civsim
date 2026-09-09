#include "level.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>
#include <random>
#include <string>
#include <vector>

namespace
{
    std::uint64_t MakeRandomSeed()
    {
        std::random_device device;
        std::mt19937_64 engine(device());
        return engine();
    }

    std::uint64_t MixSeed(std::uint64_t baseSeed,
                          std::uint64_t salt)
    {
        std::uint64_t value = baseSeed ^ (salt + 0x9e3779b97f4a7c15ULL +
                                          (baseSeed << 6) + (baseSeed >> 2));
        value ^= value >> 33;
        value *= 0xff51afd7ed558ccdULL;
        value ^= value >> 33;
        value *= 0xc4ceb9fe1a85ec53ULL;
        value ^= value >> 33;
        return value;
    }
}

namespace civsim
{
    // --------- LEVEL MODEL ---------

    // level starts with a dirt tile grid.
    Level::Level(int width,
                 int height,
                 std::optional<std::uint64_t> seed)
        : width(width),
          height(height),
          seed(seed.value_or(MakeRandomSeed())),
          tiles(std::vector<TileType>(static_cast<std::size_t>(width) * static_cast<std::size_t>(height), TileType::Dirt))
    {
    }

    TileType &Level::At(int x, int y)
    {
        return tiles[static_cast<std::size_t>(y * width + x)];
    }

    const TileType &Level::At(int x, int y) const
    {
        return tiles[static_cast<std::size_t>(y * width + x)];
    }

    // --------- SAVE / EXPORT ---------

    bool SaveLevelAsBinary(const Level &level,
                           std::vector<std::uint8_t> &bytes)
    {
        bytes.clear();
        bytes.reserve(level.tiles.size());

        for (const TileType tile : level.tiles)
        {
            bytes.push_back(static_cast<std::uint8_t>(tile));
        }

        return !bytes.empty();
    }

    bool SaveLevelAsPng(const Level &level,
                        const std::array<raylib::Texture2D, 5> &textures,
                        Image &pngImage)
    {
        std::array<Image, 5> images{};
        for (std::size_t i = 0; i < images.size(); ++i)
            images[i] = LoadImageFromTexture(textures[i]);

        pngImage = GenImageColor(level.width * settings::TileSize,
                                 level.height * settings::TileSize,
                                 BLANK);

        for (int y = 0; y < level.height; ++y)
        {
            for (int x = 0; x < level.width; ++x)
            {
                const TileType tile = level.At(x, y);
                const std::size_t index = static_cast<std::size_t>(tile);
                if (index >= textures.size())
                    continue;

                Image src = images[index];
                const Rectangle srcRec{0.f, 0.f,
                                       static_cast<float>(src.width),
                                       static_cast<float>(src.height)};
                const Rectangle dstRec{static_cast<float>(x * settings::TileSize),
                                       static_cast<float>(y * settings::TileSize),
                                       static_cast<float>(settings::TileSize),
                                       static_cast<float>(settings::TileSize)};

                ImageDraw(&pngImage,
                          src,
                          srcRec,
                          dstRec,
                          WHITE);
            }
        }

        for (Image &image : images)
            UnloadImage(image);

        return true;
    }

    // --------- ROCK GENERATION ---------

    // rock clusters are discovered from dirt tiles.
    namespace detail
    {
        std::vector<RockCluster> MakeRockClusters(const Level &level)
        {
            std::vector<RockCluster> clusters;
            std::vector<std::vector<bool>> seen(level.height,
                                                std::vector<bool>(level.width, false));

            for (int y = 0; y < level.height; ++y)
            {
                for (int x = 0; x < level.width; ++x)
                {
                    if (level.At(x, y) != TileType::Rock || seen[y][x])
                        continue;

                    RockCluster cluster;
                    std::queue<std::pair<int, int>> frontier;
                    frontier.push({x, y});
                    seen[y][x] = true;

                    while (!frontier.empty())
                    {
                        const auto [cx, cy] = frontier.front();
                        frontier.pop();

                        const int flatIndex = cy * level.width + cx;
                        cluster.tileIndexes.push_back(flatIndex);

                        for (int dy = -1; dy <= 1; ++dy)
                        {
                            for (int dx = -1; dx <= 1; ++dx)
                            {
                                if (dx == 0 && dy == 0)
                                    continue;

                                const int nx = cx + dx;
                                const int ny = cy + dy;
                                if (nx < 0 || nx >= level.width ||
                                    ny < 0 || ny >= level.height ||
                                    seen[ny][nx] ||
                                    level.At(nx, ny) != TileType::Rock)
                                {
                                    continue;
                                }

                                seen[ny][nx] = true;
                                frontier.push({nx, ny});
                            }
                        }
                    }

                    if (!cluster.tileIndexes.empty())
                        clusters.push_back(std::move(cluster));
                }
            }

            return clusters;
        }
    }

    bool Level::GenerateRocks(float volume, float spacing, float density)
    {
        const std::uint64_t rockSeed = MixSeed(this->seed, 0x524f434bULL);
        std::mt19937_64 randomEngine(rockSeed);
        std::vector<raylib::Vector2> rockCenters;
        std::vector<raylib::Vector2> rockFrontier;
        std::vector<raylib::Vector2> dirtTiles;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                if (At(x, y) == TileType::Dirt)
                    dirtTiles.push_back(raylib::Vector2{static_cast<float>(x), static_cast<float>(y)});
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
            static_cast<float>(targetRockTiles) / (rockCount * PI));
        const float centerGap = averageRadius * safeSpacing;

        std::uniform_int_distribution<int> tilePick(0,
                                                    static_cast<int>(dirtTiles.size()) - 1);
        int placedRockTiles = 0;

        for (int rock = 0;
             rock < rockCount && placedRockTiles < targetRockTiles;
             ++rock)
        {
            raylib::Vector2 center = dirtTiles.front();
            bool foundCenter = false;

            for (int attempt = 0;
                 attempt < settings::RockPlaceTries;
                 ++attempt)
            {
                const raylib::Vector2 candidate = dirtTiles[tilePick(randomEngine)];
                bool farEnough = true;

                for (const raylib::Vector2 &oldCenter : rockCenters)
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
                const raylib::Vector2 tile = rockFrontier[frontierIndex];
                rockFrontier.erase(rockFrontier.begin() + frontierIndex);
                const int tileX = static_cast<int>(tile.x);
                const int tileY = static_cast<int>(tile.y);

                if (At(tileX, tileY) != TileType::Dirt)
                    continue;

                // turn one dirt tile into rock, then grow neighbors.
                At(tileX, tileY) = TileType::Rock;
                ++grownRockTiles;
                ++placedRockTiles;

                const std::array<raylib::Vector2, 4> neighbors{
                    raylib::Vector2{tile.x - 1.f, tile.y},
                    raylib::Vector2{tile.x + 1.f, tile.y},
                    raylib::Vector2{tile.x, tile.y - 1.f},
                    raylib::Vector2{tile.x, tile.y + 1.f}};

                for (const raylib::Vector2 &neighbor : neighbors)
                {
                    const int neighborX = static_cast<int>(neighbor.x);
                    const int neighborY = static_cast<int>(neighbor.y);

                    if (neighborX >= 0 && neighborX < width && neighborY >= 0 && neighborY < height && At(neighborX, neighborY) == TileType::Dirt)
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
        std::vector<std::vector<bool>> inQueue(height, std::vector<bool>(width, false));

        const auto pushDirty = [&](int x, int y)
        {
            if (x < 0 || x >= width || y < 0 || y >= height)
                return;
            if (At(x, y) != TileType::Dirt || inQueue[y][x])
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
                    if (nx < 0 || nx >= width || ny < 0 || ny >= height)
                        continue;

                    if (At(nx, ny) == TileType::Rock)
                        ++rockNear;
                }
            }
            return rockNear;
        };

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                if (At(x, y) != TileType::Dirt)
                    continue;

                for (int dy = -1; dy <= 1; ++dy)
                {
                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        if (dx == 0 && dy == 0)
                            continue;

                        const int nx = x + dx;
                        const int ny = y + dy;
                        if (nx < 0 || nx >= width || ny < 0 || ny >= height)
                            continue;

                        if (At(nx, ny) == TileType::Rock)
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

            if (At(x, y) != TileType::Dirt)
                continue;

            if (countRocksAround(x, y) >= holeRule)
            {
                At(x, y) = TileType::Rock;

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

        rockClusters = detail::MakeRockClusters(*this);
        return true;
    }

    // --------- RIVER GENERATION HELPERS ---------

    // river points are sampled from the map edges.
    namespace detail
    {
        std::vector<RiverPoint> MakeRiverPoints(const Level &level,
                                                std::uint64_t seed)
        {
            const int width = level.width;
            const int height = level.height;
            const int thirdWidth = std::max(1, width / 3);
            const int thirdHeight = std::max(1, height / 3);

            std::mt19937_64 randomEngine(seed);
            const auto pick = [&](int low, int high) -> int
            {
                if (low > high)
                    std::swap(low, high);

                if (low == high)
                    return low;

                std::uniform_int_distribution<int> dist(low, high);
                return dist(randomEngine);
            };

            std::vector<RiverPoint> points;

            points.push_back({0, 0});
            points.push_back({pick(0, thirdWidth - 1), 0});
            points.push_back({pick(thirdWidth, 2 * thirdWidth - 1), 0});
            points.push_back({pick(2 * thirdWidth, width - 1), 0});
            points.push_back({width - 1, 0});
            points.push_back({width - 1, pick(0, thirdHeight - 1)});
            points.push_back({width - 1, pick(thirdHeight, 2 * thirdHeight - 1)});
            points.push_back({width - 1, pick(2 * thirdHeight, height - 1)});
            points.push_back({width - 1, height - 1});
            points.push_back({pick(2 * thirdWidth, width - 1), height - 1});
            points.push_back({pick(thirdWidth, 2 * thirdWidth - 1), height - 1});
            points.push_back({pick(0, thirdWidth - 1), height - 1});
            points.push_back({0, height - 1});
            points.push_back({0, pick(2 * thirdHeight, height - 1)});
            points.push_back({0, pick(thirdHeight, 2 * thirdHeight - 1)});
            points.push_back({0, pick(0, thirdHeight - 1)});

            return points;
        }

        std::optional<int> ChooseValidEndPointIndex(const std::vector<RiverPoint> &points,
                                                    int startIndex,
                                                    RiverGeneratorMode mode)
        {
            if (points.empty())
                return std::nullopt;

            const int count = static_cast<int>(points.size());
            const int minLag = 4;

            std::vector<int> candidates;
            for (int i = 0; i < count; ++i)
            {
                const int circularDistance = std::min(std::abs(i - startIndex), count - std::abs(i - startIndex));
                if (circularDistance >= minLag)
                    candidates.push_back(i);
            }

            if (candidates.empty())
                return std::nullopt;

            std::mt19937 randomEngine(std::random_device{}());
            std::uniform_int_distribution<int> pick(0, static_cast<int>(candidates.size()) - 1);
            return candidates[pick(randomEngine)];
        }

        std::optional<RiverPoint> TryGetPoint(const std::vector<RiverPoint> &points,
                                              std::optional<int> pointIndex)
        {
            if (!pointIndex.has_value())
                return std::nullopt;

            if (pointIndex.value() < 0 || pointIndex.value() >= static_cast<int>(points.size()))
                return std::nullopt;

            return points[pointIndex.value()];
        }

        float DistanceToSegment(float pointX, float pointY,
                                float startX, float startY, float endX, float endY)
        {
            const float segmentX = endX - startX;
            const float segmentY = endY - startY;
            const float segmentLengthSquared = segmentX * segmentX + segmentY * segmentY;

            if (segmentLengthSquared == 0.f)
                return std::hypot(pointX - startX, pointY - startY);

            const float position = std::clamp(
                ((pointX - startX) * segmentX + (pointY - startY) * segmentY) / segmentLengthSquared,
                0.f,
                1.f);
            const float closestX = startX + position * segmentX;
            const float closestY = startY + position * segmentY;
            return std::hypot(pointX - closestX, pointY - closestY);
        }
    }

    // --------- LEVEL FACTORY ---------

    namespace levelgen
    {
        Level GenerateLevel(std::optional<std::uint64_t> seed)
        {
            return Level(settings::MapWidth,
                         settings::MapHeight,
                         seed);
        }

        Level GenerateDefaultLevel(std::optional<std::uint64_t> seed)
        {
            Level level = GenerateLevel(seed);
            level.GenerateRiver(settings::RiverWidth,
                                settings::RiverBankWidth,
                                RiverGeneratorMode::RandomEndpoints);
            level.GenerateRocks(settings::RockVolume,
                                settings::RockSpacing,
                                settings::RockDensity);
            return level;
        }
    }

    // --------- RIVER GENERATION ---------

    bool Level::GenerateRiver(int riverWidth,
                              int bankWidth)
    {
        return GenerateRiver(riverWidth,
                             bankWidth,
                             RiverGeneratorMode::RandomEndpoints);
    }

    bool Level::GenerateRiver(int riverWidth,
                              int bankWidth,
                              RiverGeneratorMode mode,
                              std::optional<int> startPointIndex,
                              std::optional<int> endPointIndex)
    {
        const std::uint64_t riverSeed = MixSeed(this->seed, 0x52495652ULL);
        // use one river stream from the level seed,
        // so the same map seed makes the same river.
        const auto points = detail::MakeRiverPoints(*this, riverSeed);
        const int pointCount = static_cast<int>(points.size());

        int startIndex = -1;
        int endIndex = -1;
        std::mt19937_64 randomEngine(riverSeed);

        if (mode == RiverGeneratorMode::RandomEndpoints)
        {
            std::uniform_int_distribution<int> pick(0, pointCount - 1);
            startIndex = pick(randomEngine);

            std::vector<int> candidates;
            for (int i = 0; i < pointCount; ++i)
            {
                const int circularDistance = std::min(std::abs(i - startIndex), pointCount - std::abs(i - startIndex));
                if (circularDistance >= 4)
                    candidates.push_back(i);
            }

            if (candidates.empty())
                return false;

            std::uniform_int_distribution<int> pickEnd(0, static_cast<int>(candidates.size()) - 1);
            endIndex = candidates[pickEnd(randomEngine)];
        }
        else if (mode == RiverGeneratorMode::FixedStartRandomEnd)
        {
            if (!startPointIndex.has_value())
                return false;

            startIndex = startPointIndex.value();
            if (startIndex < 0 || startIndex >= pointCount)
                return false;

            std::vector<int> candidates;
            for (int i = 0; i < pointCount; ++i)
            {
                const int circularDistance = std::min(std::abs(i - startIndex), pointCount - std::abs(i - startIndex));
                if (circularDistance >= 4)
                    candidates.push_back(i);
            }

            if (candidates.empty())
                return false;

            std::uniform_int_distribution<int> pickEnd(0, static_cast<int>(candidates.size()) - 1);
            endIndex = candidates[pickEnd(randomEngine)];
        }
        else if (mode == RiverGeneratorMode::FixedStartFixedEnd)
        {
            if (!startPointIndex.has_value() || !endPointIndex.has_value())
                return false;

            startIndex = startPointIndex.value();
            endIndex = endPointIndex.value();

            if (startIndex < 0 || startIndex >= pointCount ||
                endIndex < 0 || endIndex >= pointCount)
                return false;

            const int circularDistance = std::min(std::abs(endIndex - startIndex),
                                                  pointCount - std::abs(endIndex - startIndex));
            if (circularDistance < 4)
                return false;
        }
        else
        {
            return false;
        }

        const RiverPoint &riverStart = points[startIndex];
        const RiverPoint &riverEnd = points[endIndex];

        // use the river width and bank width as the zones.
        // water wins inside the river radius, sand sits in bank.
        const float riverRadiusInTiles = riverWidth / 2.f;
        const float bankRadiusInTiles = riverRadiusInTiles + bankWidth;

        const int x0 = riverStart.x;
        const int y0 = riverStart.y;
        const int x1 = riverEnd.x;
        const int y1 = riverEnd.y;

        const float dx = static_cast<float>(x1 - x0);
        const float dy = static_cast<float>(y1 - y0);
        const float length = std::max(1.f, std::hypot(dx, dy));

        const float normalX = -dy / length;
        const float normalY = dx / length;

        // pull the bend wiggle and the internal route count from the shared river tune values.
        const int maxSwing = std::max(1, settings::RiverWiggleSpread);
        std::uniform_int_distribution<int> swing(-maxSwing, maxSwing);

        std::vector<RiverPoint> routePoints;
        routePoints.push_back(RiverPoint{x0, y0});

        // routePoints samples a line from start to end,
        // and each route point gets a tiny wiggle side push.
        const int steps = std::max(2, settings::RiverRouteSegments);

        for (int i = 1; i < steps; ++i)
        {
            const int px = x0 + static_cast<int>(std::round((static_cast<float>(i) / steps) * (x1 - x0)));
            const int py = y0 + static_cast<int>(std::round((static_cast<float>(i) / steps) * (y1 - y0)));

            const int side = swing(randomEngine);
            const int pxWiggle = static_cast<int>(std::round(normalX * side));
            const int pyWiggle = static_cast<int>(std::round(normalY * side));

            const int wx = std::clamp(px + pxWiggle, 0, width - 1);
            const int wy = std::clamp(py + pyWiggle, 0, height - 1);
            routePoints.push_back(RiverPoint{wx, wy});
        }

        routePoints.push_back(RiverPoint{x1, y1});

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const float tileX = static_cast<float>(x);
                const float tileY = static_cast<float>(y);
                float distance = std::numeric_limits<float>::max();

                // keep the closest river segment for every tile.
                // a tile picks the closest segment line,
                // then becomes water or sand by distance.
                // each tile checks every route segment and picks
                // the shortest distance from the river path.
                for (int i = 0; i + 1 < static_cast<int>(routePoints.size()); ++i)
                {
                    const RiverPoint &a = routePoints[i];
                    const RiverPoint &b = routePoints[i + 1];
                    distance = std::min(distance,
                                        detail::DistanceToSegment(tileX,
                                                                  tileY,
                                                                  static_cast<float>(a.x),
                                                                  static_cast<float>(a.y),
                                                                  static_cast<float>(b.x),
                                                                  static_cast<float>(b.y)));
                }

                if (distance <= riverRadiusInTiles)
                    At(x, y) = TileType::Water;
                else if (distance <= bankRadiusInTiles)
                    At(x, y) = TileType::Sand;
            }
        }

        return true;
    }

    // --------- CURSOR / LOOKUP / DISPLAY ---------

    std::optional<CursorTile> GetTileUnderCursor(const Level &level,
                                                 const raylib::Camera2D &camera,
                                                 raylib::Vector2 origin,
                                                 int tileSize)
    {
        // screen to world
        const float worldX = (GetMouseX() - camera.offset.x) / camera.zoom + camera.target.x;
        const float worldY = (GetMouseY() - camera.offset.y) / camera.zoom + camera.target.y;
        const int tileX = static_cast<int>(std::floor((worldX - origin.x) / tileSize));
        const int tileY = static_cast<int>(std::floor((worldY - origin.y) / tileSize));

        if (tileX < 0 || tileX >= level.width || tileY < 0 || tileY >= level.height)
            return std::nullopt;

        return CursorTile{tileX, tileY, level.At(tileX, tileY)};
    }

    const char *GetTileName(TileType tile)
    {
        const auto index = static_cast<std::size_t>(tile);
        if (index >= TileInfos.size())
            return "unknown";

        return TileInfos[index].name;
    }

    bool TESTFUNC(Level &level)
    {
        if (level.rockClusters.empty())
            return false;

        std::vector<std::size_t> eligible;
        eligible.reserve(level.rockClusters.size());
        for (std::size_t i = 0; i < level.rockClusters.size(); ++i)
        {
            if (level.rockClusters[i].tileIndexes.size() >= 50)
                eligible.push_back(i);
        }

        if (eligible.empty())
            return false;

        std::mt19937 randomEngine(std::random_device{}());
        std::uniform_int_distribution<std::size_t> clusterPick(0,
                                                               eligible.size() - 1);
        const std::size_t clusterIndex = eligible[clusterPick(randomEngine)];
        RockCluster &cluster = level.rockClusters[clusterIndex];

        std::vector<int> sampled = cluster.tileIndexes;
        std::shuffle(sampled.begin(), sampled.end(), randomEngine);

        const int tileReplaceCount = std::max(1,
                                              static_cast<int>(std::lround(sampled.size() * 0.40f)));

        for (int i = 0; i < tileReplaceCount; ++i)
        {
            const int flatIndex = sampled[i];
            const int x = flatIndex % level.width;
            const int y = flatIndex / level.width;
            level.At(x, y) = TileType::Ore;
        }

        return true;
    }

    void DrawLevel(const Level &level,
                   const std::array<raylib::Texture2D, 5> &textures,
                   raylib::Vector2 origin)
    {
        for (int y = 0; y < level.height; ++y)
        {
            for (int x = 0; x < level.width; ++x)
            {
                const auto index = static_cast<std::size_t>(level.At(x, y));
                if (index >= textures.size())
                    continue;

                const raylib::Texture2D &texture = textures[index];
                DrawTexture(texture,
                            static_cast<int>(origin.x + x * settings::TileSize),
                            static_cast<int>(origin.y + y * settings::TileSize),
                            WHITE);
            }
        }
    }
}
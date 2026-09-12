#include "level/gen/river.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <vector>

namespace civsim::level::gen::river
{
    // helper forward declarations (definitions live at the bottom).
    namespace
    {
        std::vector<RiverPoint> MakeRiverPoints(const Level &level,
                                                std::uint64_t seed);
        std::optional<int> ChooseValidEndPointIndex(const std::vector<RiverPoint> &points,
                                                    int startIndex,
                                                    RiverGeneratorMode mode);
        std::optional<RiverPoint> TryGetPoint(const std::vector<RiverPoint> &points,
                                              std::optional<int> pointIndex);
        float DistanceToSegment(float pointX, float pointY,
                                float startX, float startY, float endX, float endY);
    }

    bool GenerateRiver(Level &level,
                       int riverWidth,
                       int bankWidth)
    {
        return GenerateRiver(level,
                             riverWidth,
                             bankWidth,
                             RiverGeneratorMode::RandomEndpoints);
    }

    bool GenerateRiver(Level &level,
                       int riverWidth,
                       int bankWidth,
                       RiverGeneratorMode mode,
                       std::optional<int> startPointIndex,
                       std::optional<int> endPointIndex)
    {
        const std::uint64_t riverSeed = seed::MixSeed(level.seed, 0x52495652ULL);
        // use one river stream from the level seed,
        // so the same map seed makes the same river.
        const auto points = MakeRiverPoints(level, riverSeed);
        const int pointCount = static_cast<int>(points.size());

        int startIndex = -1;
        int endIndex = -1;
        std::mt19937_64 randomEngine(riverSeed);

        // pick the two border points the river runs between, based on the mode.
        if (mode == RiverGeneratorMode::RandomEndpoints)
        {
            std::uniform_int_distribution<int> pick(0, pointCount - 1);
            startIndex = pick(randomEngine);

            // end must be far enough from start around the border loop.
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

            // reject too-short rivers (start and end too close around the loop).
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

        // direction of the straight start->end line, and its length.
        const float dx = static_cast<float>(x1 - x0);
        const float dy = static_cast<float>(y1 - y0);
        const float length = std::max(1.f, std::hypot(dx, dy));

        // unit normal (perpendicular) to the line: used to push wiggle sideways.
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
            // base point along the straight line, at fraction i/steps.
            const int px = x0 + static_cast<int>(std::round((static_cast<float>(i) / steps) * (x1 - x0)));
            const int py = y0 + static_cast<int>(std::round((static_cast<float>(i) / steps) * (y1 - y0)));

            // random sideways offset along the normal, so the river bends.
            const int side = swing(randomEngine);
            const int pxWiggle = static_cast<int>(std::round(normalX * side));
            const int pyWiggle = static_cast<int>(std::round(normalY * side));

            // clamp the wiggled point back inside the map.
            const int wx = std::clamp(px + pxWiggle, 0, level.width - 1);
            const int wy = std::clamp(py + pyWiggle, 0, level.height - 1);
            routePoints.push_back(RiverPoint{wx, wy});
        }

        routePoints.push_back(RiverPoint{x1, y1});

        // for every tile, find the closest route segment, then paint
        // water inside the river radius and sand inside the bank radius.
        for (int y = 0; y < level.height; ++y)
        {
            for (int x = 0; x < level.width; ++x)
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
                                        DistanceToSegment(tileX,
                                                          tileY,
                                                          static_cast<float>(a.x),
                                                          static_cast<float>(a.y),
                                                          static_cast<float>(b.x),
                                                          static_cast<float>(b.y)));
                }

                if (distance <= riverRadiusInTiles)
                    level.At(x, y) = tile::TileType::Water;
                else if (distance <= bankRadiusInTiles)
                    level.At(x, y) = tile::TileType::Sand;
            }
        }

        return true;
    }
}

namespace civsim::level::gen::river
{
    // river points are sampled from the map edges.
    namespace
    {
        // MakeRiverPoints() walks the map border and drops 16 candidate
        // entry/exit points: 4 per edge, each in its own third of that edge.
        // The river later picks two of these as start and end.
        std::vector<RiverPoint> MakeRiverPoints(const Level &level,
                                                std::uint64_t seed)
        {
            const int width = level.width;
            const int height = level.height;
            // split each edge into thirds so points spread around the border.


            
            const int thirdWidth = std::max(1, width / 3);
            const int thirdHeight = std::max(1, height / 3);

            std::mt19937_64 randomEngine(seed);
            // pick() returns a random int in [low, high], swapping if needed.
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

            // top edge: corners plus one random point per third.
            points.push_back({0, 0});
            points.push_back({pick(0, thirdWidth - 1), 0});
            points.push_back({pick(thirdWidth, 2 * thirdWidth - 1), 0});
            points.push_back({pick(2 * thirdWidth, width - 1), 0});
            // right edge.
            points.push_back({width - 1, 0});
            points.push_back({width - 1, pick(0, thirdHeight - 1)});
            points.push_back({width - 1, pick(thirdHeight, 2 * thirdHeight - 1)});
            points.push_back({width - 1, pick(2 * thirdHeight, height - 1)});
            // bottom edge.
            points.push_back({width - 1, height - 1});
            points.push_back({pick(2 * thirdWidth, width - 1), height - 1});
            points.push_back({pick(thirdWidth, 2 * thirdWidth - 1), height - 1});
            points.push_back({pick(0, thirdWidth - 1), height - 1});
            // left edge.
            points.push_back({0, height - 1});
            points.push_back({0, pick(2 * thirdHeight, height - 1)});
            points.push_back({0, pick(thirdHeight, 2 * thirdHeight - 1)});
            points.push_back({0, pick(0, thirdHeight - 1)});

            return points;
        }

        // ChooseValidEndPointIndex() picks a random border point that is
        // far enough (>= minLag) from the start, measured around the border loop.
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
                // circular distance: how far apart the two points are around the loop.
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

        // TryGetPoint() safely reads a border point by index, or nullopt if out of range.
        std::optional<RiverPoint> TryGetPoint(const std::vector<RiverPoint> &points,
                                              std::optional<int> pointIndex)
        {
            if (!pointIndex.has_value())
                return std::nullopt;

            if (pointIndex.value() < 0 || pointIndex.value() >= static_cast<int>(points.size()))
                return std::nullopt;

            return points[pointIndex.value()];
        }

        // DistanceToSegment() returns the shortest distance from a point
        // to the line segment (start -> end). Used to measure how far a
        // tile is from the river path.
        float DistanceToSegment(float pointX, float pointY,
                                float startX, float startY, float endX, float endY)
        {
            const float segmentX = endX - startX;
            const float segmentY = endY - startY;
            const float segmentLengthSquared = segmentX * segmentX + segmentY * segmentY;

            // degenerate segment: just measure point-to-point.
            if (segmentLengthSquared == 0.f)
                return std::hypot(pointX - startX, pointY - startY);

            // project the point onto the segment, clamped to [0, 1] so it stays on the segment.
            const float position = std::clamp(
                ((pointX - startX) * segmentX + (pointY - startY) * segmentY) / segmentLengthSquared,
                0.f,
                1.f);
            const float closestX = startX + position * segmentX;
            const float closestY = startY + position * segmentY;
            return std::hypot(pointX - closestX, pointY - closestY);
        }
    }
}

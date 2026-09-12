#include "level/cluster.hpp"
#include "level/level.hpp"

#include <queue>
#include <utility>
#include <vector>

namespace civsim::level::cluster
{
    const RockCluster *FindClusterByTile(const Level &level, int tileIndex)
    {
        for (const auto &cluster : level.rockClusters)
        {
            for (int idx : cluster.tileIndexes)
            {
                if (idx == tileIndex)
                {
                    return &cluster;
                }
            }
        }
        return nullptr; // not found
    }

    // rock clusters are discovered from dirt tiles.
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

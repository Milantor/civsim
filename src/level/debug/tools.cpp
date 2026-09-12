#include "level/debug/tools.hpp"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

namespace civsim::level::debug
{
    bool Rock2OreClusterPenetratorTEST(Level &level, const cluster::RockCluster &cluster)
    {
        // if (level.rockClusters.empty())
        //     return false;

        // std::vector<std::size_t> eligible;
        // eligible.reserve(level.rockClusters.size());
        // for (std::size_t i = 0; i < level.rockClusters.size(); ++i)
        // {
        //     if (level.rockClusters[i].tileIndexes.size() >= 50)
        //         eligible.push_back(i);
        // }

        // if (eligible.empty())
        //     return false;

        std::mt19937 randomEngine(std::random_device{}());
        // std::uniform_int_distribution<std::size_t> clusterPick(0,
        //                                                        eligible.size() - 1);
        // const std::size_t clusterIndex = eligible[clusterPick(randomEngine)];
        // RockCluster &cluster = level.rockClusters[clusterIndex];

        // copy the cluster tile list and shuffle it, so we pick random tiles.
        std::vector<int> sampled = cluster.tileIndexes;
        std::shuffle(sampled.begin(), sampled.end(), randomEngine);

        // replace 40% of the cluster tiles (at least one).
        const int tileReplaceCount = std::max(1,
                                              static_cast<int>(std::lround(sampled.size() * 0.40f)));

        for (int i = 0; i < tileReplaceCount; ++i)
        {
            // flat index -> (x, y) in the row-major tile field.
            const int flatIndex = sampled[i];
            const int x = flatIndex % level.width;
            const int y = flatIndex / level.width;
            level.At(x, y) = tile::TileType::Ore;
        }

        return true;
    }
}

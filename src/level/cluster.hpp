#pragma once

#include <vector>

namespace civsim::level
{
    // forward declaration: cluster helpers only need a Level reference.
    struct Level;
}

// cluster: connected rock components and their lookup helpers.
namespace civsim::level::cluster
{
    // ROCK CLUSTER:

    /**
     * Real connected rock component, stored as the flat row-major tile indexes it owns.
     */
    struct RockCluster
    {
        std::vector<int> tileIndexes;
    };

    /**
     * Find the rock cluster that owns the given flat tile index.
     *
     * @param level current map object
     * @param tileIndex flat row-major tile index to look up
     * @return pointer to the owning cluster, or nullptr when the tile is not in any cluster
     */
    const RockCluster *FindClusterByTile(const Level &level, int tileIndex);

    /**
     * Discover the connected rock components from the level tile field.
     *
     * @param level current map object
     * @return the list of rock clusters found on the map
     */
    std::vector<RockCluster> MakeRockClusters(const Level &level);
}

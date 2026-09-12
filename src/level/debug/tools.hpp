#pragma once

#include "level/cluster.hpp"
#include "level/level.hpp"

// debug: developer-only test helpers.
namespace civsim::level::debug
{
    /**
     * Change a random 40% of the given rock cluster into the new Ore tile type.
     *
     * @param level current map object
     * @param cluster rock cluster to convert
     * @return true when the cluster was converted
     */
    bool Rock2OreClusterPenetratorTEST(Level &level, const cluster::RockCluster &cluster);
}

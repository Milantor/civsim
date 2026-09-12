#pragma once

#include "level/cluster.hpp"
#include "level/level.hpp"
#include "level/seed.hpp"
#include "settings.hpp"

// rocks: rock cluster generation.
namespace civsim::level::gen::rocks
{
    /**
     * Grow rock clusters on the dirt field using the tune values.
     *
     * @param level current map object
     * @param volume fraction of dirt tiles to turn into rock
     * @param spacing minimum gap between rock cluster centers
     * @param density how many rock clusters to try
     * @return true when rocks were grown
     */
    bool GenerateRocks(Level &level,
                       float volume,
                       float spacing,
                       float density);
}

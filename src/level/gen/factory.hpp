#pragma once

#include "level/gen/river.hpp"
#include "level/gen/rocks.hpp"
#include "level/level.hpp"
#include "settings.hpp"

#include <cstdint>
#include <optional>

namespace civsim::level::gen
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

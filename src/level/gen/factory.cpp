#include "level/gen/factory.hpp"

namespace civsim::level::gen
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
        river::GenerateRiver(level,
                             settings::RiverWidth,
                             settings::RiverBankWidth,
                             river::RiverGeneratorMode::RandomEndpoints);
        rocks::GenerateRocks(level,
                             settings::RockVolume,
                             settings::RockSpacing,
                             settings::RockDensity);
        return level;
    }
}

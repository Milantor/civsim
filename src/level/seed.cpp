#include "level/seed.hpp"

#include <cstdint>
#include <random>

namespace civsim::seed
{
    std::uint64_t MakeRandomSeed()
    {
        // pull one value from the system random device.
        std::random_device device;
        std::mt19937_64 engine(device());
        return engine();
    }

    std::uint64_t MixSeed(std::uint64_t baseSeed,
                          std::uint64_t salt)
    {
        // fold the salt into the base seed, then avalanche the bits.
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

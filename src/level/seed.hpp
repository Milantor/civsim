#pragma once

#include <cstdint>

// seed: random seed creation and deterministic seed mixing.
namespace civsim::seed
{
    /**
     * Create a fresh random seed from the system random device.
     *
     * @return a new random 64-bit seed value
     */
    std::uint64_t MakeRandomSeed();

    /**
     * Mix a base seed with a salt into a new deterministic seed.
     *
     * @param baseSeed base seed to derive from
     * @param salt domain salt so different systems get different streams
     * @return a mixed 64-bit seed value
     */
    std::uint64_t MixSeed(std::uint64_t baseSeed,
                          std::uint64_t salt);
}

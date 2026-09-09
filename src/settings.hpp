#pragma once

#include <array>

namespace civsim
{
    // settings: shared tune values for the game.
    namespace settings
    {
        // map size
        inline constexpr int MapWidth = 250;
        inline constexpr int MapHeight = 250;
        inline constexpr int TileSize = 32;

        // river size
        inline constexpr int RiverWidth = 16;
        inline constexpr int RiverBankWidth = 5;

        // river bends
        // route segments: how many little polyline joints the river route uses.
        inline constexpr int RiverRouteSegments = 9;
        // wiggle spread: tiny left/right normal push on each segment during route carving.
        // keep it very low so the river looks smooth instead of jagged.
        inline constexpr int RiverWiggleSpread = 3;

        // rock gen
        inline constexpr float RockVolume = 0.10f;
        inline constexpr float RockSpacing = 0.20f;
        inline constexpr float RockDensity = 0.50f;
        inline constexpr int RockMinCount = 1;
        inline constexpr int RockMaxCount = 40;
        inline constexpr int RockPlaceTries = 100;
        inline constexpr int RockFillNeighborNeed = 5;

        // cam speed
        inline constexpr float CameraMoveSpeed = 500.f;

        // zoom steps
        inline constexpr std::array<float, 5> CameraZoomLevels{0.25f, 0.5f, 1.f, 2.f, 4.f};
        inline constexpr float CameraStartZoom = 1.f;

        // screen set
        inline constexpr int TargetFps = 60;
        inline constexpr int HelpTextX = 20;
        inline constexpr int HelpTextY = 20;
        inline constexpr int HelpTextSize = 20;
        inline constexpr int FpsTextX = 20;
        inline constexpr int FpsTextY = 50;
        inline constexpr int CursorTextX = 20;
        inline constexpr int CursorTextY = 80;
        inline constexpr int CursorTextSize = 20;

        // game text
        inline constexpr const char *WindowTitle = "civsim - procedural level";
        inline constexpr const char *HelpText = "WASD: move | wheel or numpad +/-: zoom | space: new map";
        inline constexpr const char *CursorTextFormat = "tile: %s (%d, %d)";
        inline constexpr const char *CursorOutsideText = "tile: outside map";

        // save files
        inline constexpr const char *GeneratedLevelFile = "generatedlevel.dat";
        inline constexpr const char *GeneratedLevelPng = "generatedlevel.png";

        // tile files
        inline constexpr const char *DirtFile = "/sprites/tiles/dirt.png";
        inline constexpr const char *WaterFile = "/sprites/tiles/water.png";
        inline constexpr const char *SandFile = "/sprites/tiles/sand.png";
        inline constexpr const char *RockFile = "/sprites/tiles/rock.png";
        inline constexpr const char *OreFile = "/sprites/tiles/ore.png";
    }
}

namespace GameSettings = civsim::settings;
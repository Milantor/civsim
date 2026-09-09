#include "main.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

using civsim::DrawLevel;
using civsim::GetTileName;
using civsim::GetTileUnderCursor;
using civsim::Level;
using civsim::SaveLevelAsBinary;
using civsim::SaveLevelAsPng;
using civsim::Rock2OreClusterPenetratorTEST;
using civsim::TileInfos;
using civsim::TileType;

// local aliases for the small namespace tree
namespace settings = civsim::settings;
namespace levelgen = civsim::levelgen;
namespace camera = civsim::camera;

// --------- APP BOOT ---------

int main()
{
    // set up window
    raylib::Window window(GetScreenWidth(), GetScreenHeight(), settings::WindowTitle);

    // set up cam
    raylib::Camera2D gameCamera(raylib::Vector2{GetScreenWidth() / 2.f, GetScreenHeight() / 2.f}, raylib::Vector2{0.f, 0.f});
    gameCamera.zoom = settings::CameraStartZoom;
    window.ToggleBorderless().SetTargetFPS(settings::TargetFps).ToggleFullscreen();
    EnableCursor();

    std::array<raylib::Texture2D, 5> textures;
    for (std::size_t i = 0; i < TileInfos.size(); ++i)
    {
        textures[i] = raylib::Texture2D(std::string(CIVSIM_RESOURCE_DIR) + TileInfos[i].spriteFile);
    }

    Level level = levelgen::GenerateDefaultLevel();

    // main loop
    while (!window.ShouldClose())
    {
        const raylib::Vector2 origin{-level.width * settings::TileSize / 2.f,
                                     -level.height * settings::TileSize / 2.f};
        const auto cursorTile = GetTileUnderCursor(level, gameCamera, origin, settings::TileSize);

        if (IsKeyPressed(KEY_SPACE))
        {
            level = levelgen::GenerateDefaultLevel();
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && cursorTile.has_value() && cursorTile->type == TileType::Rock)
        {
            auto cluster = civsim::findClusterByTile(level, cursorTile->x + cursorTile->y * level.width);
            Rock2OreClusterPenetratorTEST(level, *cluster);
        }

        camera::UpdateCameraMovement(gameCamera,
                                     level.width * settings::TileSize,
                                     level.height * settings::TileSize);
        camera::UpdateCameraZoom(gameCamera);
        UpdateDrawFrame(
            gameCamera,
            level,
            textures);
    }

    // save last map on close
    const std::filesystem::path outDir = std::filesystem::current_path() / "build" / "bin";
    std::filesystem::create_directories(outDir);
    const std::string levelFile = (outDir / settings::GeneratedLevelFile).string();
    const std::string pngFile = (outDir / settings::GeneratedLevelPng).string();

    std::vector<std::uint8_t> levelBytes;
    if (SaveLevelAsBinary(level, levelBytes))
    {
        std::ofstream out(levelFile, std::ios::binary);
        if (out)
        {
            out.write(reinterpret_cast<const char *>(levelBytes.data()),
                      static_cast<std::streamsize>(levelBytes.size()));
        }
    }

    Image pngImage = {0};
    if (SaveLevelAsPng(level,
                       textures,
                       pngImage))
    {
        ExportImage(pngImage, pngFile.c_str());
        UnloadImage(pngImage);
    }

    // shut down
    return 0;
}

// --------- DRAW FRAME ---------

void UpdateDrawFrame(raylib::Camera2D &gameCamera,
                     const Level &level,
                     const std::array<raylib::Texture2D, 5> &textures)
{
    BeginDrawing();
    ClearBackground(BLACK);
    gameCamera.BeginMode();

    const raylib::Vector2 origin{-level.width * settings::TileSize / 2.f,
                                 -level.height * settings::TileSize / 2.f};
    DrawLevel(level, textures, origin);

    gameCamera.EndMode();
    // TODO: ANNIHILATE! maby new super cool gui system (of course that)
    DrawText(settings::HelpText, settings::HelpTextX, settings::HelpTextY, settings::HelpTextSize, RAYWHITE);
    DrawFPS(settings::FpsTextX, settings::FpsTextY);

    const auto cursorTile = GetTileUnderCursor(level, gameCamera, origin, settings::TileSize);
    if (cursorTile.has_value())
    {
        const char *tileText = TextFormat(
            settings::CursorTextFormat,
            GetTileName(cursorTile->type),
            cursorTile->x,
            cursorTile->y);
        DrawText(tileText,
                 settings::CursorTextX,
                 settings::CursorTextY,
                 settings::CursorTextSize,
                 RAYWHITE);
    }
    else
    {
        DrawText(settings::CursorOutsideText,
                 settings::CursorTextX,
                 settings::CursorTextY,
                 settings::CursorTextSize,
                 RAYWHITE);
    }
    EndDrawing();
}
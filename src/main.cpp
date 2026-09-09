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
using civsim::TESTFUNC;
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
    window.ToggleBorderless().SetTargetFPS(settings::TargetFps);
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
        if (IsKeyPressed(KEY_SPACE))
        {
            level = levelgen::GenerateDefaultLevel();
        }
        if (IsKeyPressed(KEY_O))
        {
            TESTFUNC(level);
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

void UpdateDrawFrame(raylib::Camera2D &camera,
                     const Level &level,
                     const std::array<raylib::Texture2D, 5> &textures)
{
    BeginDrawing();
    ClearBackground(BLACK);
    camera.BeginMode();

    const raylib::Vector2 origin{
        -level.width * settings::TileSize / 2.f,
        -level.height * settings::TileSize / 2.f};
    DrawLevel(
        level,
        textures,
        origin);

    camera.EndMode();
    DrawText(settings::HelpText,
             settings::HelpTextX,
             settings::HelpTextY,
             settings::HelpTextSize,
             RAYWHITE);
    DrawFPS(settings::FpsTextX, settings::FpsTextY);

    const auto cursorTile = GetTileUnderCursor(
        level,
        camera,
        origin,
        settings::TileSize);
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

// --------- ECS SAMPLE TILE ---------

void CreateRandomTileEntity(entt::registry &registry,
                            const raylib::Texture2D &tileTexture)
{
    const auto entity = registry.create();
    const int margin = tileTexture.width / 2;
    const int halfScreenWidth = GetScreenWidth() / 2;
    const int halfScreenHeight = GetScreenHeight() / 2;
    registry.emplace<Position>(entity, raylib::Vector2{
                                           static_cast<float>(GetRandomValue(-halfScreenWidth + margin, halfScreenWidth - margin)),
                                           static_cast<float>(GetRandomValue(-halfScreenHeight + margin, halfScreenHeight - margin))});
    registry.emplace<SpriteData>(entity, &tileTexture);
}

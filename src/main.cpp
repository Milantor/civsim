#include "main.hpp"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

using civsim::level::Level;

// local aliases for the small namespace tree (sorted for readability)
namespace camera = civsim::camera;
namespace cluster = civsim::level::cluster;
namespace cursor = civsim::level::cursor;
namespace debug = civsim::level::debug;
namespace io = civsim::level::io;
namespace levelgen = civsim::level::gen;
namespace settings = civsim::settings;
namespace tile = civsim::tile;

// --------- APP BOOT ---------

// builds the tile texture atlas
raylib::Texture2D BuildAtlas(std::vector<tile::TileInfo> &infos);

// creates the map VBO (declaration only for now)
unsigned int BuildTileVBO(const Level &level, const std::vector<tile::TileInfo> &infos);

// draws the map VBO
void DrawTileVBO(unsigned int vboId, int vertexCount, const raylib::Texture2D &atlasTexture);

int main()
{
    // set up window
    raylib::Window window(GetScreenWidth(), GetScreenHeight(), settings::WindowTitle);

    // set up cam
    raylib::Camera2D gameCamera(raylib::Vector2{GetScreenWidth() / 2.f, GetScreenHeight() / 2.f}, raylib::Vector2{0.f, 0.f});
    gameCamera.zoom = settings::CameraStartZoom;
    window.ToggleBorderless().SetTargetFPS(settings::TargetFps).ToggleFullscreen();
    EnableCursor();

    auto atlasTexture = BuildAtlas(tile::TileInfos);

    Level level = levelgen::GenerateDefaultLevel();

    // main loop
    while (!window.ShouldClose())
    {
        const raylib::Vector2 origin{-level.width * settings::TileSize / 2.f,
                                     -level.height * settings::TileSize / 2.f};
        const auto cursorTile = cursor::GetTileUnderCursor(level, gameCamera, origin, settings::TileSize);

        if (IsKeyPressed(KEY_SPACE))
        {
            level = levelgen::GenerateDefaultLevel();
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && cursorTile.has_value() && cursorTile->type == tile::TileType::Rock)
        {
            auto rockCluster = cluster::FindClusterByTile(level, cursorTile->x + cursorTile->y * level.width);
            debug::Rock2OreClusterPenetratorTEST(level, *rockCluster);
        }

        camera::UpdateCameraMovement(gameCamera,
                                     level.width * settings::TileSize,
                                     level.height * settings::TileSize);
        camera::UpdateCameraZoom(gameCamera);
        UpdateDrawFrame(
            gameCamera,
            level,
            atlasTexture);
    }

    // save last map on close
    const std::filesystem::path outDir = std::filesystem::current_path() / "build" / "bin";
    std::filesystem::create_directories(outDir);
    const std::string levelFile = (outDir / settings::GeneratedLevelFile).string();
    const std::string pngFile = (outDir / settings::GeneratedLevelPng).string();

    std::vector<std::uint8_t> levelBytes;
    if (io::SaveLevelAsBinary(level, levelBytes))
    {
        std::ofstream out(levelFile, std::ios::binary);
        if (out)
        {
            out.write(reinterpret_cast<const char *>(levelBytes.data()),
                      static_cast<std::streamsize>(levelBytes.size()));
        }
    }

    raylib::Image pngImage;
    if (io::SaveLevelAsPng(level,
                           atlasTexture,
                           pngImage))
    {
        raylib::ExportImage(pngImage, pngFile);
        //i need it for eye reason
        raylib::Image atlasImage = LoadImageFromTexture(atlasTexture);
        raylib::ExportImage(atlasImage, (outDir / "atlas.png").string());
    }

    // shut down
    return 0;
}

raylib::Texture2D BuildAtlas(std::vector<tile::TileInfo> &infos)
{
    const int tileSize = settings::TileSize;
    const size_t count = infos.size();
    // pick a sensible column count (sqrt of the tile count)
    int cols = (int)std::ceil(std::sqrt(count));
    int rows = (count + cols - 1) / cols;
    int atlasWidth = cols * tileSize;
    int atlasHeight = rows * tileSize;
    Image atlasImage = GenImageColor(atlasWidth, atlasHeight, BLANK);

    for (size_t i = 0; i < count; ++i)
    {
        Image sprite = LoadImage((std::string(CIVSIM_RESOURCE_DIR) + infos[i].sprite.spriteFile).c_str());
        int x = (i % cols) * tileSize;
        int y = (i / cols) * tileSize;
        Rectangle srcRect = {0, 0, (float)tileSize, (float)tileSize};
        Rectangle dstRect = {(float)x, (float)y, (float)tileSize, (float)tileSize};
        ImageDraw(&atlasImage, sprite, srcRect, dstRect, WHITE);
        // write the UVs
        infos[i].sprite.u0 = (float)x / atlasWidth;
        infos[i].sprite.v0 = (float)y / atlasHeight;
        infos[i].sprite.u1 = (float)(x + tileSize) / atlasWidth;
        infos[i].sprite.v1 = (float)(y + tileSize) / atlasHeight;
        UnloadImage(sprite);
    }

    raylib::Texture2D atlasTexture = LoadTextureFromImage(atlasImage);
    UnloadImage(atlasImage);
    return atlasTexture;
}

// builds the map VBO (stub for now)
unsigned int BuildTileVBO(const Level &level, const std::vector<tile::TileInfo> &infos)
{
    // the vertex VBO will be created here
    // ...
    return 0;
}

void DrawTileVBO(unsigned int vboId, int vertexCount, const raylib::Texture2D &atlasTexture)
{
    // draw the map VBO
    // ...
}

// --------- DRAW FRAME ---------

void UpdateDrawFrame(raylib::Camera2D &gameCamera,
                     const Level &level,
                     const raylib::Texture2D &atlasTexture)
{
    BeginDrawing();
    ClearBackground(BLACK);
    gameCamera.BeginMode();

    const raylib::Vector2 origin{-level.width * settings::TileSize / 2.f,
                                 -level.height * settings::TileSize / 2.f};
    io::DrawLevel(level, atlasTexture, origin);

    gameCamera.EndMode();
    // TODO: ANNIHILATE! maby new super cool gui system (of course that)
    DrawText(settings::HelpText, settings::HelpTextX, settings::HelpTextY, settings::HelpTextSize, RAYWHITE);
    DrawFPS(settings::FpsTextX, settings::FpsTextY);

    const auto cursorTile = cursor::GetTileUnderCursor(level, gameCamera, origin, settings::TileSize);
    if (cursorTile.has_value())
    {
        const char *tileText = TextFormat(
            settings::CursorTextFormat,
            tile::GetTileName(cursorTile->type),
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
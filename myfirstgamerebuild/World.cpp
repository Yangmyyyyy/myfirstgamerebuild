#include "World.h"
#include <filesystem>

void World::Load(const char* resourceDirectory)
{
    Unload();
    const std::filesystem::path resourceDir(resourceDirectory);
    forestTexture = LoadTexture((resourceDir / "forest.png").string().c_str());
    Image terrainImage = LoadImage((resourceDir / "terrain.png").string().c_str());
    terrainTexture = terrainImage.data != nullptr ? LoadTextureFromImage(terrainImage) : Texture2D{};
    if (terrainImage.data != nullptr)
    {
        int split = static_cast<int>(terrainImage.height * 0.64f); // 两条素材之间的透明间隙
        for (int row = 0; row < 2; row++)
        {
            int top = row == 0 ? 0 : split;
            int height = row == 0 ? split : terrainImage.height - split;
            Image part = ImageFromImage(terrainImage, { 0, static_cast<float>(top), static_cast<float>(terrainImage.width), static_cast<float>(height) });
            Rectangle bounds = GetImageAlphaBorder(part, 0.1f); // 只取草地本体，排除透明留白
            terrainFrames[row] = { bounds.x, top + bounds.y, bounds.width, bounds.height };
            UnloadImage(part);
        }
    }
    UnloadImage(terrainImage);
    if (terrainTexture.id != 0) SetTextureFilter(terrainTexture, TEXTURE_FILTER_POINT);
}

void World::DrawBackground() const
{
    if (forestTexture.id != 0)
        DrawTexturePro(forestTexture, {24,80,1233,693}, {0,0,static_cast<float>(GameConfig::ScreenWidth),static_cast<float>(GameConfig::ScreenHeight)}, {0,0}, 0, WHITE);
}

void World::DrawPlatforms() const
{
    for (int i = 0; i < static_cast<int>(platforms.size()); i++)
    {
        Rectangle platform = platforms[i]; // 复制仅用于绘制，原来的碰撞矩形不改
        if (terrainTexture.id == 0)
        {
            DrawRectangleRec(platform, RED); // 素材丢失时保留可见的测试平台
            continue;
        }
        Rectangle source = terrainFrames[i == 0 ? 0 : 1];
        if (i == 0)
        {
            // 厚地面重复铺，末块只画剩余部分，避免把岩石拉成长条。
            float tileHeight = platform.height + 8.0f;
            float tileWidth = source.width / source.height * tileHeight;
            for (float x = platform.x; x < platform.x + platform.width; x += tileWidth)
            {
                float width = tileWidth;
                if (x + width > platform.x + platform.width) width = platform.x + platform.width - x;
                Rectangle piece = source;
                piece.width *= width / tileWidth;
                DrawTexturePro(terrainTexture, piece, { x, platform.y - 8, width, tileHeight }, { 0, 0 }, 0, WHITE);
            }
        }
        else
        {
            // 左右端各保留一块，只有中间延展；草叶可超出碰撞体，但脚底仍站在原平台顶面。
            float sourceCap = source.width * 0.1f;
            float cap = 16.0f;
            float y = platform.y - 6.0f;
            float height = platform.height + 10.0f;
            DrawTexturePro(terrainTexture, { source.x, source.y, sourceCap, source.height },
                { platform.x, y, cap, height }, { 0, 0 }, 0, WHITE);
            DrawTexturePro(terrainTexture, { source.x + sourceCap, source.y, source.width - 2 * sourceCap, source.height },
                { platform.x + cap, y, platform.width - 2 * cap, height }, { 0, 0 }, 0, WHITE);
            DrawTexturePro(terrainTexture, { source.x + source.width - sourceCap, source.y, sourceCap, source.height },
                { platform.x + platform.width - cap, y, cap, height }, { 0, 0 }, 0, WHITE);
        }
    }
}

void World::Unload()
{
    if (forestTexture.id != 0) UnloadTexture(forestTexture);
    if (terrainTexture.id != 0) UnloadTexture(terrainTexture);
    forestTexture = {};
    terrainTexture = {};
}

const std::array<Rectangle, 3>& World::GetPlatforms() const { return platforms; }
float World::GetWidth() const { return GameConfig::WorldWidth; }
float World::GetHeight() const { return static_cast<float>(GameConfig::ScreenHeight); }

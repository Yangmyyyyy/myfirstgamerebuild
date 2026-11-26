#include "World.h"
#include <filesystem>
#include <algorithm>

void World::Load(const char* resourceDirectory)
{
    Unload();
    const std::filesystem::path resourceDir(resourceDirectory);
    forestTexture = LoadTexture((resourceDir / "forest.png").string().c_str());
    // 只读取草地与泥土主体，不把生成图外围光晕画进场景。
    terrainTexture = LoadTexture((resourceDir / "terrain_chunky_v1.png").string().c_str());
    terrainFrames[0] = { 55, 272, 1425, 165 };
    terrainFrames[1] = { 110, 714, 1315, 88 };
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
            float tileHeight = platform.height; // 草地顶面直接对齐碰撞面，不再上抬8像素。
            float tileWidth = source.width / source.height * tileHeight;
            for (float x = platform.x; x < platform.x + platform.width; x += tileWidth)
            {
                float width = tileWidth;
                if (x + width > platform.x + platform.width) width = platform.x + platform.width - x;
                Rectangle piece = source;
                piece.width *= width / tileWidth;
                DrawTexturePro(terrainTexture, piece, { x, platform.y, width, tileHeight }, { 0, 0 }, 0, WHITE);
            }
        }
        else
        {
            // 左右端各保留一块，中间等比例重复铺设；草叶可超出碰撞体，但脚底仍站在原平台顶面。
            float y = platform.y; // 悬空平台也使用同一顶面，脚底不会陷入草皮。
            float height = platform.height + 10.0f;
            float scale = height / source.height;
            float sourceCap = source.height * 0.65f;
            float cap = sourceCap * scale;
            DrawTexturePro(terrainTexture, { source.x, source.y, sourceCap, source.height },
                { platform.x, y, cap, height }, { 0, 0 }, 0, WHITE);
            // 中间按同一比例重复铺设，末块裁切；不用横向拉伸像素石块。
            float middleEnd = platform.x + platform.width - cap;
            float repeatWidth = source.height * 2.0f * scale;
            for (float x = platform.x + cap; x < middleEnd; x += repeatWidth)
            {
                float width = std::min(repeatWidth, middleEnd - x);
                DrawTexturePro(terrainTexture, { source.x + sourceCap, source.y, width / scale, source.height },
                    { x, y, width, height }, { 0, 0 }, 0, WHITE);
            }
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

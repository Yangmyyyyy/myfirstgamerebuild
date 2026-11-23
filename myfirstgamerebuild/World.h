#pragma once
#include "raylib.h"
#include "GameConfig.h"
#include <array> // std::array同时保存平台数据和固定数量

class World
{
public:
    World() = default;
    World(const World&) = delete; // 场景纹理不能让多个对象重复释放
    World& operator=(const World&) = delete;
    void Load(const char* resourceDirectory);
    void DrawBackground() const; // 窗口坐标下先画远景
    void DrawPlatforms() const;  // 镜头模式内画平台
    void Unload();
    const std::array<Rectangle, 3>& GetPlatforms() const; // 只读地图，碰撞不能改平台
    float GetWidth() const;
    float GetHeight() const;
private:
    Texture2D forestTexture = {};
    Texture2D terrainTexture = {};
    Rectangle terrainFrames[2] = {};
    std::array<Rectangle, 3> platforms = {{
        { 0, 550, GameConfig::WorldWidth, 50 }, // 地面宽度等于世界宽度
        { 200, 430, 160, 20 },
        { 430, 330, 160, 20 }
    }};
};

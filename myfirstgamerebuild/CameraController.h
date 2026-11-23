#pragma once
#include "raylib.h"
class CameraController
{
public:
    CameraController(); // 创建时设置镜头偏移和缩放
    void Update(const Rectangle& player, float worldWidth);
    Camera2D GetCamera() const;
private:
    Camera2D camera = {};
};

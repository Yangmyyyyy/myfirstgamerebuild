#pragma once
#include "raylib.h"

// 纯碰撞工具：只修正矩形位置，不读按键、不画图、不决定攻击或血量。
namespace Collision
{
    struct VerticalResult
    {
        bool hit = false;    // 是否撞到平台
        bool landed = false; // 是否从上方落到平台上
    };
    bool MoveX(Rectangle& body, float distance, const Rectangle* platforms, int count);
    VerticalResult MoveY(Rectangle& body, float distance, const Rectangle* platforms, int count);
}

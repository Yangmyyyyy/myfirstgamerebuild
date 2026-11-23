#include "Collision.h"

bool Collision::MoveX(Rectangle& body, float distance, const Rectangle* platforms, int count)
{
    body.x += distance; // 先只移动X，再检查矩形是否与平台相交
    for (int i=0; i<count; i++)
    {
        if (CheckCollisionRecs(body, platforms[i]))
        {
            if (distance > 0) body.x = platforms[i].x - body.width;
            else if (distance < 0) body.x = platforms[i].x + platforms[i].width;
            return true; // 本次检查已经完成，返回后不再检查其余平台
        }
    }
    return false;
}

Collision::VerticalResult Collision::MoveY(Rectangle& body, float distance, const Rectangle* platforms, int count)
{
    body.y += distance;
    for (int i=0; i<count; i++)
    {
        if (CheckCollisionRecs(body, platforms[i]))
        {
            if (distance > 0) body.y = platforms[i].y - body.height;
            else if (distance < 0) body.y = platforms[i].y + platforms[i].height;
            return { true, distance > 0 }; // 顶头也算hit，但只有下落才算landed
        }
    }
    return {};
}

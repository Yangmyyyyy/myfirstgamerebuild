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

// 重做Day10复盘：MoveX/MoveY知道检测步骤，但平台首地址和数量必须由调用处传入。
// 用户问landed在哪里判断：distance > 0本身就是比较表达式，正数为true，0/负数为false。
// return {true, distance > 0}按VerticalResult中hit、landed的声明顺序提供两个值。
// 它等价于result.hit=true; result.landed=distance>0; return result;，不是另有隐藏函数。
// 无碰撞return {}使用成员默认值：hit=false、landed=false。

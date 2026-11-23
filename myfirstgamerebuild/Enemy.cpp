#include "Enemy.h"
const Rectangle& Enemy::GetBounds() const { return body; }
int Enemy::GetHealth() const { return health; }
void Enemy::Draw() const
{
    if (health > 0)
    {
        DrawRectangleRec(body, BLUE); // 只有活着的敌人才绘制
    }
}void Enemy::TakeDamage(int damage) // 实现敌人的受伤功能
{
    health -= damage; // 从当前血量中减去传进来的伤害

    if (health < 0)
    {
        health = 0; // 血量最低为0，避免显示负数
    }
}
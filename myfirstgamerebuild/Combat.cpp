#include "Combat.h"
#include "Player.h"
#include "Enemy.h"
#include "GameConfig.h"

void Combat::UpdateTimers(float deltaTime)
{
    if (attackTimeLeft > 0) attackTimeLeft -= deltaTime;
}

void Combat::TryStart(bool pressed, bool playerAlive, float dashTimeLeft, float facingDirection)
{
    if (pressed && playerAlive && dashTimeLeft <= 0 && attackTimeLeft <= 0)
    {
        attackTimeLeft = GameConfig::AttackDuration;
        attackDirection = facingDirection; // 发动瞬间锁方向
    }
}
void Combat::Cancel() // 实现属于Combat类的Cancel函数
{
    attackTimeLeft = 0.0f; // 清零剩余时间，让攻击立即结束
}
bool Combat::IsAttacking() const { return attackTimeLeft > 0; }
float Combat::GetTimeLeft() const { return attackTimeLeft; }

Rectangle Combat::GetAttackBox(const Rectangle& player) const
{
    Rectangle box = { player.x + player.width, player.y, GameConfig::AttackReach, player.height };
    if (attackDirection < 0) box.x = player.x - box.width;
    return box; // 根据最新玩家坐标计算，不在这里修改玩家位置
}

void Combat::DrawAttackBox(const Player& player) const
{
    if (IsAttacking() && !player.IsDead() && player.GetDashTimeLeft() <= 0)
    {
        Rectangle box = GetAttackBox(player.GetBounds());
        DrawRectangleRec(box, Fade(ORANGE, 0.35f));
        DrawRectangleLinesEx(box, 2.0f, ORANGE);
    }
}

void Combat::ResolveContactDamage(Player& player, const Enemy& enemy) const
{
    // 保留原版本接触扣血；敌人真正攻击做好后再移除这个入口。
    if (CheckCollisionRecs(player.GetBounds(), enemy.GetBounds()))
        player.TakeDamage(GameConfig::ContactDamage);
}

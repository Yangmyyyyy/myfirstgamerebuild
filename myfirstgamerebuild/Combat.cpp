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
        hasHitEnemy = false; // 新的一剑开始，重新允许命中一次
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
    if (enemy.GetHealth() > 0 &&
        CheckCollisionRecs(player.GetBounds(), enemy.GetBounds()))
    {
        player.TakeDamage(GameConfig::ContactDamage);
        // 敌人活着，并且身体接触玩家时，才保留当前接触伤害
    }
}
void Combat::ResolvePlayerAttack(const Player& player, Enemy& enemy)
{
    if (!IsAttacking() || hasHitEnemy || player.IsDead() ||
        player.GetDashTimeLeft() > 0 || enemy.GetHealth() <= 0)
    {
        return; // 当前不能造成伤害，直接结束本次函数调用
    }

    Rectangle box = GetAttackBox(player.GetBounds());
    // 根据玩家最新位置和本次锁定方向，计算攻击框

    if (CheckCollisionRecs(box, enemy.GetBounds()))
    {
        enemy.TakeDamage(GameConfig::PlayerAttackDamage);
        // 攻击框碰到敌人，扣除配置中的伤害

        hasHitEnemy = true; // 记住这一剑已经命中，后续帧不再扣血
    }
}

// Day08 日末复盘：每次新攻击重置hasHitEnemy，真正命中后设true，不能每帧清零。
// return;只结束当前void函数，不退出整个游戏；未攻击/已命中/死亡/冲刺时提前返回。
// Cancel清攻击计时；只有成功发动Dash才调用，冷却中按C不会取消；死亡和R也显式清理。
// 不画敌人不等于删除数据，所以接触伤害还要检查敌人HP>0；存活敌人的旧接触伤害暂时保留。
// 待学：前摇抬剑不伤、生效窗口能伤一次、后摇收招不伤；连击/弹反/AI只是设计，尚未实现。

#include "Combat.h"
#include "Player.h"
#include "Enemy.h"
#include "GameConfig.h"
#include <algorithm> // std::clamp限制踏步进度

void Combat::UpdateTimers(float deltaTime)
{
    if (attackTimeLeft > 0)
    {
        attackTimeLeft -= deltaTime; // 正在攻击时，减少剩余时间

        if (attackTimeLeft <= 0)
        {
            if (attackQueued && comboStep < 3)
            {
                comboStep += 1; // 当前段数加1：第一段接第二段，第二段接第三段
                attackTimeLeft = GameConfig::AttackDuration; // 为新的一段重新设置攻击时间
                attackQueued = false; // 这次预约已经用掉，下一次接招需要再次按J
                hasHitEnemy = false; // 新的一段可以重新对敌人造成一次伤害
            }
            else
            {
                Cancel(); // 没有预约，或者第三段已经结束，就清理本轮连击
            }
        }
    }
}

void Combat::TryStart(bool pressed, bool playerAlive, float dashTimeLeft, float facingDirection)
{
    if (pressed && playerAlive && dashTimeLeft <= 0 && attackTimeLeft <= 0)
    {
        attackTimeLeft = GameConfig::AttackDuration;
        attackDirection = facingDirection; // 发动瞬间锁方向
        hasHitEnemy = false; // 新的一剑开始，重新允许命中一次
        comboStep =1;         // 重置攻击段数
        attackQueued = false; // 清除下一段攻击的预约
    }
    else if (pressed && playerAlive && dashTimeLeft <= 0 &&
        IsAttacking() && comboStep < 3)
    {
        attackQueued = true; // 正在攻击时再次按J，记住要接下一段
    }
}
void Combat::Cancel()
{
    attackTimeLeft = 0.0f; // 清除攻击剩余时间
    comboStep = 0;         // 清除当前攻击段数
    attackQueued = false; // 清除下一段攻击的预约
}
bool Combat::IsAttacking() const { return attackTimeLeft > 0; }
float Combat::GetTimeLeft() const { return attackTimeLeft; }
int Combat::GetComboStep() const
{
    return comboStep; // 返回Combat保存的当前攻击段数
}
float Combat::GetAttackMoveX(float deltaTime) const
{
    if (!IsAttacking() || deltaTime <= 0.0f) return 0.0f; // 不攻击/没有经过时间，就没有前踏

    float distance = GameConfig::AttackChopStepDistance;
    if (comboStep == 2) distance = GameConfig::AttackSlashStepDistance;
    else if (comboStep == 3) distance = GameConfig::AttackThrustStepDistance;

    float elapsed = GameConfig::AttackDuration - attackTimeLeft; // 本段攻击已经进行多久
    float duration = GameConfig::AttackStepEnd - GameConfig::AttackStepStart;
    if (duration <= 0.0f) return 0.0f; // 配置区间无效时不移动，避免除以0

    float current = std::clamp((elapsed - GameConfig::AttackStepStart) / duration, 0.0f, 1.0f);
    float previous = std::clamp((elapsed - deltaTime - GameConfig::AttackStepStart) / duration, 0.0f, 1.0f);
    // 平滑曲线：起步和停步速度接近0，中段较快，避免匀速滑着走。
    float currentDistance = current * current * (3.0f - 2.0f * current);
    float previousDistance = previous * previous * (3.0f - 2.0f * previous);
    return attackDirection * distance * (currentDistance - previousDistance); // 只返回本帧增量，不重复加总距离
}

bool Combat::IsAttackActive() const
{
    if (!IsAttacking())
    {
        return false; // 根本没在攻击，当然不能造成攻击伤害
    }

    float elapsed = GameConfig::AttackDuration - attackTimeLeft; // 总时长减剩余时间，得到已经攻击多久

    float activeStart = GameConfig::AttackActiveStart; // 第一段仍用你已经确认的竖劈窗口
    float activeEnd = GameConfig::AttackActiveEnd;
    if (comboStep == 2)
    {
        activeStart = GameConfig::AttackSlashActiveStart; // 第二段匹配横劈出剑
        activeEnd = GameConfig::AttackSlashActiveEnd;
    }
    else if (comboStep == 3)
    {
        activeStart = GameConfig::AttackThrustActiveStart; // 第三段匹配突刺伸剑
        activeEnd = GameConfig::AttackThrustActiveEnd;
    }
    bool reachedStart = elapsed >= activeStart; // 比较产生true/false
    bool beforeEnd = elapsed < activeEnd;

    return reachedStart && beforeEnd; // 两个条件都成立，才返回true
}

Rectangle Combat::GetAttackBox(const Rectangle& player) const
{
    Rectangle box = { player.x + player.width, player.y, GameConfig::AttackReach, player.height };
    if (attackDirection < 0) box.x = player.x - box.width;
    return box; // 根据最新玩家坐标计算，不在这里修改玩家位置
}

void Combat::DrawAttackBox(const Player& player) const
{
    if (IsAttackActive() && !player.IsDead() && player.GetDashTimeLeft() <= 0)
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
    if (!IsAttackActive() || hasHitEnemy || player.IsDead() ||
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

#pragma once
#include "raylib.h"
class Player; // 前置声明：这里只需要知道有这个类型，避免头文件互相包含
class Enemy;

// 保存攻击计时、方向和每剑一次的命中结算；前摇/后摇与弹反尚待实现。
class Combat
{
public:
    void UpdateTimers(float deltaTime);
    void TryStart(bool pressed, bool playerAlive, float dashTimeLeft, float facingDirection);
    void Cancel(); // 声明一个取消攻击的功能，不返回结果
    bool IsAttacking() const;
    float GetTimeLeft() const;
    Rectangle GetAttackBox(const Rectangle& player) const;
    void DrawAttackBox(const Player& player) const;
    void ResolveContactDamage(Player& player, const Enemy& enemy) const;
    void ResolvePlayerAttack(const Player& player, Enemy& enemy);
    // 读取玩家状态，检查攻击是否命中，并修改敌人血量
private:
    float attackTimeLeft = 0.0f;
    float attackDirection = 1.0f;
    bool hasHitEnemy = false;
};

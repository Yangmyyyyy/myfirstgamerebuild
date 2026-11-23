#pragma once
#include "raylib.h"
class Player; // 前置声明：这里只需要知道有这个类型，避免头文件互相包含
class Enemy;

// 目前只保存玩家攻击框；伤害、前摇/后摇与弹反将在后续课程实现。
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
private:
    float attackTimeLeft = 0.0f;
    float attackDirection = 1.0f;
};

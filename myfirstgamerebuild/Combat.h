#pragma once
#include "raylib.h"
class Player; // 前置声明：这里只需要知道有这个类型，避免头文件互相包含
class Enemy;

// 保存三连击计时、方向、预约和每段一次命中；已有生效窗口，弹反尚未实现。
class Combat
{
public:
    void UpdateTimers(float deltaTime);
    void TryStart(bool pressed, bool playerAlive, float dashTimeLeft, float facingDirection);
    void Cancel(); // 声明一个取消攻击的功能，不返回结果
    bool IsAttacking() const;
    bool IsAttackActive() const; // 声明：判断攻击是否处于伤害生效阶段，返回true或false
    float GetTimeLeft() const;
    float GetAttackMoveX(float deltaTime) const; // 查询本帧攻击前踏位移，沿发动时锁定的方向
    int GetComboStep() const; // 声明：返回当前攻击段数，查询时不修改状态
    Rectangle GetAttackBox(const Rectangle& player) const;
    void DrawAttackBox(const Player& player) const;
    void ResolveContactDamage(Player& player, const Enemy& enemy) const;
    void ResolvePlayerAttack(const Player& player, Enemy& enemy);
    // 读取玩家状态，检查攻击是否命中，并修改敌人血量
private:
    float attackTimeLeft = 0.0f;
    float attackDirection = 1.0f;
    bool hasHitEnemy = false;
    int comboStep = 0;        // 当前攻击段数：0表示没有连击，1第一刀、2横劈、3下蹲横扫
    bool attackQueued = false; // 是否已经预约下一段攻击：初始没有预约
};

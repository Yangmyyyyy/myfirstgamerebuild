#pragma once
#include "raylib.h"
#include "GameConfig.h"
class World; // 前置声明：告诉编译器，World是一个类

// Day09：已有左右巡逻和原地等待；重力/平台碰撞、索敌追击及主动攻击尚未实现。
class Enemy
{
public:
    const Rectangle& GetBounds() const;
    int GetHealth() const;
    void TakeDamage(int damage); // 接收本次伤害数值，修改敌人的当前血量
    void Draw() const;
    void Update(float deltaTime, const World& world); // 接收帧时间和地图引用
private:
    Rectangle body = { 250, 350, 40, 40 };
    int health = GameConfig::EnemyMaxHealth;
    float facingDirection = 1.0f;  // 敌人面向：1向右，-1向左
    float patrolTimeLeft = GameConfig::EnemyPatrolDuration; // 出生后先行走2秒
    bool isPatrolWaiting = false; // false表示正在走，true表示原地等待
};

#pragma once
#include "raylib.h"
#include "GameConfig.h"

// 当前敌人是静态蓝块：有血量、受伤与零血隐藏，AI和主动攻击尚未实现。
class Enemy
{
public:
    const Rectangle& GetBounds() const;
    int GetHealth() const;
    void TakeDamage(int damage); // 接收本次伤害数值，修改敌人的当前血量
    void Draw() const;
private:
    Rectangle body = { 250, 350, 40, 40 };
    int health = GameConfig::EnemyMaxHealth;
};

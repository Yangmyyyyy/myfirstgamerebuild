#pragma once
#include "raylib.h"
#include "GameConfig.h"

// 当前敌人仍是静态蓝色方块，暂不添加AI、敌人攻击或新的死亡规则。
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

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
    void Update(float deltaTime, const World& world,
        const Rectangle& playerBounds); // 读取玩家碰撞矩形，不能通过这里修改玩家
private:
    Rectangle body = { 250, 350, 40, 40 };
    int health = GameConfig::EnemyMaxHealth;
    float facingDirection = 1.0f;  // 敌人面向：1向右，-1向左
    float patrolTimeLeft = GameConfig::EnemyPatrolDuration; // 出生后先行走2秒
    bool isPatrolWaiting = false; // false表示正在走，true表示原地等待
    float velocityY = 0.0f; // 敌人的纵向速度，单位像素/秒；初始没有向上或向下的速度
    bool canSeePlayer = false; // 保存索敌结果，让Update更新、Draw读取；初始未发现玩家
};

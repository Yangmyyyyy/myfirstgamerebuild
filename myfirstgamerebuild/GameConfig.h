#pragma once
// 会经常调手感的数值集中在这里。constexpr表示编译期常量，不是每帧变化的状态。
namespace GameConfig
{
    constexpr int ScreenWidth = 1000;
    constexpr int ScreenHeight = 600;
    constexpr float WorldWidth = 3000.0f;
    constexpr float PlayerWidth = 40.0f;
    constexpr float PlayerHeight = 40.0f;
    constexpr int PlayerMaxHealth = 100;
    constexpr int EnemyMaxHealth = 100;
    constexpr float WalkSpeed = 200.0f;
    constexpr float RunSpeed = 300.0f;
    constexpr float Gravity = 900.0f;
    constexpr float JumpSpeed = 400.0f;
    constexpr int AirJumps = 1;
    constexpr float CoyoteTime = 0.1f;
    constexpr float DashSpeed = 600.0f;
    constexpr float DashDuration = 0.4f;
    constexpr float DashCooldown = 0.8f;
    constexpr float DashInvincibleTime = 0.4f;
    constexpr float AttackDuration = 0.2f;
    constexpr float AttackReach = 60.0f;
    constexpr int PlayerAttackDamage = 20; // 玩家每次普通攻击造成20点伤害
    constexpr int ContactDamage = 10;
    constexpr float DamageCooldown = 1.0f;
}

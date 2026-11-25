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
    constexpr float EnemyPatrolDuration = 2.0f;     // 每段巡逻行走2秒
    constexpr float EnemyPatrolWaitDuration = 1.5f; // 每段行走结束后，原地等待1.5秒
    constexpr float WalkSpeed = 200.0f;
    constexpr float RunSpeed = 300.0f;
    constexpr float WalkAnimationCycle = 0.72f; // 行走左右脚完成一轮的秒数，只影响外观
    constexpr float RunAnimationCycle = 0.50f; // 跑步一轮更快，仍保留12帧
    constexpr float Gravity = 900.0f;
    constexpr float JumpSpeed = 400.0f;
    constexpr int AirJumps = 1;
    constexpr float CoyoteTime = 0.1f;
    constexpr float DashSpeed = 600.0f;
    constexpr float DashDuration = 0.4f;
    constexpr float DashCooldown = 0.8f;
    constexpr float DashInvincibleTime = 0.4f;
    constexpr float AttackDuration = 0.44f; // 保留你调的总时长；8帧不再平均分配时间
    constexpr float AttackChopFrameWeights[8] = {
        0.22f, 0.14f, 0.07f, 0.06f, 0.06f, 0.13f, 0.14f, 0.18f
    }; // 每帧占总时长的比例，合计1；举剑留时间，挥剑快，收招稍慢
    constexpr float AttackReach = 60.0f;
    constexpr int PlayerAttackDamage = 20; // 玩家每次普通攻击造成20点伤害
    constexpr int ContactDamage = 10;
    constexpr float DamageCooldown = 1.0f;
}

#pragma once
// 会经常调手感的数值集中在这里。constexpr表示编译期常量，不是每帧变化的状态。
namespace GameConfig
{
    constexpr float CameraZoom = 1.2f; // 镜头放大1.2倍，方便观察角色动作
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
    constexpr float AttackChopFrameWeights[] = { 0.43f, 0.12f, 0.20f, 0.25f }; // 新素材按真实帧数分配时间，合计1
    constexpr float AttackActiveStart = AttackDuration * 0.43f; // 攻击进行到总时长的43%时，开始允许伤害
    constexpr float AttackActiveEnd = AttackDuration * 0.55f;   // 进行到55%时，结束伤害生效阶段
    constexpr float AttackSlashFrameWeights[] = { 0.24f, 0.18f, 0.18f, 0.12f, 0.12f, 0.16f }; // 新素材按真实帧数分配时间，合计1
    constexpr float AttackThrustFrameWeights[] = { 0.28f, 0.38f, 0.17f, 0.17f }; // 新素材按真实帧数分配时间，合计1
    constexpr float AttackSlashActiveStart = AttackDuration * 0.24f; // 前两帧结束，剑开始横向挥出
    constexpr float AttackSlashActiveEnd = AttackDuration * 0.60f; // 第五张结束，横劈收招前关闭伤害
    constexpr float AttackThrustActiveStart = AttackDuration * 0.28f; // 前两帧准备结束
    constexpr float AttackThrustActiveEnd = AttackDuration * 0.66f; // 第五张刺出结束，之后不再造成伤害
    constexpr float AttackChopStepDistance = 12.0f; // 第一段前踏总距离，像素
    constexpr float AttackSlashStepDistance = 18.0f; // 第二段横劈向前带一步
    constexpr float AttackThrustStepDistance = 28.0f; // 第三段突刺前踏稍远
    constexpr float AttackStepStart = AttackDuration * 0.18f; // 准备动作后开始踏步
    constexpr float AttackStepEnd = AttackDuration * 0.66f; // 挥动结束附近停止，不在收招时持续滑行
    constexpr float AttackReach = 60.0f;
    constexpr int PlayerAttackDamage = 20; // 玩家每次普通攻击造成20点伤害
    constexpr int ContactDamage = 10;
    constexpr float DamageCooldown = 1.0f;
}

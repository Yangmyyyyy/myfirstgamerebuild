#pragma once
#include "raylib.h"
#include "GameConfig.h"
#include "PlayerInput.h"
#include "PlayerVisual.h"
class World;
class Combat;
class Enemy;

// 保存玩法状态：输入响应、重力、跳跃、冲刺、受伤和复位。动画交给PlayerVisual。
class Player
{
public:
    void Update(float deltaTime, const PlayerInput& input, const World& world, Combat& combat, const Enemy& enemy);
    void Reset(); // 恢复当前版本已有的玩家状态；不会重新加载图片
    void TakeDamage(int amount);
    const Rectangle& GetBounds() const;
    int GetHealth() const;
    bool IsDead() const;
    bool IsGrounded() const;
    float GetDashTimeLeft() const;
    float GetVelocityY() const;
    float GetFacingDirection() const;
    PlayerVisualState GetVisualState(bool running) const;
private:
    Rectangle player = { (GameConfig::ScreenWidth - GameConfig::PlayerWidth) / 2,
        (GameConfig::ScreenHeight - GameConfig::PlayerHeight) / 2,
        GameConfig::PlayerWidth, GameConfig::PlayerHeight };
    int playerHealth = GameConfig::PlayerMaxHealth;
    float facingDirection = 1.0f;
    float velocityY = 0.0f;
    bool isGrounded = false;
    int airJumpsLeft = GameConfig::AirJumps;
    float coyoteTimeLeft = 0.0f;
    float damageCooldown = 0.0f;
    float dashTimeLeft = 0.0f;
    float dashCooldown = 0.0f;
    float dashDirection = 1.0f;
    float dashInvincibleTime = 0.0f;
    float directionX = 0.0f; // 本帧输入结果，用于动画判断
    float frameStartX = 0.0f;
};

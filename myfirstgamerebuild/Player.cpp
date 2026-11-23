#include "Player.h"
#include "World.h"
#include "Collision.h"
#include "Combat.h"
#include "Enemy.h"

void Player::Update(float deltaTime, const PlayerInput& input, const World& world, Combat& combat, const Enemy& enemy)
{
    const auto& platforms = world.GetPlatforms(); // 引用地图，不复制所有平台
    float playercurSpeed = GameConfig::WalkSpeed;
    frameStartX = player.x; // 用实际位移区分行走和推墙，不只看有没有按键
    // 方向每帧从零开始。左右两个独立 if 可以同时成立，屏幕 x 向右增大，y 向下增大。
        if (playerHealth <= 0 && input.restartPressed) Reset();
    if (isGrounded)
    {
        coyoteTimeLeft = GameConfig::CoyoteTime; // 站在地面上，持续准备好 0.1 秒宽限
    }
    else if (coyoteTimeLeft > 0)
    {
        coyoteTimeLeft -= deltaTime; // 已经离地，逐帧消耗宽限时间
    }
    if (damageCooldown > 0)
    {
        damageCooldown -= deltaTime; // 每帧减少冷却时间
    }
    directionX = 0.0f;

    if (input.running)
        playercurSpeed = GameConfig::RunSpeed;
    else
        playercurSpeed = GameConfig::WalkSpeed;
    if (playerHealth > 0)
    {
        if (input.jumpPressed &&
            (isGrounded || coyoteTimeLeft > 0 || airJumpsLeft > 0))
        {
            if (!isGrounded && coyoteTimeLeft <= 0)
            {
                airJumpsLeft -= 1; // 已离地且宽限结束，才消耗空中跳跃次数
            }

            velocityY = -GameConfig::JumpSpeed;  // 设置起跳速度
            isGrounded = false;   // 起跳后进入空中状态
            coyoteTimeLeft = 0.0f; // 本次起跳用完地面机会，避免再次免费跳
        }
        if (input.jumpReleased && velocityY < 0)
        {
            velocityY *= 0.5f; // 上升中松开空格，把剩余向上速度减半
        }
        if (input.left)
            directionX -= 1.0f;
        if (input.right)
            directionX += 1.0f;
        if (directionX != 0)
        {
            facingDirection = directionX; // 有左右输入才更新，松手后保留最后朝向
        }
    }
    if (dashTimeLeft > 0)
    {
        dashTimeLeft -= deltaTime;
    }
    if (dashCooldown > 0)
    {
        dashCooldown -= deltaTime;
    }
    if (dashInvincibleTime > 0)
    {
        dashInvincibleTime -= deltaTime;
    }
    if  (dashInvincibleTime <= 0)
        dashInvincibleTime = 0.0f; // 避免负数，方便后续判断
        combat.TryStart(input.attackPressed, playerHealth > 0, dashTimeLeft, facingDirection);
    if (input.dashPressed && dashCooldown <= 0 && playerHealth > 0)
    {
        dashTimeLeft = GameConfig::DashDuration; // 设置冲刺持续时间
        dashCooldown = GameConfig::DashCooldown; // 设置冲刺冷却时间
        dashInvincibleTime = GameConfig::DashInvincibleTime; // 设置冲刺无敌时间
        dashDirection = facingDirection; // 发动时复制当前面向，锁定本次冲刺方向
    }
    // 每轴先尝试移动，再将矩形贴到挡路平台的表面。
    float moveX = directionX * playercurSpeed * deltaTime; // 默认使用普通横移

    if (dashTimeLeft > 0 && playerHealth > 0)
    {
        moveX = dashDirection * GameConfig::DashSpeed * deltaTime; // 冲刺时改用锁定方向的位移
    }

    bool hitWallOnX = Collision::MoveX(player, moveX, platforms.data(), static_cast<int>(platforms.size()));
    if (hitWallOnX && moveX != 0) dashTimeLeft = 0.0f; // 碰撞工具报告结果，玩法决定停止冲刺
    velocityY += GameConfig::Gravity * deltaTime; // 重力先改变纵向速度
    Collision::VerticalResult vertical = Collision::MoveY(player, velocityY * deltaTime,
        platforms.data(), static_cast<int>(platforms.size()));
    isGrounded = vertical.landed;
    if (vertical.hit) velocityY = 0.0f;
    if (isGrounded) airJumpsLeft = GameConfig::AirJumps;
    // 矩形用左上角定位：右边是 x+width，脚底是 y+height。
    if (player.x < 0)
        player.x = 0;
    if (player.x > world.GetWidth() - player.width)
        player.x = world.GetWidth() - player.width; // 限制在关卡右端
    if (player.y < 0)
    {
        player.y = 0;
        if (velocityY < 0)
            velocityY = 0.0f; // 顶到窗口顶部，停止上升
    }
    if (player.y >= GameConfig::ScreenHeight - player.height)
    {
        player.y = GameConfig::ScreenHeight - player.height;
        velocityY = 0.0f; // 底部边界也必须同时修正速度
        isGrounded = true;
        airJumpsLeft = GameConfig::AirJumps; // 剩余空中跳跃次数，落地后恢复为 1
    }
        combat.ResolveContactDamage(*this, enemy); // 先移动和碰撞，再检查接触伤害
    if (playerHealth <= 0 )
        playerHealth = 0;
    bool isDead = (playerHealth <= 0);
    if (isDead)
    {
        dashTimeLeft = 0.0f; // 这一帧扣血后若已死亡，结束剩余冲刺
        dashInvincibleTime = 0.0f; // 同时结束冲刺无敌
    }
}

void Player::Reset()
{
    playerHealth = GameConfig::PlayerMaxHealth; // 恢复已有复位规则，不重新加载图片
    player.x = (GameConfig::ScreenWidth - player.width) / 2.0f;
    player.y = (GameConfig::ScreenHeight - player.height) / 2.0f;
    isGrounded = false;
    damageCooldown = 0.0f;
    velocityY = 0.0f;
    dashTimeLeft = 0.0f;
    dashCooldown = 0.0f;
    dashInvincibleTime = 0.0f;
    airJumpsLeft = GameConfig::AirJumps;
    coyoteTimeLeft = 0.0f;
}

void Player::TakeDamage(int amount)
{
    if (damageCooldown <= 0 && dashInvincibleTime == 0)
    {
        playerHealth -= amount;
        if (playerHealth < 0) playerHealth = 0;
        damageCooldown = GameConfig::DamageCooldown;
    }
}

const Rectangle& Player::GetBounds() const { return player; }
int Player::GetHealth() const { return playerHealth; }
bool Player::IsDead() const { return playerHealth <= 0; }
bool Player::IsGrounded() const { return isGrounded; }
float Player::GetDashTimeLeft() const { return dashTimeLeft; }
float Player::GetVelocityY() const { return velocityY; }
float Player::GetFacingDirection() const { return facingDirection; }

PlayerVisualState Player::GetVisualState(bool running) const
{
    PlayerVisualState state;
    state.dead = IsDead();
    state.grounded = isGrounded;
    state.moving = directionX != 0 && player.x != frameStartX;
    state.running = running;
    state.velocityY = velocityY;
    state.facingDirection = facingDirection;
    state.dashDirection = dashDirection;
    state.dashTimeLeft = dashTimeLeft;
    state.dashInvincibleTime = dashInvincibleTime;
    state.damageCooldown = damageCooldown;
    return state; // 只给外观一份快照，它不能通过这份数据改玩家
}

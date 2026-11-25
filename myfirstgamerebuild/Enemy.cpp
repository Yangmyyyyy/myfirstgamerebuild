#include "Enemy.h"
#include "Collision.h" // 使用已经写好的平台碰撞功能
#include "World.h" // 引入完整定义，为后续读取平台做准备

const Rectangle& Enemy::GetBounds() const { return body; }
int Enemy::GetHealth() const { return health; }
void Enemy::Draw() const
{
    if (health > 0)
    {
        DrawRectangleRec(body, BLUE); // 只有活着的敌人才绘制
    }
}
void Enemy::TakeDamage(int damage) // 实现敌人的受伤功能
{
    health -= damage; // 从当前血量中减去传进来的伤害

    if (health < 0)
    {
        health = 0; // 血量最低为0，避免显示负数
    }
}
void Enemy::Update(float deltaTime, const World& world,
    const Rectangle& playerBounds) // 与头文件声明的参数保持一致
{
    if (health <= 0)
    {
        return; // 敌人死亡后，结束本次更新，不再执行下面的巡逻逻辑
    }

    const auto& platforms = world.GetPlatforms(); // 引用地图已有的平台数组，只读取，不复制
    float enemyCenterX = body.x + body.width / 2.0f; // 敌人左边坐标加一半宽度，得到中心横坐标
    float playerCenterX = playerBounds.x + playerBounds.width / 2.0f; // 玩家中心横坐标
    float distanceX = playerCenterX - enemyCenterX; // 玩家中心减敌人中心，保留左右方向
    patrolTimeLeft -= deltaTime; // 每帧减去实际经过的时间

    if (patrolTimeLeft <= 0.0f)
    {
        if (isPatrolWaiting)
        {
            isPatrolWaiting = false;         // 等待结束，恢复行走
            facingDirection = -facingDirection; // 开始下一段巡逻时，反转方向
        }
        else
        {
            isPatrolWaiting = true; // 行走结束，进入原地等待
        }

        patrolTimeLeft = isPatrolWaiting
            ? GameConfig::EnemyPatrolWaitDuration // true：刚进入等待，设置1.5秒
            : GameConfig::EnemyPatrolDuration;    // false：刚恢复行走，设置2秒
    }
    float forwardDistance = distanceX * facingDirection; // 把横向间距转换为相对朝向的间距：前方为正，后方为负
    bool isPlayerInFront = forwardDistance > 0.0f; // 大于0得到true，表示玩家中心在敌人面前
    const float detectionRange = 300.0f; // 索敌的水平距离，单位像素
    bool isPlayerInRange = isPlayerInFront && forwardDistance <= detectionRange; // 玩家在前方，并且距离不超过300像素
    float enemyCenterY = body.y + body.height / 2.0f; // 敌人中心纵坐标
    float playerCenterY = playerBounds.y + playerBounds.height / 2.0f; // 玩家中心纵坐标
    float distanceY = playerCenterY - enemyCenterY; // 正数表示玩家在下方，负数表示在上方

    const float detectionHeight = 100.0f; // 允许玩家中心在敌人中心上下100像素以内
    bool isPlayerNearHeight =
        distanceY >= -detectionHeight && distanceY <= detectionHeight; // 高度差必须在-100到100之间

    canSeePlayer = isPlayerInRange && isPlayerNearHeight; // 更新头文件里声明的成员变量
    if (!isPatrolWaiting)
    {
        Collision::MoveX(
            body,                              // 要移动的敌人，函数会直接修改它的位置
            facingDirection * 60.0f * deltaTime, // 本帧横向位移：正数向右，负数向左
            platforms.data(),                  // 从这里读取平台数组
            static_cast<int>(platforms.size())  // 需要检查的平台数量
        );
    }

    velocityY += GameConfig::Gravity * deltaTime; // 重力让向下的速度逐渐增加

    Collision::VerticalResult vertical = Collision::MoveY(
        body,                              // 要移动并修正位置的敌人矩形
        velocityY * deltaTime,             // 本帧纵向位移，单位为像素
        platforms.data(),                  // 平台数组的首地址，供函数逐个读取
        static_cast<int>(platforms.size())  // 平台数量，转换成函数要求的int
    );

    if (vertical.hit)
    {
        velocityY = 0.0f; // 碰到平台后停止纵向运动，避免速度持续累积
    }
}

// Day09复盘（2026-11-24，本段是学习注释，不增加玩法）：
// 1. facingDirection记住朝向：1向右，-1向左；取负实现换向，不是修改函数名称。
// 2. patrolTimeLeft和isPatrolWaiting是对象成员，跨帧保存，不能每帧重新初始化。
// 3. 先用deltaTime递减计时，再切换阶段，最后根据新状态决定本帧是否移动。
// 4. 行走2秒后等待1.5秒；等待结束才转向。等待期间只停止位移，计时仍继续。
// 5. 条件 ? 真值 : 假值按切换后的等待状态选择时长；初始值也要设为行走2秒。
// 6. 死亡后的return只结束本次Update调用，不结束游戏主循环。
// 7. Enemy.h声明接口，Enemy::Update定义行为，main的enemy.Update(...)每帧调用。
// 8. class World;仅声明类型名，不是完整类定义，也不创建新的地图对象。
// 9. const World& world引用调用处已有地图；const表示这里不能通过它修改地图。
// 10. #include "World.h"提供完整定义；要使用GetPlatforms等成员时需要它。
// 11. 声明/定义/调用的参数要对应。增加World前置声明不会自动增加Update参数。
// 12. //后直到换行都属于注释，不能把结束用的}接在注释后面。
// 当前world参数只接入，还没读取平台；横移60像素/秒仍为临时测试值。
// 下次：读取地图平台，复用Collision加重力和分轴碰撞，再做前向索敌与追击。

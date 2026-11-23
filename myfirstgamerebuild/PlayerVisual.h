#pragma once // 防止头文件在一个编译单元内重复定义
#include "raylib.h"

// 主循环提供的“外观快照”：只告诉动画现在发生了什么，不负责修改玩法。
struct PlayerVisualState
{
    bool dead = false;           // 玩家是否死亡
    bool grounded = false;       // 脚下是否有支撑
    bool moving = false;         // 有移动输入，并且这一帧真的产生了横移
    bool running = false;        // 是否按住加速键
    float velocityY = 0.0f;      // 判断空中是在上升还是下落
    float facingDirection = 1.0f;
    float dashDirection = 1.0f;  // 冲刺发动时锁定的方向
    float dashTimeLeft = 0.0f;
    float dashInvincibleTime = 0.0f;
    float damageCooldown = 0.0f;
};

// 只管理骑士的外观。血量、输入规则、移动、碰撞和攻击仍由main.cpp处理。
class PlayerVisual
{
public: // 外部使用这些函数；main不需要知道内部怎样截帧和选图
    PlayerVisual() = default; // 使用下面各成员的默认初始值创建对象
    PlayerVisual(const PlayerVisual&) = delete; // 纹理由这个对象独有，避免复制后重复释放
    PlayerVisual& operator=(const PlayerVisual&) = delete;

    void Load(const char* resourceDirectory); // InitWindow之后调用一次，加载图片
    void Update(float deltaTime, const PlayerVisualState& state); // 物理更新后，按最新状态推进动画
    void Draw(const Rectangle& player) const; // 镜头模式内绘制，只读取玩家位置
    void Unload(); // CloseWindow之前释放；清空句柄，重复调用也不会再次释放
    const char* GetMotionName() const; // F1调试文字使用
    int GetFrame() const; // 返回0～5的当前帧编号

private: // 数据归对象自己管理，类外通过函数使用它们
    enum class Motion { Idle, Walk, Run, Dash, Rise, Fall, Dead };
    Texture2D knightTexture = {};
    Texture2D runTexture = {};
    Rectangle animationFrames[4][6] = {};
    Rectangle runFrames[6] = {};
    float spriteScale = 1.0f;
    float runScale = 1.0f;
    float animationTime = 0.0f; // 原来main里的计时，现在属于这个对象
    Motion motion = Motion::Idle;
    Motion previousAnimation = Motion::Idle;
    int animation = 0; // 当前选择的图集行，Update写入、Draw读取
    int frame = 0;
    bool grounded = false;
    float lookDirection = 1.0f;
    Color tint = WHITE;
};

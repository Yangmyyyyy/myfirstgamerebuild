#include "raylib.h"
#include "GameConfig.h"
#include "World.h"
#include "Player.h"
#include "PlayerInput.h"
#include "Enemy.h"
#include "Combat.h"
#include "PlayerVisual.h"
#include "CameraController.h"
#include "Hud.h"
#include <filesystem>

// Day08：main只安排顺序。找具体规则时，进入对应模块的.cpp，不需要翻一个长函数。
int main()
{
    InitWindow(GameConfig::ScreenWidth, GameConfig::ScreenHeight, "My First Game");
    SetTargetFPS(60);
    namespace fs = std::filesystem;
    fs::path resourceDir = "resources";
    if (!fs::exists(resourceDir / "hero" / "hero_idle_8.png"))
        resourceDir = fs::path(GetApplicationDirectory()) / "resources";
    World world;
    Player player;
    Enemy enemy;
    Combat combat;
    PlayerVisual playerVisual;
    CameraController camera;
    world.Load(resourceDir.string().c_str()); // 窗口创建后才加载纹理
    playerVisual.Load(resourceDir.string().c_str());
    bool showCollision = false;

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();
        PlayerInput input = ReadPlayerInput(); // 先读这一帧的输入
        combat.UpdateTimers(deltaTime); // 更新已有攻击倒计时，再允许启动下一次攻击
        enemy.Update(deltaTime, world, player.GetBounds()); // 取出玩家碰撞矩形，传给敌人
        player.Update(deltaTime, input, world, combat, enemy); // 输入、物理、接触伤害
        combat.ResolvePlayerAttack(player, enemy);
        // 玩家移动和状态处理完成后，再结算攻击命中
        camera.Update(player.GetBounds(), world.GetWidth()); // 使用玩家处理后的最新位置
        PlayerVisualState visualState = player.GetVisualState(input.running); // 先取得移动、跳跃等已有外观信息
        visualState.attacking = combat.IsAttacking(); // 把当前是否攻击填进外观信息
        visualState.attackTimeLeft = combat.GetTimeLeft(); // 把攻击剩余时间也填进去
        playerVisual.Update(deltaTime, visualState); // 将补充完整的信息交给动画模块

        BeginDrawing();
        ClearBackground(SKYBLUE);
        world.DrawBackground(); // 固定远景在镜头外
        BeginMode2D(camera.GetCamera());
        world.DrawPlatforms();
        enemy.Draw();
        combat.DrawAttackBox(player);
        playerVisual.Draw(player.GetBounds());
        if (input.debugPressed) showCollision = !showCollision;
        if (showCollision) DrawRectangleLinesEx(player.GetBounds(), 1.0f, LIME);
        EndMode2D();
        DrawHud(player, enemy, combat, playerVisual, showCollision); // 界面固定在窗口上
        EndDrawing();
    }

    playerVisual.Unload(); // 在CloseWindow之前释放GPU资源
    world.Unload();
    CloseWindow();
    return 0;
}

// Day08 日末复盘（2026-11-23）：素材接入与模块拆分由助手代做，攻击逻辑由你亲自完成。
// 先读输入，再更新攻击时间和玩家物理，最后ResolvePlayerAttack结算；绘制不负责扣血。
// class是类型，player/combat是具体对象；对象.函数()调用功能，类名::函数()是.cpp中的实现。
// .h声明接口，.cpp写实现；public供外部调用，private保存模块自己管理的数据。
// main里的GetVisualState提供快照，Draw(player.GetBounds())把选好的图片连接到玩家位置。
// 明天复习：先看本文件流程，再看Combat的TryStart/Cancel/ResolvePlayerAttack，不必一次读完所有类。
// 前摇/生效/后摇今天只介绍了概念，当前仍是0.2秒攻击；练习题未回答，不记为已掌握。

// Day09日末：用户亲写敌人Update、走2秒/停1.5秒、换向和地图引用传参。
// enemy.Update位于player.Update前，使旧接触伤害使用更新后的敌人位置。
// 外观包、slim_v2体型修订、锚点、色板Shader由助手代做，仍需逐步学习。
// 暂存的攻击动画尚未启用；敌人重力/碰撞/索敌/主动攻击也还没实现。

// 重做Day10复盘：Enemy::Update新增第三参数player.GetBounds()，提供玩家只读矩形。
// 玩家本帧处理完发动/取消攻击后，把Combat状态填入visualState，再交给动画模块。
// 用户接入第一段竖劈；素材和后续帧节奏/锚点修正由助手代做，仍待学习。
// 首段总时长0.44秒且整段可命中一次；三连击与真正攻击生效窗口尚未实现。

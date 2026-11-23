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
    if (!fs::exists(resourceDir / "knight_motion_v2.png"))
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
        player.Update(deltaTime, input, world, combat, enemy); // 输入、物理、接触伤害
        combat.ResolvePlayerAttack(player, enemy);
        // 玩家移动和状态处理完成后，再结算攻击命中
        camera.Update(player.GetBounds(), world.GetWidth()); // 使用玩家处理后的最新位置
        playerVisual.Update(deltaTime, player.GetVisualState(input.running)); // 最后选择外观

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

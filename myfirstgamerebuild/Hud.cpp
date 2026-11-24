#include "Hud.h"
#include "Player.h"
#include "Enemy.h"
#include "Combat.h"
#include "PlayerVisual.h"
#include "GameConfig.h"

void DrawHud(const Player& player, const Enemy& enemy, const Combat& combat,
    const PlayerVisual& visual, bool showCollision)
{
    if (player.IsDead())
    {
        DrawText("U DEAD", GameConfig::ScreenWidth/2-100, GameConfig::ScreenHeight/2-50, 50, RED);
        DrawText("PRESS [R] TO RESTART", GameConfig::ScreenWidth/2-300, GameConfig::ScreenHeight/2, 50, RED);
    }
    DrawText(TextFormat("Health: %d", player.GetHealth()), 20, 20, 24, BLACK);
    DrawText(TextFormat("Enemy HP: %d", enemy.GetHealth()), 20, 104, 20, BLACK);
    if (combat.IsAttacking()) DrawText("ATTACK", 20, 132, 24, ORANGE);
    DrawText("A/D Walk | Shift Run | Space Jump | C Dash | J Attack | F1 Debug", 20, 52, 18, BLACK);
    if (showCollision)
        DrawText(TextFormat("Motion: %s | Frame: %d / %d | Ground: %d | FPS: %d",
            visual.GetMotionName(), visual.GetFrame()+1, visual.GetFrameCount(), player.IsGrounded(), GetFPS()), 20, 78, 18, BLACK);
}

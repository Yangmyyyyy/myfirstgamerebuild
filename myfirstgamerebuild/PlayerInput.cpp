#include "PlayerInput.h"
#include "raylib.h"

PlayerInput ReadPlayerInput()
{
    PlayerInput input;
    input.left = IsKeyDown(KEY_A);
    input.right = IsKeyDown(KEY_D);
    input.running = IsKeyDown(KEY_LEFT_SHIFT);
    input.jumpPressed = IsKeyPressed(KEY_SPACE);
    input.jumpReleased = IsKeyReleased(KEY_SPACE);
    input.attackPressed = IsKeyPressed(KEY_J);
    input.dashPressed = IsKeyPressed(KEY_C);
    input.restartPressed = IsKeyPressed(KEY_R);
    input.debugPressed = IsKeyPressed(KEY_F1);
    return input;
}

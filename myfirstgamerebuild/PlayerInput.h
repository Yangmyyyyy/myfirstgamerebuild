#pragma once
// 一帧的按键快照，不是玩家状态。放开按键后下一帧会重新读取。
struct PlayerInput
{
    bool left = false;
    bool right = false;
    bool running = false;
    bool jumpPressed = false;
    bool jumpReleased = false;
    bool attackPressed = false;
    bool dashPressed = false;
    bool restartPressed = false;
    bool debugPressed = false;
};
PlayerInput ReadPlayerInput(); // 主循环每帧读取一次，方便检查输入和做回归测试

//raylib 专门用来开发游戏和图形程序的库
#include "raylib.h"
#include "cmath"

// 重做 Day03（2026-11-16）：圆心坐标、WASD、四边限制、按秒移动与方向归一化。
// int main() 定义函数，花括号里放执行语句。int playerX = 400; 定义整数变量并给出初值。
// 以前误写 playerX += playerX + 1;，400 会变成 801。每帧只加 1 应写 playerX += 1;。
// C++ 提供类型、条件和循环。raylib 提供窗口、键盘、计时和绘制函数。
// SDL 和虚幻有各自的接口，坐标、位移、边界和调试这些编程方法可以继续使用。
int main()
{
	// 宽高传给 InitWindow 后决定绘图区的像素尺寸。坐标加 1 就沿对应轴移动一个像素单位。
	// const 给固定数值起名。画圆和边界共用半径，改源码初始值后重新生成即可。
	const int screenWidth = 1000;
	const int screenHeight = 600;
	const int playerRadius = 20;
	InitWindow(screenWidth,screenHeight,"My First Game");
	// 圆心从窗口中央开始。坐标在循环外定义，保留每帧移动后的值。float 可保存小数。
	float playerX = screenWidth / 2;
	float playerY = screenHeight / 2;
	// 这里约定速度单位为像素/秒，f 表示 float。运行时变速可用普通 float，或用基础速度乘状态倍率。
	float playerNormalSpeed = 200.0f;
	float playerDushSpeed = 300.0f;
	float playercurSpeed;
	while (!WindowShouldClose())
	{
		// raylib 的 GetFrameTime() 返回上一帧耗时（秒）。每轮重新读取，注意变量名大小写。
		// 当前未设置目标帧率。SetTargetFPS(60) 可设置目标，实际帧耗时仍可能变化。
		// 每轮按键时累计速度乘耗时。200 × 0.016 约为 3.2 像素。之前每帧加 1 会随帧率变快。
		// 固定时间步是另一种更新方式。当前移动用实际帧耗时。
		float deltaTime = GetFrameTime(); // 保存上一帧耗时，单位是秒
		// 方向每帧从零开始。四个独立 if 可以同时成立，屏幕 x 向右增大，y 向下增大。
		float directionX = 0.0f;
		float directionY = 0.0f;
		if (IsKeyDown(KEY_D))
			directionX += 1;
		if (IsKeyDown(KEY_A))
			directionX -= 1;
		if (IsKeyDown(KEY_W))
			directionY -= 1;
		if (IsKeyDown(KEY_S))
			directionY += 1;
		if (IsKeyDown(KEY_LEFT_SHIFT))
			playercurSpeed = playerDushSpeed;
		else
			playercurSpeed = playerNormalSpeed;
		// std::sqrt 来自 C++ 标准库。W+D 的方向 (1,-1) 长度为 √2，两个分量同除长度可保持方向并统一速度。
		// 长度为 0 跳过移动，长度为 1 直接移动。if 下有多条语句时用大括号，否则 else 可能失去对应的 if。
		// 也可先统一调整两个方向分量，再写一次坐标更新。两种方案的内存开销基本相同，合并主要方便维护。
		float directionLength = std::sqrt(directionX * directionX + directionY * directionY);
		if (directionLength != 0)
			if (directionLength > 1)
			{
				playerX += playercurSpeed * deltaTime * directionX / directionLength;
				playerY += playercurSpeed * deltaTime * directionY / directionLength;
			}
			else
			{
				playerX += playercurSpeed * deltaTime * directionX;
				playerY += playercurSpeed * deltaTime * directionY;
			}
        // 移动后先限制圆心。左、上留一个半径，右、下用窗口尺寸减半径。
        // raylib 默认在 EndDrawing 处理输入。把更新写在它后面也能赶上下次绘制。
        // 边界修正要在画圆前执行，否则画出的这一帧可能已经出界。
        if (playerX <= playerRadius)
			playerX = playerRadius;
		if (playerX >= screenWidth - playerRadius)
			playerX = screenWidth - playerRadius;
		if (playerY <= playerRadius)
			playerY = playerRadius;
		if (playerY >= screenHeight - playerRadius)
			playerY = screenHeight - playerRadius;
        BeginDrawing();
		ClearBackground(SKYBLUE);
		// DrawCircle 接收两个整数坐标，DrawCircleV 接收含两个 float 的 Vector2。
		// 用 {playerX, playerY} 组成一对坐标。写成 (playerX, playerY) 只会得到逗号右边的值。
		DrawCircleV({ playerX,playerY }, playerRadius, YELLOW);
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
//raylib 专门用来开发游戏和图形程序的库
#include "raylib.h"
#include "cmath"

// 重做 Day03（2026-11-16）：圆心坐标、WASD、四边限制、按秒移动与方向归一化。
// int main() 定义函数，花括号里放执行语句。int playerX = 400; 定义整数变量并给出初值。
// 以前误写 playerX += playerX + 1;，400 会变成 801。每帧只加 1 应写 playerX += 1;。
// C++ 提供类型、条件和循环。raylib 提供窗口、键盘、计时和绘制函数。
// SDL 和虚幻有各自的接口，坐标、位移、边界和调试这些编程方法可以继续使用。
// 重做 Day04（2026-11-17）
// 按住左 Shift 时每帧选 300 像素/秒，松开后选 200。直走和斜走共用当前速度。
// Rectangle 保存左上角坐标和宽高。DrawRectangleRec 接收整个矩形和颜色。
// ClearBackground 清当前画布。待绘制的形状可能到 EndDrawing 才提交并显示。
// 一般先 BeginDrawing，再清背景和画墙、玩家，最后 EndDrawing。
// raylib.h 已声明碰撞函数。参数依次是 Vector2 圆心、float 半径、Rectangle 墙。
// { playerX, playerY } 合成一个圆心参数。函数返回 bool，用 = 把结果存给 isTouchingWall。
// 角色外观可以很复杂，挡墙时仍可用脚边的小圆，挥剑时可另设攻击命中区域。
// 下次继续：整理重复检测和未完成的 if，再根据碰墙结果选择玩家圆的颜色。
// 重做 Day05（2026-11-18）
// 条件断点在程序到达该行时求值。放在检测完成后，可暂停查看本帧的碰撞结果。
// 先归一化方向，再分别尝试 X 和 Y 移动。每次只改变一轴，再用当前圆心检查墙。
// 撞墙时把旧值写回该轴。只把旧坐标传给 DrawCircleV，会改变画面但不会修正下一帧的起点。
// || 表示任一轴碰撞就给出变色反馈。绘制阶段使用已经处理好的玩家坐标。
// 重做 Day06（2026-11-19）：受伤、死亡重开与横版平台基础。
// Rec 接收 Rectangle；DrawCircleV 接收含两个 float 坐标的 Vector2。
// 圆对矩形碰撞依次接收圆心、半径、矩形三个参数；花括号把两个坐标组成一个圆心。
// if/else if/else 按顺序只选首个成立分支。默认白色，碰墙绿色，碰敌人红色。
// 最大血量是常量，当前血量可变且在循环外保存。TextFormat 的 %d 用整数值生成显示文字。
// 冷却在循环外保存，每帧先减少旧倒计时，再检测接触并扣血，受伤后重置为1秒。
// 输入前检查血量；移动后处理伤害；最后计算死亡并绘制，保证使用对应阶段的数据。
// 重开在输入前恢复血量、位置、冷却。加入重力后还需在重开时清零 velocityY，待补。
// velocityY 是纵向速度，重力乘帧耗时改变速度，速度乘帧耗时改变位置。正方向向下。
// platforms 是矩形数组，索引从0开始；地面和半空平台共用循环进行绘制与分轴碰撞。
// hitWallOnX/Y 定义在 for 外，循环里记录碰撞，后面的绘制才能读取。
// 平台纵向碰撞同时修正位置和速度。边界限制只改坐标时，速度仍会累积。
// 明天先讲 break（尚未理解），再实测平台、整理W/S和重开速度、接落地状态与跳跃。
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
	float velocityY = 0.0f;       // 当前纵向速度，单位是像素/秒，正数向下，负数向上
	const float gravity = 900.0f; // 重力加速度，每秒让向下速度增加 900 像素/秒，之后可调手感
	// 这里约定速度单位为像素/秒，f 表示 float。运行时变速可用普通 float，或用基础速度乘状态倍率。
	float playerNormalSpeed = 200.0f;
	float playerDushSpeed = 300.0f;
	float playercurSpeed;
	Rectangle platforms[] = {
	{ 0, 550, 1000, 50 }, // 地面，左上角 (0,550)，宽1000、高50
	{ 200, 430, 160, 20 }, // 较低的平台
	{ 430, 330, 160, 20 }  // 较高的平台
	};
	Rectangle enemy(250, 250, 40, 40);
	const int playerMaxHealth = 100;     // 最大血量固定为 100
	int playerHealth = playerMaxHealth; // 初始满血，受伤时修改当前血量
	float damageCooldown = 0.0f; // 距离下次允许受伤还剩几秒，初始为 0，允许立即受伤
	while (!WindowShouldClose())
	{
		// raylib 的 GetFrameTime() 返回上一帧耗时（秒）。每轮重新读取，注意变量名大小写。
		// 当前未设置目标帧率。SetTargetFPS(60) 可设置目标，实际帧耗时仍可能变化。
		// 每轮按键时累计速度乘耗时。200 × 0.016 约为 3.2 像素。之前每帧加 1 会随帧率变快。
		// 固定时间步是另一种更新方式。当前移动用实际帧耗时。
		float deltaTime = GetFrameTime(); // 保存上一帧耗时，单位是秒
		// 方向每帧从零开始。四个独立 if 可以同时成立，屏幕 x 向右增大，y 向下增大。
		if (playerHealth <= 0 && IsKeyPressed(KEY_R))
		{
			playerHealth = playerMaxHealth; // 按 R 键恢复满血
			playerX = screenWidth / 2;      // 重置玩家位置
			playerY = screenHeight / 2;
			damageCooldown = 0.0f; // 重置受伤冷却时间
		}
		if (damageCooldown > 0)
		{
			damageCooldown -= deltaTime; // 每帧减少冷却时间
		}
		float directionX = 0.0f;
		float directionY = 0.0f;

		if (IsKeyDown(KEY_LEFT_SHIFT))
			playercurSpeed = playerDushSpeed;
		else
			playercurSpeed = playerNormalSpeed;
		if (playerHealth > 0)
		{
			if (IsKeyDown(KEY_W))
				directionY -= 1.0f;
			if (IsKeyDown(KEY_S))
				directionY += 1.0f;
			if (IsKeyDown(KEY_A))
				directionX -= 1.0f;
			if (IsKeyDown(KEY_D))
				directionX += 1.0f;
		}
		// std::sqrt 来自 C++ 标准库。W+D 的方向 (1,-1) 长度为 √2，两个分量同除长度可保持方向并统一速度。
		// 长度为 0 跳过移动，长度为 1 直接移动。if 下有多条语句时用大括号，否则 else 可能失去对应的 if。
		// 也可先统一调整两个方向分量，再写一次坐标更新。两种方案的内存开销基本相同，合并主要方便维护。
		float oldX = playerX; // 保存本帧移动前的横坐标
		float oldY = playerY; // 保存本帧移动前的纵坐标
		float directionLength = std::sqrt(directionX * directionX + directionY * directionY);
		if (directionLength > 1)
		{
			directionX = directionX / directionLength;
			directionY = directionY / directionLength;// 斜走时缩小两个方向分量，保持总速度一致
		}
		playerX += directionX * playercurSpeed * deltaTime;
		bool hitWallOnX = false; // 放在循环外，后面的变色判断也能使用
		for (int i = 0; i < 3; i++)
		{
			if (CheckCollisionCircleRec({ playerX, playerY }, playerRadius, platforms[i]))
			{
				hitWallOnX = true; // 碰到任意一块平台，就记录下来
				playerX = oldX;    // 立即撤销横向移动
				break;            // 找到挡路的平台后停止检查
			}
		}
		velocityY += gravity * deltaTime; // 重力先改变纵向速度
		playerY += velocityY * deltaTime; // 再用纵向速度计算这一帧的位移
		bool hitWallOnY = false; // 循环外保存结果，后面的变色判断也要用
		for (int i = 0; i < 3; i++)
		{
			if (CheckCollisionCircleRec({ playerX, playerY }, playerRadius, platforms[i]))
			{
				hitWallOnY = true; // 记录纵向撞到平台
				playerY = oldY;    // 撤销这一步纵向移动
				velocityY = 0.0f; // 停止纵向运动
				break;            // 找到障碍后，结束检查
			}
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
		bool isTouchingWall = hitWallOnX || hitWallOnY; // 任一方向撞墙，这一帧就显示绿圆 // 把圆和墙是否重叠的结果存下来，等会儿用它决定墙的颜色
		bool isTouchingEnemy = CheckCollisionCircleRec({ playerX, playerY }, playerRadius, enemy);
		if (isTouchingEnemy == true && damageCooldown <= 0)
		{
			playerHealth -= 10;
			damageCooldown = 1.0f; // 设置受伤冷却时间
		}
		if (playerHealth <= 0 )
			playerHealth = 0;
		bool isDead = (playerHealth <= 0);

		BeginDrawing();
		ClearBackground(SKYBLUE);
		DrawText(TextFormat("Health: %d", playerHealth), 20, 20, 24, BLACK);
		// %d 留出一个整数位置，用 playerHealth 当前的值填进去
		for (int i = 0; i < 3; i++)
		{
			DrawRectangleRec(platforms[i], RED);
		}
		DrawRectangleRec(enemy, BLUE);
		// DrawCircle 接收两个整数坐标，DrawCircleV 接收含两个 float 的 Vector2。
		// 用 {playerX, playerY} 组成一对坐标。写成 (playerX, playerY) 只会得到逗号右边的值。
		if (isTouchingWall == true)
		    DrawCircleV({ playerX,playerY }, playerRadius, GREEN);
		else if (isTouchingEnemy == true)
			DrawCircleV({ playerX, playerY }, playerRadius, RED);
		else
		{
			DrawCircleV({ playerX,playerY }, playerRadius, WHITE);
		}
		if (isDead == true)
		{
			DrawText("U DEAD", screenWidth / 2 - 100, screenHeight / 2 - 50, 50, RED);
			DrawText("PRESS [R] TO RESTART", screenWidth / 2 - 300, screenHeight / 2 , 50, RED);
		}
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
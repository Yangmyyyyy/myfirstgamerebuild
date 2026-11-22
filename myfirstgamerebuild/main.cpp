//raylib 专门用来开发游戏和图形程序的库
#include "raylib.h"


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
// { player.x, player.y } 合成一个圆心参数。函数返回 bool，用 = 把结果存给 isTouchingWall。
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
// 重做 Day07：玩家现为40×40矩形，x/y表示左上角。平台与敌人使用CheckCollisionRecs。
// 分轴碰撞把身体贴到平台边缘；下落落地和上升顶头分别处理，只有落地设isGrounded。
int main()
{
	// 宽高传给 InitWindow 后决定绘图区的像素尺寸。坐标加 1 就沿对应轴移动一个像素单位。
	// 玩家绘制和碰撞共用宽高，边界限制也按这份尺寸计算。
	const int screenWidth = 1000;
	const int screenHeight = 600;
	const float playerWidth = 40.0f;  // 碰撞体宽度，像素
	const float playerHeight = 40.0f; // 碰撞体高度，像素
	InitWindow(screenWidth,screenHeight,"My First Game");
	// Rectangle 的 x/y 是左上角。减去一半宽高，让矩形中心仍在窗口中央。
	Rectangle player = { (screenWidth - playerWidth) / 2.0f, (screenHeight - playerHeight) / 2.0f, playerWidth, playerHeight };
	float facingDirection = 1.0f; // 初始朝右，放在循环外以保留方向
	float velocityY = 0.0f;       // 当前纵向速度，单位是像素/秒，正数向下，负数向上
	bool isGrounded = false; // 玩家是否已经落地，初始在空中
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
	Rectangle enemy = { 250, 350, 40, 40 };
	const int playerMaxHealth = 100;     // 最大血量固定为 100
	int playerHealth = playerMaxHealth; // 初始满血，受伤时修改当前血量
	float damageCooldown = 0.0f; // 距离下次允许受伤还剩几秒，初始为 0，允许立即受伤
	float dashTimeLeft = 0.0f; // 这次冲刺还剩多少秒，归零时结束冲刺
	float dashCooldown = 0.0f; // 距离下次允许冲刺还剩多少秒
	float dashDirection = 1.0f; // 保存本次冲刺方向，冲刺期间保持不变
	const float dashSpeed = 600.0f; // 冲刺速度，单位是像素/秒
	float dashInvincibleTime = 0.0f; // 冲刺无敌还剩多少秒，初始没有无敌
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
			player.x = (screenWidth - player.width) / 2.0f;      // 重置玩家位置
			player.y = (screenHeight - player.height) / 2.0f;
			isGrounded = false; // 重开后从空中重新检查落地
			damageCooldown = 0.0f; // 重置受伤冷却时间
			velocityY = 0.0f; // 重开时清掉上一次游戏留下的纵向速度
			dashTimeLeft = 0.0f; // 重开时清掉上一次游戏留下的冲刺时间
			dashCooldown = 0.0f; // 重开时清掉上一次游戏留下的冲刺冷却时间
			dashInvincibleTime = 0.0f; // 新一局从没有冲刺无敌的状态开始
		}
		if (damageCooldown > 0)
		{
			damageCooldown -= deltaTime; // 每帧减少冷却时间
		}
		float directionX = 0.0f;

		if (IsKeyDown(KEY_LEFT_SHIFT))
			playercurSpeed = playerDushSpeed;
		else
			playercurSpeed = playerNormalSpeed;
		if (playerHealth > 0)
		{
			if (IsKeyPressed(KEY_SPACE) && isGrounded)
			{
				velocityY = -400.0f; // 给一次向上的速度，负数表示向上
				isGrounded = false; // 起跳后立即进入空中状态
			}
			if (IsKeyDown(KEY_A))
				directionX -= 1.0f;
			if (IsKeyDown(KEY_D))
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
		if (IsKeyPressed(KEY_C) && dashCooldown <= 0 && playerHealth > 0)
		{
			dashTimeLeft = 0.4f; // 设置冲刺持续时间
			dashCooldown = 0.8f; // 设置冲刺冷却时间
			dashInvincibleTime = 0.4f; // 设置冲刺无敌时间
			dashDirection = facingDirection; // 发动时复制当前面向，锁定本次冲刺方向
		}
		// 每轴先尝试移动，再将矩形贴到挡路平台的表面。
		float moveX = directionX * playercurSpeed * deltaTime; // 默认使用普通横移

		if (dashTimeLeft > 0 && playerHealth > 0)
		{
			moveX = dashDirection * dashSpeed * deltaTime; // 冲刺时改用锁定方向的位移
		}

		player.x += moveX; // 本帧只执行一次横向移动
		bool hitWallOnX = false; // 放在循环外，后面的变色判断也能使用
		for (int i = 0; i < 3; i++)
		{
			if (CheckCollisionRecs(player, platforms[i]))
			{
				hitWallOnX = true; // 碰到任意一块平台，就记录下来
				if (moveX > 0)      // 本帧实际往右移动
				{
					player.x = platforms[i].x - player.width; // 向右撞墙，右边贴住墙左边
					dashTimeLeft = 0.0f; // 横向撞到平台，立即结束本次冲刺
				}
				else if (moveX < 0) // 本帧实际往左移动
				{
					player.x = platforms[i].x + platforms[i].width; // 向左撞墙，左边贴住墙右边
					dashTimeLeft = 0.0f; // 横向撞到平台，立即结束本次冲刺
				}
				break;            // 找到挡路的平台后停止检查
			}
		}
		velocityY += gravity * deltaTime; // 重力先改变纵向速度
		player.y += velocityY * deltaTime; // 再用纵向速度计算这一帧的位移
		isGrounded = false; // 本帧重新检查脚下支撑，避免走出平台后仍保持落地状态
		bool hitWallOnY = false; // 循环外保存结果，后面的变色判断也要用
		for (int i = 0; i < 3; i++)
		{
			if (CheckCollisionRecs(player, platforms[i]))
			{
				hitWallOnY = true; // 记录纵向撞到平台
				if (velocityY > 0)
				{
					player.y = platforms[i].y - player.height; // 下落：脚底贴住平台顶面
					isGrounded = true; // 只有脚踩平台才算落地
				}
				else if (velocityY < 0)
				{
					player.y = platforms[i].y + platforms[i].height; // 顶头：头顶贴住平台底面
				}
				velocityY = 0.0f; // 停止纵向运动
				break;            // 找到障碍后，结束检查
			}
		}
		// 矩形用左上角定位：右边是 x+width，脚底是 y+height。
		if (player.x < 0)
			player.x = 0;
		if (player.x > screenWidth - player.width)
			player.x = screenWidth - player.width;
		if (player.y < 0)
		{
			player.y = 0;
			if (velocityY < 0)
				velocityY = 0.0f; // 顶到窗口顶部，停止上升
		}
		if (player.y >= screenHeight - player.height)
		{
			player.y = screenHeight - player.height;
			velocityY = 0.0f; // 底部边界也必须同时修正速度
			isGrounded = true;
		}
		bool isTouchingWall = hitWallOnX || hitWallOnY; // 任一方向撞墙，这一帧就显示绿圆 // 把圆和墙是否重叠的结果存下来，等会儿用它决定墙的颜色
		bool isTouchingEnemy = CheckCollisionRecs(player, enemy);
		if (isTouchingEnemy == true && damageCooldown <= 0 && dashInvincibleTime == 0)
		{
			playerHealth -= 10;
			damageCooldown = 1.0f; // 设置受伤冷却时间
		}
		if (playerHealth <= 0 )
			playerHealth = 0;
		bool isDead = (playerHealth <= 0);
		if (isDead)
		{
			dashTimeLeft = 0.0f; // 这一帧扣血后若已死亡，结束剩余冲刺
			dashInvincibleTime = 0.0f; // 同时结束冲刺无敌
		}
		BeginDrawing();
		ClearBackground(SKYBLUE);
		DrawText(TextFormat("Health: %d", playerHealth), 20, 20, 24, BLACK);
		// %d 留出一个整数位置，用 playerHealth 当前的值填进去
		for (int i = 0; i < 3; i++)
		{
			DrawRectangleRec(platforms[i], RED);
		}
		DrawRectangleRec(enemy, BLUE);
		DrawRectangleRec(player, WHITE); // 显示与碰撞使用同一个矩形
		if (isDead == true)
		{
			DrawText("U DEAD", screenWidth / 2 - 100, screenHeight / 2 - 50, 50, RED);
			DrawText("PRESS [R] TO RESTART", screenWidth / 2 - 300, screenHeight / 2 , 50, RED);
		}
		DrawRectangleRec(player, WHITE);
		if (facingDirection > 0)
		{
			// 朝右：眼睛放在身体右侧，距离右边缘 8 像素
			DrawCircleV({ player.x + player.width - 8.0f, player.y + 12.0f }, 3.0f, BLACK);
		}
		else
		{
			// 朝左：眼睛放在身体左侧，距离左边缘 8 像素
			DrawCircleV({ player.x + 8.0f, player.y + 12.0f }, 3.0f, BLACK);
		}
		EndDrawing();
	}
	CloseWindow();
	return 0;
}
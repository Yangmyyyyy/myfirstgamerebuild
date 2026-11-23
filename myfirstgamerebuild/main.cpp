//raylib 专门用来开发游戏和图形程序的库
#include "raylib.h"
#include "PlayerVisual.h" // 角色动画的声明，具体实现见PlayerVisual.cpp
#include <filesystem> // 查找资源目录，让 Visual Studio 和双击 exe 都能加载图片


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
// Day07 收尾（2026-11-22）：以下是当前实现；上面的 Day03～06 注释保留当时学习过程。
// break 只退出本次最内层 for；下一帧 while 再次执行 for 时，i 从 0 重新开始。
// 已移除 W/S 和二维归一化；当前左右输入为 -1/0/1。重开时也清零纵向速度。
// isGrounded 先清 false，再由本帧下落碰平台设 true；顶头只清速度，不算落地。
// facingDirection 在循环外保留；没有左右输入时不覆盖，A+D 抵消时保持原朝向。
// C 是单次冲刺：600 像素/秒、持续0.4秒、冷却0.8秒、无敌0.4秒，均为当前测试值。
// 发动时复制方向；先计算 moveX，冲刺时覆盖普通位移，最后仅执行一次横移。
// 冲刺倒计时在发动前更新；无敌时间<=0归零，因此后面用==0判断当前是有效的。
// 撞平台结束冲刺；死亡与 R 重开清理状态。地图端点目前只修位置，没有单独取消冲刺。
// 闪烁：剩余时间/0.08 转整数后 %2 分奇偶；&& 先于 ||。身体重复绘制会覆盖前一次颜色。
// 二段跳：地面跳不消耗空跳，空中可再跳一次；落地/R恢复次数，顶头不恢复。
// 赋值 airJumpsLeft=1 修改原变量；在内层加 int 会声明同名新变量，遮蔽外层。
// 松空格且仍上升时速度乘0.5，可降低跳高；轻按/长按差异还需专项复测。
// 离地宽限0.1秒：地面刷新，空中倒计时，起跳立即清零，快速再按只能消耗一次空跳。
// if 内给外部变量赋值不会缩短它的寿命；velocityY==0也可能是空中最高点，不能表示落地。
// 窗口1000宽，世界3000宽；地面和边界使用世界宽，镜头仅改变显示，不移动世界坐标。
// Camera2D：target关注世界位置，offset决定屏幕位置；移动完成后更新镜头，绘制读取新位置。
// BeginMode2D/EndMode2D包住所有世界物体；血量等HUD在模式外，使用固定窗口坐标。
// 镜头横向限制500～2500；死区试验已按要求撤回，当前直接跟随。跟随已试玩通过。
// 待做：专项复测宽限与短跳；选一套许可明确的素材；攻击/AI/弹反/破防/处决尚未实现。
// Day08整理：外观加载/选帧/绘制/释放已移到PlayerVisual类；main保留玩法与地图。
// 明天阅读顺序：PlayerVisual.h的public接口 → 本文件的四个调用 → PlayerVisual.cpp实现。
int main()
{
	// 宽高传给 InitWindow 后决定绘图区的像素尺寸。坐标加 1 就沿对应轴移动一个像素单位。
	// 玩家绘制和碰撞共用宽高，边界限制也按这份尺寸计算。
	const int screenWidth = 1000;
	const int screenHeight = 600;
	const float worldWidth = 3000.0f; // 整个关卡的宽度，窗口每次只能显示其中一部分
	const float playerWidth = 40.0f;  // 碰撞体宽度，像素
	const float playerHeight = 40.0f; // 碰撞体高度，像素
	InitWindow(screenWidth,screenHeight,"My First Game" );
	SetTargetFPS(60); // 限制目标帧率；移动仍乘deltaTime，避免超高帧率下极小重力位移的精度问题
	// 先定位资源，再加载；窗口创建后才有上传纹理需要的图形环境。
	namespace fs = std::filesystem;
	fs::path resourceDir = "resources";
	if (!fs::exists(resourceDir / "knight_motion_v2.png"))
	{
		resourceDir = fs::path(GetApplicationDirectory()) / "resources";
	}
	Texture2D forestTexture = LoadTexture((resourceDir / "forest.png").string().c_str());
	Image terrainImage = LoadImage((resourceDir / "terrain.png").string().c_str());
	Texture2D terrainTexture = LoadTextureFromImage(terrainImage);
	Rectangle terrainFrames[2] = {}; // 0为厚地面，1为薄悬空平台
	if (terrainImage.data != nullptr)
	{
		int split = static_cast<int>(terrainImage.height * 0.64f); // 两条素材之间的透明间隙
		for (int row = 0; row < 2; row++)
		{
			int top = row == 0 ? 0 : split;
			int height = row == 0 ? split : terrainImage.height - split;
			Image part = ImageFromImage(terrainImage, { 0, static_cast<float>(top), static_cast<float>(terrainImage.width), static_cast<float>(height) });
			Rectangle bounds = GetImageAlphaBorder(part, 0.1f); // 只取草地本体，排除透明留白
			terrainFrames[row] = { bounds.x, top + bounds.y, bounds.width, bounds.height };
			UnloadImage(part);
		}
	}
	UnloadImage(terrainImage);
	SetTextureFilter(terrainTexture, TEXTURE_FILTER_POINT);
	PlayerVisual playerVisual; // 创建角色外观对象，在循环外保留动画状态
	playerVisual.Load(resourceDir.string().c_str()); // 图形环境已创建，加载一次图片
	bool showCollision = false; // F1 可显示原有碰撞矩形，便于检查外观与判定的区别
	// Rectangle 的 x/y 是左上角。减去一半宽高，让矩形中心仍在窗口中央。
	Rectangle player = { (screenWidth - playerWidth) / 2.0f, (screenHeight - playerHeight) / 2.0f, playerWidth, playerHeight };
	Camera2D camera = {}; // 各项先初始化为零
	camera.offset = { screenWidth / 2.0f, 0.0f }; // 跟随目标显示在窗口横向中央
	camera.target = { player.x + player.width / 2.0f, 0.0f }; // 关注玩家中心的横坐标
	camera.zoom = 1.0f; // 原尺寸显示，暂不缩放
	float facingDirection = 1.0f; // 初始朝右，放在循环外以保留方向
	float velocityY = 0.0f;       // 当前纵向速度，单位是像素/秒，正数向下，负数向上
	bool isGrounded = false; // 玩家是否已经落地，初始在空中
	const float gravity = 900.0f; // 重力加速度，每秒让向下速度增加 900 像素/秒，之后可调手感
	// 这里约定速度单位为像素/秒，f 表示 float。运行时变速可用普通 float，或用基础速度乘状态倍率。
	float playerNormalSpeed = 200.0f;
	float playerDushSpeed = 300.0f;
	float playercurSpeed;
	Rectangle platforms[] = {
	{ 0, 550, worldWidth, 50 }, // 地面，左上角 (0,550)，宽 worldWidth（3000）、高50
	{ 200, 430, 160, 20 }, // 较低的平台
	{ 430, 330, 160, 20 }  // 较高的平台
	};
	Rectangle enemy = { 250, 350, 40, 40 };
	const int enemyMaxHealth = 100; // 敌人最大血量，暂定100
	int enemyHealth = enemyMaxHealth; // 当前血量，之后攻击命中时修改
	const int playerMaxHealth = 100;     // 最大血量固定为 100
	int playerHealth = playerMaxHealth; // 初始满血，受伤时修改当前血量
	int airJumpsLeft = 1; // 剩余空中跳跃次数，落地后恢复为 1
	float coyoteTimeLeft = 0.0f; // 离开地面后，还允许地面起跳的剩余时间
	float damageCooldown = 0.0f; // 距离下次允许受伤还剩几秒，初始为 0，允许立即受伤
	float dashTimeLeft = 0.0f; // 这次冲刺还剩多少秒，归零时结束冲刺
	float dashCooldown = 0.0f; // 距离下次允许冲刺还剩多少秒
	float dashDirection = 1.0f; // 保存本次冲刺方向，冲刺期间保持不变
	const float dashSpeed = 600.0f; // 冲刺速度，单位是像素/秒
	float dashInvincibleTime = 0.0f; // 冲刺无敌还剩多少秒，初始没有无敌
	float attackTimeLeft = 0.0f; // 本次攻击还剩多少秒；0表示没有攻击
	float attackDirection = 1.0f; // 保存本次攻击方向，1向右，-1向左
	while (!WindowShouldClose())
	{
		// raylib 的 GetFrameTime() 返回上一帧耗时（秒）。每轮重新读取，注意变量名大小写。
		// 当前目标帧率60，实际耗时仍可能变化，移动和动画都按deltaTime推进。
		// 每轮按键时累计速度乘耗时。200 × 0.016 约为 3.2 像素。之前每帧加 1 会随帧率变快。
		// 固定时间步是另一种更新方式。当前移动用实际帧耗时。
		float deltaTime = GetFrameTime(); // 保存上一帧耗时，单位是秒
		if (attackTimeLeft > 0)
			attackTimeLeft -= deltaTime; // 每帧减少已经开始的攻击时间
		float frameStartX = player.x; // 用实际位移区分行走和推墙，不只看有没有按键
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
			airJumpsLeft = 1; // 新一局从一次空中跳跃机会开始
			coyoteTimeLeft = 0.0f; // 重开时清除上一局的宽限时间
		}
		if (isGrounded)
		{
			coyoteTimeLeft = 0.1f; // 站在地面上，持续准备好 0.1 秒宽限
		}
		else if (coyoteTimeLeft > 0)
		{
			coyoteTimeLeft -= deltaTime; // 已经离地，逐帧消耗宽限时间
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
			if (IsKeyPressed(KEY_SPACE) &&
				(isGrounded || coyoteTimeLeft > 0 || airJumpsLeft > 0))
			{
				if (!isGrounded && coyoteTimeLeft <= 0)
				{
					airJumpsLeft -= 1; // 已离地且宽限结束，才消耗空中跳跃次数
				}

				velocityY = -400.0f;  // 设置起跳速度
				isGrounded = false;   // 起跳后进入空中状态
				coyoteTimeLeft = 0.0f; // 本次起跳用完地面机会，避免再次免费跳
			}
			if (IsKeyReleased(KEY_SPACE) && velocityY < 0)
			{
				velocityY *= 0.5f; // 上升中松开空格，把剩余向上速度减半
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
		if (IsKeyPressed(KEY_J) &&
			playerHealth > 0 &&
			dashTimeLeft <= 0 &&
			attackTimeLeft <= 0)
		{
			attackTimeLeft = 0.2f; // 刚按下J，开启0.2秒攻击状态
			attackDirection = facingDirection; // 发动瞬间复制面向，本次攻击不再随转身改变
		}
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
		bool hitWallOnX = false; // 放在循环外，保留本帧横向碰撞结果
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
		bool hitWallOnY = false; // 循环外保存本帧纵向碰撞结果
		for (int i = 0; i < 3; i++)
		{
			if (CheckCollisionRecs(player, platforms[i]))
			{
				hitWallOnY = true; // 记录纵向撞到平台
				if (velocityY > 0)
				{
					player.y = platforms[i].y - player.height; // 下落：脚底贴住平台顶面
					isGrounded = true; // 只有脚踩平台才算落地
					airJumpsLeft = 1; // 剩余空中跳跃次数，落地后恢复为 1
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
		if (player.x > worldWidth - player.width)
			player.x = worldWidth - player.width; // 限制在关卡右端
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
			airJumpsLeft = 1; // 剩余空中跳跃次数，落地后恢复为 1
		}
		bool isTouchingWall = hitWallOnX || hitWallOnY; // 合并两轴结果；当前闪烁绘制未使用此变量
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
		camera.target.x = player.x + player.width / 2.0f; // 每帧直接跟随玩家中心
		if (camera.target.x < screenWidth / 2.0f)
		{
			camera.target.x = screenWidth / 2.0f; // 左端停住，窗口左边刚好对齐地图左边
		}

		if (camera.target.x > worldWidth - screenWidth / 2.0f)
		{
			camera.target.x = worldWidth - screenWidth / 2.0f; // 右端停住，避免看到地图右侧外面
		}
		Rectangle attackBox = {
	        player.x + player.width, // 默认放在玩家右边，紧挨身体
        	player.y,                // 顶部暂时与玩家碰撞框对齐
        	60.0f,                   // 向前伸出60像素
	        player.height            // 高度暂时与玩家碰撞框相同
		};

		if (attackDirection < 0)
		{
			attackBox.x = player.x - attackBox.width;
			// 朝左时，攻击框放在身体左边
		}
		// 物理/伤害处理完成后，再把最新结果交给外观类；它不会修改player或攻击状态。
		PlayerVisualState visualState;
		visualState.dead = isDead;
		visualState.grounded = isGrounded;
		visualState.moving = directionX != 0 && player.x != frameStartX;
		visualState.running = IsKeyDown(KEY_LEFT_SHIFT);
		visualState.velocityY = velocityY;
		visualState.facingDirection = facingDirection;
		visualState.dashDirection = dashDirection;
		visualState.dashTimeLeft = dashTimeLeft;
		visualState.dashInvincibleTime = dashInvincibleTime;
		visualState.damageCooldown = damageCooldown;
		playerVisual.Update(deltaTime, visualState); // 先更新动画，再开始绘制这一帧
		BeginDrawing();
		ClearBackground(SKYBLUE);
		// 背景是远景，先在窗口坐标下绘制，裁掉原照片四周灰框；平台仍在镜头内。
		if (forestTexture.id != 0)
			DrawTexturePro(forestTexture, { 24, 80, 1233, 693 }, { 0, 0, 1000, 600 }, { 0, 0 }, 0, WHITE);
		BeginMode2D(camera); // 开始用镜头显示地图里的物体
		// 你现有的平台、敌人、玩家和眼睛绘制代码放在这里
		for (int i = 0; i < 3; i++)
		{
			Rectangle platform = platforms[i]; // 复制仅用于绘制，原来的碰撞矩形不改
			if (terrainTexture.id == 0)
			{
				DrawRectangleRec(platform, RED); // 素材丢失时保留可见的测试平台
				continue;
			}
			Rectangle source = terrainFrames[i == 0 ? 0 : 1];
			if (i == 0)
			{
				// 厚地面重复铺，末块只画剩余部分，避免把岩石拉成长条。
				float tileHeight = platform.height + 8.0f;
				float tileWidth = source.width / source.height * tileHeight;
				for (float x = platform.x; x < platform.x + platform.width; x += tileWidth)
				{
					float width = tileWidth;
					if (x + width > platform.x + platform.width) width = platform.x + platform.width - x;
					Rectangle piece = source;
					piece.width *= width / tileWidth;
					DrawTexturePro(terrainTexture, piece, { x, platform.y - 8, width, tileHeight }, { 0, 0 }, 0, WHITE);
				}
			}
			else
			{
				// 左右端各保留一块，只有中间延展；草叶可超出碰撞体，但脚底仍站在原平台顶面。
				float sourceCap = source.width * 0.1f;
				float cap = 16.0f;
				float y = platform.y - 6.0f;
				float height = platform.height + 10.0f;
				DrawTexturePro(terrainTexture, { source.x, source.y, sourceCap, source.height },
					{ platform.x, y, cap, height }, { 0, 0 }, 0, WHITE);
				DrawTexturePro(terrainTexture, { source.x + sourceCap, source.y, source.width - 2 * sourceCap, source.height },
					{ platform.x + cap, y, platform.width - 2 * cap, height }, { 0, 0 }, 0, WHITE);
				DrawTexturePro(terrainTexture, { source.x + source.width - sourceCap, source.y, sourceCap, source.height },
					{ platform.x + platform.width - cap, y, cap, height }, { 0, 0 }, 0, WHITE);
			}
		}
		DrawRectangleRec(enemy, BLUE);
		if (attackTimeLeft > 0 && playerHealth > 0 && dashTimeLeft <= 0)
		{
			DrawRectangleRec(attackBox, Fade(ORANGE, 0.35f));
			// 半透明橙色填充，方便看范围，也能看见后面的物体

			DrawRectangleLinesEx(attackBox, 2.0f, ORANGE);
			// 橙色边框，线宽2像素
		}
		playerVisual.Draw(player); // 在世界坐标下绘制，使用物理处理后的玩家位置
		if (IsKeyPressed(KEY_F1)) showCollision = !showCollision;
		if (showCollision) DrawRectangleLinesEx(player, 1.0f, LIME);
		EndMode2D(); // 后面的文字恢复使用窗口坐标
		if (isDead == true)
		{
			DrawText("U DEAD", screenWidth / 2 - 100, screenHeight / 2 - 50, 50, RED);
			DrawText("PRESS [R] TO RESTART", screenWidth / 2 - 300, screenHeight / 2, 50, RED);
		}
		DrawText(TextFormat("Health: %d", playerHealth), 20, 20, 24, BLACK);
		DrawText(TextFormat("Enemy HP: %d", enemyHealth), 20, 104, 20, BLACK);
		if (attackTimeLeft > 0)
		{
			DrawText("ATTACK", 20, 132, 24, ORANGE);
			// 攻击状态持续期间显示文字，暂时用于确认按键和计时是否正常
		}
		// 在屏幕固定位置显示敌人血量，%d由enemyHealth的整数值替换
		DrawText("A/D Walk | Shift Run | Space Jump | C Dash | F1 Debug", 20, 52, 18, BLACK);
		if (showCollision) DrawText(TextFormat("Motion: %s | Frame: %d / 6 | Ground: %d | FPS: %d", playerVisual.GetMotionName(), playerVisual.GetFrame() + 1, isGrounded, GetFPS()), 20, 78, 18, BLACK);
		// %d 留出一个整数位置，用 playerHealth 当前的值填进去
		EndDrawing();
	}
	playerVisual.Unload(); // 窗口仍存在时，释放外观类拥有的纹理
	UnloadTexture(forestTexture);
	UnloadTexture(terrainTexture);
	CloseWindow();
	return 0;
}

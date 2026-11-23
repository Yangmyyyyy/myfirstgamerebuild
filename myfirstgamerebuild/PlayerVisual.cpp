#include "PlayerVisual.h" // 先包含声明，编译器才知道这些函数属于哪个类
#include <filesystem>

// PlayerVisual:: 表示“这是PlayerVisual类里面声明的函数的具体实现”。
// .h告诉别人能调用什么；.cpp负责说明这些功能具体怎样完成。
void PlayerVisual::Load(const char* resourceDirectory)
{
    Unload(); // 将来重新加载时先清理已有资源
    const std::filesystem::path resourceDir(resourceDirectory);
    Image knightImage = LoadImage((resourceDir / "knight_motion_v2.png").string().c_str());
    knightTexture = knightImage.data != nullptr ? LoadTextureFromImage(knightImage) : Texture2D{};
    spriteScale = 1.0f;
    if (knightImage.data != nullptr)
    {
    float maxRunHeight = 1.0f;
    for (int row = 0; row < 4; row++)
    {
        for (int frame = 0; frame < 6; frame++)
        {
            int left = frame * knightImage.width / 6;
            int top = row * knightImage.height / 4;
            Rectangle cell = { static_cast<float>(left), static_cast<float>(top),
                static_cast<float>((frame + 1) * knightImage.width / 6 - left),
                static_cast<float>((row + 1) * knightImage.height / 4 - top) };
            Image part = ImageFromImage(knightImage, cell);
            Rectangle bounds = GetImageAlphaBorder(part, 0.1f); // 排除透明留白，脚底才容易对齐
            animationFrames[row][frame] = { left + bounds.x, top + bounds.y, bounds.width, bounds.height };
            if (row == 0 && bounds.height > maxRunHeight) maxRunHeight = bounds.height;
            UnloadImage(part);
        }
    }
    spriteScale = 76.0f / maxRunHeight; // 美术身高约76像素；40×40碰撞体仍独立保留
    }
    UnloadImage(knightImage); // 图片已上传为纹理，释放 CPU 侧图片内存
    if (knightTexture.id != 0) SetTextureFilter(knightTexture, TEXTURE_FILTER_POINT); // 像素图使用最近邻采样
    // 跑步单独用修正换腿的图集，不改已经认可的待机、行走与冲刺图片。
    Image runImage = LoadImage((resourceDir / "knight_run_v3.png").string().c_str());
    runTexture = runImage.data != nullptr ? LoadTextureFromImage(runImage) : Texture2D{};
    runScale = 1.0f;
    if (runImage.data != nullptr)
    {
    float maxHeight = 1.0f;
    for (int frame = 0; frame < 6; frame++)
    {
        int column = frame % 3; // 六帧按三列两行保存
        int row = frame / 3;
        int left = column * runImage.width / 3;
        int top = row * runImage.height / 2;
        Image part = ImageFromImage(runImage, { static_cast<float>(left), static_cast<float>(top),
            static_cast<float>((column + 1) * runImage.width / 3 - left),
            static_cast<float>((row + 1) * runImage.height / 2 - top) });
        Rectangle bounds = GetImageAlphaBorder(part, 0.1f);
        runFrames[frame] = { left + bounds.x, top + bounds.y, bounds.width, bounds.height };
        if (bounds.height > maxHeight) maxHeight = bounds.height;
        UnloadImage(part);
    }
    runScale = 72.0f / maxHeight; // 跑步身体前倾，外观略低于76像素的站姿
    }
    UnloadImage(runImage);
    if (runTexture.id != 0) SetTextureFilter(runTexture, TEXTURE_FILTER_POINT);
}

void PlayerVisual::Update(float deltaTime, const PlayerVisualState& state)
{
    // 把剩余无敌时间分成每档 0.08 秒，转换为整数档位
    int flashStep = static_cast<int>(state.dashInvincibleTime / 0.08f);
    int flashStep2 = static_cast<int>(state.damageCooldown / 0.08f);
    tint = ((state.dashInvincibleTime > 0 && flashStep % 2 == 0) ||
        (state.damageCooldown > 0 && flashStep2 % 2 == 0)) ? YELLOW : WHITE;
    // 先读取物理更新后的状态，再选外观；不在这里修改碰撞或速度。
    motion = Motion::Idle;
    if (state.dead) motion = Motion::Dead;
    else if (state.dashTimeLeft > 0) motion = Motion::Dash;
    else if (!state.grounded) motion = state.velocityY < 0 ? Motion::Rise : Motion::Fall;
    else if (state.moving)
        motion = state.running ? Motion::Run : Motion::Walk;

    // 同一动作内计时一直保留，只有切换动作才从头播放；待机也需要播放呼吸循环。
    if (motion != previousAnimation) { animationTime = 0.0f; previousAnimation = motion; }
    if (motion != Motion::Dead) animationTime += deltaTime;
    animation = 0;
    if (motion == Motion::Walk) animation = 1;
    else if (motion == Motion::Run || motion == Motion::Rise || motion == Motion::Fall) animation = 2;
    else if (motion == Motion::Dash) animation = 3;
    float frameSeconds = 0.11f; // 行走保持原来的节奏
    if (motion == Motion::Idle) frameSeconds = 0.18f; // 待机保持缓慢呼吸
    else if (motion == Motion::Run) frameSeconds = 0.10f; // 跑步从0.075调慢，六帧一轮0.60秒，不改变移动速度
    frame = static_cast<int>(animationTime / frameSeconds) % 6;
    if (motion == Motion::Dash)
    {
        frame = static_cast<int>((0.4f - state.dashTimeLeft) / (0.4f / 6));
        if (frame < 0) frame = 0;
        if (frame > 5) frame = 5;
    }
    // 空中先使用收腿的独立取帧，停止在空中跑步；专用跳跃/死亡图仍待补全。
    if (motion == Motion::Rise) frame = 1;
    if (motion == Motion::Fall) frame = 2;
    if (motion == Motion::Dead) frame = 0;

    grounded = state.grounded;
    lookDirection = motion == Motion::Dash ? state.dashDirection : state.facingDirection;
}

// const Rectangle&是只读引用：使用玩家的矩形，不复制，也不能在这里改变它。
// 函数末尾const表示Draw不改变这个外观对象的成员。
void PlayerVisual::Draw(const Rectangle& player) const
{
    if (knightTexture.id != 0)
    {
        Rectangle source = animationFrames[animation][frame];
        Texture2D activeTexture = knightTexture;
        float activeScale = spriteScale;
        if (motion == Motion::Run && runTexture.id != 0)
        {
            source = runFrames[frame]; // 只有跑步换成左右腿交替的六帧
            activeTexture = runTexture;
            activeScale = runScale;
        }
        float width = source.width * activeScale;
        float height = source.height * activeScale;
        // 只修正外观：落地时靴底略压入草面，避免贴图边缘留白显得悬空。
        float footOffset = grounded ? 3.0f : 0.0f;
        if (grounded)
            DrawEllipse(static_cast<int>(player.x + player.width / 2),
                static_cast<int>(player.y + player.height + 2), 14, 2, Fade(BLACK, 0.20f)); // 脚边接触阴影，先画在角色下面
        float look = lookDirection; // 朝向在Update中从玩家状态读取
        if (look < 0) source.width = -source.width; // 负截取宽度实现水平翻转，不需重复保存左向图片
        DrawTexturePro(activeTexture, source,
            { player.x + player.width / 2 - width / 2, player.y + player.height - height + footOffset, width, height },
            { 0, 0 }, 0, tint); // 图片脚底在碰撞体脚底基础上增加视觉偏移，物理不变
    }
    else DrawRectangleRec(player, tint); // 素材丢失时保留矩形，游戏仍可检查
}

void PlayerVisual::Unload()
{
    if (runTexture.id != 0) UnloadTexture(runTexture);
    if (knightTexture.id != 0) UnloadTexture(knightTexture);
    runTexture = {};
    knightTexture = {};
    animationTime = 0.0f;
    motion = Motion::Idle;
    previousAnimation = Motion::Idle;
    animation = 0;
    frame = 0;
    grounded = false;
    lookDirection = 1.0f;
    tint = WHITE;
}

const char* PlayerVisual::GetMotionName() const
{
    const char* names[] = { "Idle", "Walk", "Run", "Dash", "Rise", "Fall", "Dead" };
    return names[static_cast<int>(motion)];
}

int PlayerVisual::GetFrame() const
{
    return frame;
}

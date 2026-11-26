#include "PlayerVisual.h"
#include "HeroAnimationData.h"
#include "HeroStyle.h"
#include "GameConfig.h"
#include <algorithm>
#include <filesystem>

void PlayerVisual::Load(const char* resourceDirectory)
{
    Unload(); // 重新加载前先释放旧纹理；窗口必须已创建
    const std::filesystem::path folder = std::filesystem::path(resourceDirectory) / "hero";
    for (int i = 0; i < 5; ++i)
    {
        heroTextures[i] = LoadTexture((folder / HeroAnimationData::Clips[i].file).string().c_str());
        if (heroTextures[i].id != 0) SetTextureFilter(heroTextures[i], TEXTURE_FILTER_POINT);
    }
    heroTextures[5] = LoadTexture(
        (folder / HeroAnimationData::AttackChop.file).string().c_str()
    ); // 拼出竖劈图片的路径，加载后把纹理保存到第6项

    if (heroTextures[5].id != 0)
    {
        SetTextureFilter(heroTextures[5], TEXTURE_FILTER_POINT); // 加载成功后使用点采样，让像素边缘保持清晰
    }
    heroTextures[6] = LoadTexture(
        (folder / HeroAnimationData::AttackSlash.file).string().c_str()
    ); // 读取第二段横劈图片，保存到下标6

    heroTextures[7] = LoadTexture(
        (folder / HeroAnimationData::AttackThrust.file).string().c_str()
    ); // 读取第三段突刺图片，保存到下标7

    if (heroTextures[6].id != 0)
    {
        SetTextureFilter(heroTextures[6], TEXTURE_FILTER_POINT); // 横劈加载成功后，使用点采样
    }

    if (heroTextures[7].id != 0)
    {
        SetTextureFilter(heroTextures[7], TEXTURE_FILTER_POINT); // 突刺加载成功后，使用点采样
    }
    paletteShader = LoadShader(nullptr, (folder / "hero_palette.fs").string().c_str());
    clothGainLocation = GetShaderLocation(paletteShader, "clothGain");
    leatherGainLocation = GetShaderLocation(paletteShader, "leatherGain");
    skinGainLocation = GetShaderLocation(paletteShader, "skinGain");
    referenceModeLocation = GetShaderLocation(paletteShader, "referenceMode");
    paletteReady = clothGainLocation >= 0 && leatherGainLocation >= 0 && skinGainLocation >= 0 && referenceModeLocation >= 0;
}

void PlayerVisual::Update(float deltaTime, const PlayerVisualState& state)
{
    int flashStep = static_cast<int>(state.dashInvincibleTime / 0.08f);
    int damageStep = static_cast<int>(state.damageCooldown / 0.08f);
    tint = ((state.dashInvincibleTime > 0 && flashStep % 2 == 0) ||
        (state.damageCooldown > 0 && damageStep % 2 == 0)) ? YELLOW : WHITE;

    // 使用物理报告的落地变化，只播放外观缓冲，不改速度/跳跃/碰撞。
    if (!grounded && state.grounded && (motion == Motion::Rise || motion == Motion::Fall))
        landingTimeLeft = 0.12f;
    else
        landingTimeLeft = std::max(0.0f, landingTimeLeft - deltaTime);

    motion = Motion::Idle;
    if (state.dead) motion = Motion::Dead;
    else if (state.dashTimeLeft > 0) motion = Motion::Dash;
    else if (state.attacking)
    {
        if (state.comboStep == 2)
        {
            motion = Motion::AttackSlash; // 第二段播放横劈
        }
        else if (state.comboStep == 3)
        {
            motion = Motion::AttackThrust; // 第三段播放突刺
        }
        else
        {
            motion = Motion::AttackChop; // 第一段播放竖劈
        }
    }
    else if (!state.grounded) motion = state.velocityY < 0 ? Motion::Rise : Motion::Fall;
    else if (state.moving) motion = state.running ? Motion::Run : Motion::Walk;
    else if (landingTimeLeft > 0) motion = Motion::Land;

    bool locomoting = motion == Motion::Walk || motion == Motion::Run;
    bool wasLocomoting = previousAnimation == Motion::Walk || previousAnimation == Motion::Run;
    if (!locomoting || !wasLocomoting) locomotionPhase = 0.0f; // 停下、起跳或攻击后，下次重新迈步；走跑互切保留阶段
    if (locomoting)
    {
        float cycle = motion == Motion::Run ? GameConfig::RunAnimationCycle : GameConfig::WalkAnimationCycle;
        locomotionPhase += deltaTime / cycle; // 一轮所需时间越短，相同步长内推进越快
        locomotionPhase -= static_cast<int>(locomotionPhase); // 去掉完整轮数，只留下当前这一轮的进度
    }
    if (motion != previousAnimation) { animationTime = 0.0f; previousAnimation = motion; }
    animationTime += deltaTime;
    animation = 0;
    if (motion == Motion::Walk) animation = 1;
    else if (motion == Motion::Run) animation = 2;
    else if (motion == Motion::Rise || motion == Motion::Fall || motion == Motion::Land) animation = 3;
    else if (motion == Motion::Dash) animation = 4;
    else if (motion == Motion::AttackChop) animation = 5; // 第六张纹理是竖劈图片
    else if (motion == Motion::AttackSlash) animation = 6; // 横劈纹理与数据的编号
    else if (motion == Motion::AttackThrust) animation = 7; // 突刺纹理与数据的编号

    const auto& clip = HeroAnimationData::GetClip(animation); // 按编号取得对应动作数据
    frame = static_cast<int>(animationTime / clip.seconds) % clip.count;
    // 跳跃跟随实际物理阶段，不循环整段动画，也不再叠加预览页模拟的位移。
    if (locomoting) frame = std::min(clip.count - 1, static_cast<int>(locomotionPhase * clip.count)); // 用共享步态进度选择走跑帧
    else if (motion == Motion::Rise) frame = std::min(3, 1 + static_cast<int>(animationTime / clip.seconds));
    else if (motion == Motion::Fall) frame = std::min(5, 4 + static_cast<int>(animationTime / clip.seconds));
    else if (motion == Motion::Land) frame = landingTimeLeft > 0.06f ? 6 : 7;
    else if (motion == Motion::Dash)
    {
        float progress = (GameConfig::DashDuration - state.dashTimeLeft) / GameConfig::DashDuration;
        frame = std::clamp(static_cast<int>(progress * clip.count), 0, clip.count - 1);
    }
    else if (motion == Motion::AttackChop ||
        motion == Motion::AttackSlash ||
        motion == Motion::AttackThrust)
    {
        float elapsed = GameConfig::AttackDuration - state.attackTimeLeft; // 本次攻击已经经过的时间
        float progress = std::clamp(elapsed / GameConfig::AttackDuration, 0.0f, 1.0f); // 总进度限制在0到1
        const float* frameWeights = GameConfig::AttackChopFrameWeights; // 默认读取第一段的帧时长表
        if (motion == Motion::AttackSlash) frameWeights = GameConfig::AttackSlashFrameWeights;
        else if (motion == Motion::AttackThrust) frameWeights = GameConfig::AttackThrustFrameWeights;
        float frameEnd = 0.0f; // 累加每帧比例，得到这一帧结束时的进度
        frame = clip.count - 1; // 如果进度已到末尾，保留最后一帧，不循环回举剑
        for (int i = 0; i < clip.count; ++i)
        {
            frameEnd += frameWeights[i]; // 前面帧的时长加起来，形成当前帧的结束边界
            if (progress < frameEnd)
            {
                frame = i; // 进度落在这一帧的区间，选择这一帧
                break; // 已找到本次要画的帧，不再检查后面的帧
            }
        }
    }
    else if (motion == Motion::Dead) frame = 0; // 包内没有死亡动作，明确保留待机占位

    grounded = state.grounded;
    lookDirection = motion == Motion::Dash ? state.dashDirection : state.facingDirection;
}

void PlayerVisual::Draw(const Rectangle& player) const
{
    if (heroTextures[animation].id == 0) { DrawRectangleRec(player, tint); return; }
    const auto& clip = HeroAnimationData::GetClip(animation); // 按编号取得对应动作数据
    const auto& pose = clip.frames[frame];
    Rectangle source = pose.source;
    float width = source.width * clip.scale;
    float height = source.height * clip.scale;
    float xFromAnchor = (source.x - pose.anchor.x) * clip.scale;
    float centerX = player.x + player.width / 2;
    float footY = player.y + player.height + 3.0f; // 同一绘制基准用于地面和空中，起跳时不突然少3像素
    if (grounded)
        DrawEllipse(static_cast<int>(centerX), static_cast<int>(footY), 12, 2, Fade(BLACK, 0.16f));
    // 围绕脚底锚点翻转，剑伸得更远时也不会把身体挤到碰撞框另一侧。
    float x = centerX + xFromAnchor;
    if (lookDirection < 0) { source.width = -source.width; x = centerX - xFromAnchor - width; }
    if (paletteReady)
    {
        int profileIndex = animation; // 移动动作先沿用原编号
        if (animation == 5) profileIndex = 7; // 竖劈对应调色表下标7
        else if (animation == 6) profileIndex = 5; // 横劈对应下标5
        else if (animation == 7) profileIndex = 6; // 突刺对应下标6
        const auto& profile = HeroStyle::Profiles[profileIndex]; // 读取该动作的调色配置
        SetShaderValue(paletteShader, clothGainLocation, profile.cloth, SHADER_UNIFORM_VEC3);
        SetShaderValue(paletteShader, leatherGainLocation, profile.leather, SHADER_UNIFORM_VEC3);
        SetShaderValue(paletteShader, skinGainLocation, profile.skin, SHADER_UNIFORM_VEC3);
        float referenceMode = 1.0f; // 新图已按冲刺统一配色，保留原色和刀光；tint仍负责受伤闪烁 // 行走是原色基准
        SetShaderValue(paletteShader, referenceModeLocation, &referenceMode, SHADER_UNIFORM_FLOAT);
        BeginShaderMode(paletteShader); // 只有这一张角色贴图使用调色，平台和背景不变
    }
    DrawTexturePro(heroTextures[animation], source,
        { x, footY + (pose.source.y - pose.anchor.y) * clip.scale, width, height },
        { 0, 0 }, 0, tint);
    if (paletteReady) EndShaderMode();
}

void PlayerVisual::Unload()
{
    for (auto& texture : heroTextures)
    {
        if (texture.id != 0) UnloadTexture(texture);
        texture = {};
    }
    if (paletteShader.id != 0) UnloadShader(paletteShader);
    paletteShader = {};
    paletteReady = false;
    clothGainLocation = leatherGainLocation = skinGainLocation = referenceModeLocation = -1;
    animationTime = 0.0f;
    locomotionPhase = 0.0f; // 释放/重新加载时也清理步态
    landingTimeLeft = 0.0f;
    motion = previousAnimation = Motion::Idle;
    animation = frame = 0;
    grounded = false;
    lookDirection = 1.0f;
    tint = WHITE;
}

const char* PlayerVisual::GetMotionName() const
{
    const char* names[] = {
     "Idle", "Walk", "Run", "Dash", "Rise", "Fall", "Land", "Dead",
     "AttackChop", "AttackSlash", "AttackThrust"
    }; // 名称顺序必须与Motion中的顺序一致
    return names[static_cast<int>(motion)];
}

int PlayerVisual::GetFrame() const { return frame; }
int PlayerVisual::GetFrameCount() const
{
    return HeroAnimationData::GetClip(animation).count; // 先取得当前动作数据，再返回它的帧数
}

// Day09 助手代做/待学习：使用用户素材包接入5组外观，原frames.json与攻击图完整归档。
// 仅换贴图及播放；Player/Combat/Enemy/Collision规则未改，攻击动画与刀光尚未接入。
// 死亡暂用待机首帧；预览页的offsetX/offsetY不作为游戏位移，避免跳跃冲刺叠加。
// 昨日课堂知识：Load准备纹理，Update选帧，Draw显示，Unload释放；const引用只读原数据。
// 三元表达式选择真假值；static_cast<int>截小数；%循环帧号或切换黄白闪烁。

// Day09统一外观：HeroStyle按行走配色校准，HeroAnimationData按站姿校准整体大小。
// 调色器只在Draw期间影响角色；所有PNG、动作帧、脚底锚点、碰撞与位移保持原样。

// Day09修正：旧待机的肩腰/四肢与行走不一致，现已换成按行走体型重绘的slim_v2图。
// 所有已启用动作共用行走色板；攻击修订稿仍仅归档，未改变战斗逻辑。
// Day10 助手代做：按冲刺参考重绘7组图；保留原冲刺图与所有旧版备份。
// 刀光属于贴图外观，不会扩大攻击判定框；人物缩放与包含剑、刀光的截图总范围分开校准。
// HeroAnimationData记录新版截图/脚底锚点/缩放；横劈与突刺仅备好数据，尚未接入三段连击。
// referenceMode保留新版原色与浅金刀光；tint仍负责黄白受伤/无敌闪烁。

// Day10 助手修正：行走/跑步锚点按躯干，不按摆动脚；跑步使用固定地面基线保留腾空起伏。
// 走跑共用locomotionPhase，Shift切换保留左右脚阶段；速度与碰撞仍由Player处理。

// Day10修正：待机/走跑/跳跃统一躯干横向参考；跳跃按躯干相对基准定位，不追随收起的脚。

// Day11 助手代改：统一三段躯干锚点；横劈/突刺各用独立帧时长表和伤害窗口，竖劈节奏保留。

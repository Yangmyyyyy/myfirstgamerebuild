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
    else if (!state.grounded) motion = state.velocityY < 0 ? Motion::Rise : Motion::Fall;
    else if (state.moving) motion = state.running ? Motion::Run : Motion::Walk;
    else if (landingTimeLeft > 0) motion = Motion::Land;

    if (motion != previousAnimation) { animationTime = 0.0f; previousAnimation = motion; }
    animationTime += deltaTime;
    animation = 0;
    if (motion == Motion::Walk) animation = 1;
    else if (motion == Motion::Run) animation = 2;
    else if (motion == Motion::Rise || motion == Motion::Fall || motion == Motion::Land) animation = 3;
    else if (motion == Motion::Dash) animation = 4;

    const auto& clip = HeroAnimationData::Clips[animation];
    frame = static_cast<int>(animationTime / clip.seconds) % clip.count;
    // 跳跃跟随实际物理阶段，不循环整段动画，也不再叠加预览页模拟的位移。
    if (motion == Motion::Rise) frame = std::min(3, 1 + static_cast<int>(animationTime / clip.seconds));
    else if (motion == Motion::Fall) frame = std::min(5, 4 + static_cast<int>(animationTime / clip.seconds));
    else if (motion == Motion::Land) frame = landingTimeLeft > 0.06f ? 6 : 7;
    else if (motion == Motion::Dash)
    {
        float progress = (GameConfig::DashDuration - state.dashTimeLeft) / GameConfig::DashDuration;
        frame = std::clamp(static_cast<int>(progress * clip.count), 0, clip.count - 1);
    }
    else if (motion == Motion::Dead) frame = 0; // 包内没有死亡动作，明确保留待机占位

    grounded = state.grounded;
    lookDirection = motion == Motion::Dash ? state.dashDirection : state.facingDirection;
}

void PlayerVisual::Draw(const Rectangle& player) const
{
    if (heroTextures[animation].id == 0) { DrawRectangleRec(player, tint); return; }
    const auto& clip = HeroAnimationData::Clips[animation];
    const auto& pose = clip.frames[frame];
    Rectangle source = pose.source;
    float width = source.width * clip.scale;
    float height = source.height * clip.scale;
    float xFromAnchor = (source.x - pose.anchor.x) * clip.scale;
    float centerX = player.x + player.width / 2;
    float footY = player.y + player.height + (grounded ? 3.0f : 0.0f);
    if (grounded)
        DrawEllipse(static_cast<int>(centerX), static_cast<int>(footY), 12, 2, Fade(BLACK, 0.16f));
    // 围绕脚底锚点翻转，剑伸得更远时也不会把身体挤到碰撞框另一侧。
    float x = centerX + xFromAnchor;
    if (lookDirection < 0) { source.width = -source.width; x = centerX - xFromAnchor - width; }
    if (paletteReady)
    {
        const auto& profile = HeroStyle::Profiles[animation];
        SetShaderValue(paletteShader, clothGainLocation, profile.cloth, SHADER_UNIFORM_VEC3);
        SetShaderValue(paletteShader, leatherGainLocation, profile.leather, SHADER_UNIFORM_VEC3);
        SetShaderValue(paletteShader, skinGainLocation, profile.skin, SHADER_UNIFORM_VEC3);
        float referenceMode = animation == 1 ? 1.0f : 0.0f; // 行走是原色基准
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
    landingTimeLeft = 0.0f;
    motion = previousAnimation = Motion::Idle;
    animation = frame = 0;
    grounded = false;
    lookDirection = 1.0f;
    tint = WHITE;
}

const char* PlayerVisual::GetMotionName() const
{
    const char* names[] = { "Idle", "Walk", "Run", "Dash", "Rise", "Fall", "Land", "Dead" };
    return names[static_cast<int>(motion)];
}

int PlayerVisual::GetFrame() const { return frame; }
int PlayerVisual::GetFrameCount() const { return HeroAnimationData::Clips[animation].count; }

// Day09 助手代做/待学习：使用用户素材包接入5组外观，原frames.json与攻击图完整归档。
// 仅换贴图及播放；Player/Combat/Enemy/Collision规则未改，攻击动画与刀光尚未接入。
// 死亡暂用待机首帧；预览页的offsetX/offsetY不作为游戏位移，避免跳跃冲刺叠加。
// 昨日课堂知识：Load准备纹理，Update选帧，Draw显示，Unload释放；const引用只读原数据。
// 三元表达式选择真假值；static_cast<int>截小数；%循环帧号或切换黄白闪烁。

// Day09统一外观：HeroStyle按行走配色校准，HeroAnimationData按站姿校准整体大小。
// 调色器只在Draw期间影响角色；所有PNG、动作帧、脚底锚点、碰撞与位移保持原样。

// Day09修正：旧待机的肩腰/四肢与行走不一致，现已换成按行走体型重绘的slim_v2图。
// 所有已启用动作共用行走色板；攻击修订稿仍仅归档，未改变战斗逻辑。

#include "CameraController.h"
#include "GameConfig.h"
#include <algorithm>
CameraController::CameraController()
{
    camera.offset = { GameConfig::ScreenWidth / 2.0f, GameConfig::ScreenHeight / 2.0f };
    camera.zoom = GameConfig::CameraZoom; // 放大显示，不修改角色尺寸、速度或碰撞。
}
void CameraController::Update(const Rectangle& player, float worldWidth)
{
    // 放大后，可见的世界范围变小；边界也必须除以缩放倍率。
    float halfWidth = GameConfig::ScreenWidth / (2.0f * camera.zoom);
    float halfHeight = GameConfig::ScreenHeight / (2.0f * camera.zoom);
    camera.target.x = std::clamp(player.x + player.width / 2, halfWidth, worldWidth - halfWidth);
    camera.target.y = std::clamp(player.y + player.height / 2, halfHeight, GameConfig::ScreenHeight - halfHeight);
}
Camera2D CameraController::GetCamera() const { return camera; }

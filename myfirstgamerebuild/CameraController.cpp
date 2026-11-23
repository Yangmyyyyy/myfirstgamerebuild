#include "CameraController.h"
#include "GameConfig.h"
CameraController::CameraController()
{
    camera.offset = { GameConfig::ScreenWidth / 2.0f, 0 };
    camera.zoom = 1.0f;
}
void CameraController::Update(const Rectangle& player, float worldWidth)
{
    camera.target.x = player.x + player.width / 2;
    float halfWidth = GameConfig::ScreenWidth / 2.0f;
    if (camera.target.x < halfWidth) camera.target.x = halfWidth;
    if (camera.target.x > worldWidth - halfWidth) camera.target.x = worldWidth - halfWidth;
}
Camera2D CameraController::GetCamera() const { return camera; }

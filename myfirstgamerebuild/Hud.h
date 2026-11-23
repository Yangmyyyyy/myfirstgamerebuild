#pragma once
class Player;
class Enemy;
class Combat;
class PlayerVisual;
// HUD没有需要长期保存的数据，用普通函数即可，不必每个文件都造一个class。
void DrawHud(const Player& player, const Enemy& enemy, const Combat& combat,
    const PlayerVisual& visual, bool showCollision);

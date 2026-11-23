#include "Enemy.h"
const Rectangle& Enemy::GetBounds() const { return body; }
int Enemy::GetHealth() const { return health; }
void Enemy::Draw() const { DrawRectangleRec(body, BLUE); }

#include "Object.h"

void Object::update()
{

}

void Player::move(float dx, float dy)
{
	position.x += dx * speed * dirX;
	position.y += dy * speed * dirY;
}

void Player::damaged(float damage)
{
	if (damage >= 0)
		hp -= damage;
}

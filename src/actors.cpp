#include "actors.h"


void Actor::Update(Timer &timer)
{
	previousPos = position;
	auto elapsed = timer.FrameElapsed();
	shootTimer += elapsed;
	position.x += velocity.x * elapsed;
	position.y += velocity.y * elapsed;
}


bool Actor::Shoot()
{
	if(shootTimer > shootDelay)
	{
		shootTimer = 0;
		return true;
	}
	return false;
}

bool Actor::Damage(int d)
{
	health -= d;
	return health <= 0;
}

void Actor::rollbackPos()
{
	auto dV = gamehelper::relativeVel(position, previousPos, 
		sqrtf(velocity.x * velocity.x + velocity.y * velocity.y));
	velocity = dV;
	position = previousPos;
}


void Player::Update(Timer &timer)
{
	if( velocity.x < 0 && acceleration.x > 0)
		acceleration.x = PLAYER_DECELERATION;
	if( velocity.x > 0 && acceleration.x < 0)
		acceleration.x = -PLAYER_DECELERATION;
	if( velocity.y < 0 && acceleration.y > 0)
		acceleration.y = PLAYER_DECELERATION;
	if( velocity.y > 0 && acceleration.y < 0)
		acceleration.y = -PLAYER_DECELERATION;
	
	velocity += acceleration;
	if(velocity.x < -MAX_SPEED)
		velocity.x = -MAX_SPEED;
	if(velocity.x > MAX_SPEED)
		velocity.x = MAX_SPEED;
	if(velocity.y < -MAX_SPEED)
		velocity.y = -MAX_SPEED;
	if(velocity.y > MAX_SPEED)
		velocity.y = MAX_SPEED;

	velocity.x *= FRICTION;
	velocity.y *= FRICTION;
	Actor::Update(timer);
}

void Player::Control(Btn &btn)
{
	acceleration = {0, 0};
	if(btn.press.Up())
		acceleration.y = -PLAYER_ACCELERATION;
	if(btn.press.Down())
		acceleration.y = PLAYER_ACCELERATION;
	if(btn.press.Right())
		acceleration.x = PLAYER_ACCELERATION;
	if(btn.press.Left())
		acceleration.x = -PLAYER_ACCELERATION;
}


void Bullet::Update(Timer &timer)
{
	lifetime += timer.FrameElapsed();
	if(lifetime > lifespan)
		health = 0;
	Actor::Update(timer);
}

void Bullet::rollbackPos()
{
	if(--wallHits <= 0)
		health = 0;
	Actor::rollbackPos();
}

void Bullet::setOwner(bool player)
{
	this->playerOwned = player;
	if(playerOwned)
	{
		colour = {1, 1, 1, 1};
		dim.x = 2;
		dim.y = 2;
	}
	else
	{
		colour = {1, 0, 0, 1};
		dim.x = 4;
		dim.y = 4;
	}
}

void Enemy::Update(Timer &timer)
{

	Actor::Update(timer);
}

void Enemy::Movement(glm::vec2 playerPos)
{
	
}
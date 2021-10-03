#include "actors.h"


void Actor::Update(Timer &timer)
{
	previousPos = position;
	auto elapsed = timer.FrameElapsed();
	shootTimer += elapsed;
	position.x += velocity.x * elapsed;
	position.y += velocity.y * elapsed;
}

void Actor::Draw(Render &render)
{
	if(!isRemoved())
	{
	float hpPercent = (float)health / (float)maxHealth;
	glm::vec4 colour = glm::vec4(1, 1, 1, 1); //white
	for(int i = 0 ; i < texture.dim.x; i++)
	{
		if(i > hpPercent * texture.dim.x)
			colour = glm::vec4(0.8, 0.2, 0.1, 1); //red

		render.DrawSquare(
			glm::vec4 (position.x + i, position.y + texture.dim.y + 2, 1, 2),
			0,
			colour,
			glm::vec4(0, 0, 1, 1), //texoffset
			texture.ID);
	}
	Sprite::Draw(render);
	}
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
	if(velocity.x < -maxSpeed)
		velocity.x = -maxSpeed;
	if(velocity.x > maxSpeed)
		velocity.x = maxSpeed;
	if(velocity.y < -maxSpeed)
		velocity.y = -maxSpeed;
	if(velocity.y > maxSpeed)
		velocity.y = maxSpeed;

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

void Player::Reset()
{
	bulletSpeed = INITIAL_BULLET_SPEED;
	shootDelay = INITIAL_SHOOT_DELAY;
	health = INITIAL_MAX_HEALTH;
	maxHealth = INITIAL_MAX_HEALTH;
	maxSpeed = INITIAL_MAX_SPEED;
}

void Player::Upgrade(Item::Type type)
{
	switch(type)
	{
	case Item::Type::HP:
		health++;
		if(health > maxHealth)
			maxHealth = health;
		break;
	case Item::Type::Shoot:
		shootDelay *= 0.98f;
		bulletSpeed += 0.025f;
		break;
	case Item::Type::Speed:
		maxSpeed += 0.05f;
		break;
	}
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
	if(collided)
		bounceTimer += timer.FrameElapsed();
	Actor::Update(timer);
}

void Enemy::Movement(glm::vec2 playerPos)
{
	velocity = gamehelper::relativeVel(position, playerPos, speed); 
	if(glm::distance(playerPos, position) < distanceToEnemy)
	{
		if(orbitRight)
			velocity = glm::vec2(velocity.y, -velocity.x);
		else
			velocity = glm::vec2(-velocity.y, velocity.x);
	}
	else if(bounceTimer < bounceDelay && collided)
	{
		if(orbitRight)
			velocity = glm::vec2(velocity.y, -velocity.x);
		else
			velocity = glm::vec2(-velocity.y, velocity.x);
	}
}

void Enemy::rollbackPos()
{
	collided = true;
	bounceTimer = 0;
	orbitRight = !orbitRight;
	Actor::rollbackPos();
}
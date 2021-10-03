#ifndef ACTORS_H
#define ACTORS_H

#include "sprite.h"
#include "framework/render.h"
#include "game/simpleInput.h"
#include "game/gamehelper.h"
#include "game/gameRandom.h"


#include <math.h>

class Actor : public Sprite
{
public:
	Actor(glm::vec2 pos, Tex texture) : Sprite(pos, texture) {}
	void Update(Timer &timer) override;
	void Draw(Render &render) override;
	bool Damage(int d);
	bool Shoot();
	float getBulletSpeed() { return bulletSpeed; }
	bool isRemoved() { return health <= 0; }
	virtual void rollbackPos();
	void setPosition(glm::vec2 pos) { position = pos; }
protected:
	glm::vec2 previousPos{0, 0};
	int health = 10;
	int maxHealth = 10;
	glm::vec2 velocity{0, 0};
	float shootDelay = 1000;
	float shootTimer = 400;
	float bulletSpeed = 0.05f;
};

class Player : public Actor
{
public:
	Player(glm::vec2 pos, Tex texture): Actor(pos, texture) { Reset(); }
	void Update(Timer &timer) override;
	void Control(Btn &btn);
	void Reset();
	void Upgrade(Item::Type type);
private:
	const float FRICTION = 0.9f;
	const float PLAYER_ACCELERATION = 0.01f;
	const float PLAYER_DECELERATION = 0.2f;
	const float INITIAL_MAX_SPEED = 0.1f;
	const float INITIAL_SHOOT_DELAY = 130;
	const float INITIAL_BULLET_SPEED = 0.25f;
	const float INITIAL_MAX_HEALTH = 4;
	float maxSpeed;
	glm::vec2 acceleration{0, 0};
};

class Enemy : public Actor
{
public:
	Enemy(glm::vec2 pos, Tex texture): Actor(pos, texture) {}
	void Update(Timer &timer) override;
	void Movement(glm::vec2 playerPos);
	void setDtoPlayer(float dist) { distanceToEnemy = dist; }
	void setFireDelay(float delay, float ratio) { shootDelay = delay; shootTimer = shootDelay * ratio; orbitRight = ratio > 0.5; }
	void setBulletSpeed(float speed) { bulletSpeed = speed; }
	void setHealth(int health) { this->health = health; maxHealth = health; }
	void rollbackPos() override;
	int bulletCount = 1;
private:
	float distanceToEnemy = 100.0f;
	float speed = 0.05f;
	bool orbitRight = true;
	bool collided = false;
	float bounceTimer = 400;
	float bounceDelay = 400;
};

class Bullet : public Actor
{
public:
	Bullet(glm::vec2 pos, Tex texture): Actor(pos, texture) { health = 1;}
	void Update(Timer &timer) override;
	void Draw(Render &render) override { Sprite::Draw(render); }
	void setVelocity(glm::vec2 vel) { velocity = vel; };
	void setOwner(bool player);
	bool isPlayer() { return playerOwned; }
	void rollbackPos() override;
protected:
	bool playerOwned = false;
	float lifespan = 10000;
	float lifetime = 0;
	int wallHits = 3;
};

class Particle : public Bullet
{
public:
	Particle(glm::vec2 pos, Tex texture): Bullet(pos, texture) 
	{ 
		health = 1;
		lifespan = 150;
	}
	void rollbackPos() override {};
	void Update(Timer &timer) override
	{
		auto ratio = (lifetime / lifespan);
		colour = {
			(ratio * endCol.x) + ((1 - ratio) * startCol.x),
			 (ratio * endCol.y) + ((1 - ratio) * startCol.y),
			  (ratio * endCol.z) + ((1 - ratio) * startCol.z), 1};
		Bullet::Update(timer);
	}
	void setLifespan(float lifespan) { this->lifespan = lifespan; }
	glm::vec3 startCol{0, 0, 0};
	glm::vec3 endCol{0.5, 0.5, 0.5};

};

#endif
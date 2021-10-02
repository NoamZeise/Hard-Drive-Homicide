#ifndef ACTORS_H
#define ACTORS_H

#include "sprite.h"
#include "game/simpleInput.h"
#include "game/gamehelper.h"


#include <math.h>

class Actor : public Sprite
{
public:
	Actor(glm::vec2 pos, Tex texture) : Sprite(pos, texture) {}
	void Update(Timer &timer) override;
	bool Damage(int d);
	bool Shoot();
	float getBulletSpeed() { return bulletSpeed; }
	bool isRemoved() { return health <= 0; }
	virtual void rollbackPos();
	void setPosition(glm::vec2 pos) { position = pos; }
protected:
	glm::vec2 previousPos{0, 0};
	int health = 15;
	glm::vec2 velocity{0, 0};
	float shootDelay = 700;
	float shootTimer = 0;
	float bulletSpeed = 0.15f;
};

class Player : public Actor
{
public:
	Player(glm::vec2 pos, Tex texture): Actor(pos, texture) 
	{bulletSpeed = 0.4f; shootDelay = 100; health = MAX_HP;}
	void Update(Timer &timer) override;
	void Control(Btn &btn);
	void Reset() { health = MAX_HP; }
private:
	const int MAX_HP = 3;
	const float FRICTION = 0.9f;
	const float PLAYER_ACCELERATION = 0.01f;
	const float PLAYER_DECELERATION = 0.2f;
	const float MAX_SPEED = 0.1f;
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
	void setHealth(int health) { this->health = health; }
	void rollbackPos() override;

private:
	float distanceToEnemy = 100.0f;
	float speed = 0.05f;
	bool orbitRight = true;
};

class Bullet : public Actor
{
public:
	Bullet(glm::vec2 pos, Tex texture): Actor(pos, texture) { health = 1;}
	void Update(Timer &timer) override;
	void setVelocity(glm::vec2 vel) { velocity = vel; };
	void setOwner(bool player);
	bool isPlayer() { return playerOwned; }
	void rollbackPos() override;
private:
	bool playerOwned = false;
	float lifespan = 10000;
	float lifetime = 0;
	int wallHits = 3;
};



#endif
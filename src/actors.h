#ifndef ACTORS_H
#define ACTORS_H

#include "sprite.h"
#include "game/simpleInput.h"

class Actor : public Sprite
{
public:
	Actor(glm::vec2 pos, Tex texture) : Sprite(pos, texture) {}
	void Update(Timer &timer) override;
protected:
	int health = 5;

};



#endif
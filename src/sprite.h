#ifndef SPRITE_H
#define SPRITE_H

#include "game/_glm.h"
#include "framework/typeStructs.h"
#include "framework/render.h"
#include "game/timer.h"

class Sprite
{
public:
	Sprite(glm::vec2 pos, Tex texture);
	glm::vec4 rectangle() { return glm::vec4(position.x, position.y, dim.x, dim.y);}
	virtual void Draw(Render &render);
	virtual void Update(Timer &timer) = 0;
	glm::vec2 getPos() { return position; }
protected:
	glm::vec2 position{0, 0};
	glm::vec4 colour{1, 1, 1, 1};
	glm::vec2 dim { 0, 0 };
	Tex texture;
};


class Item : public Sprite
{
public:
	enum Type
	{
		NoType,
		HP,
		Shoot,
		Speed,
	};

	Item(glm::vec2 pos, Tex texture, Type type) : Sprite(pos, texture) { this->type = type; }
	void Update(Timer &timer) override {}
	void setPos(glm::vec2 pos) { position = pos; }
	Type getType() { return type; }
	void setType(Type type) { this->type = type; }
private:
	Type type;
};

#endif

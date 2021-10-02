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
	glm::vec4 rectangle() { return glm::vec4(position.x, position.y, texture.dim.x, texture.dim.y);}
	void Draw(Render &render);
	virtual void Update(Timer &timer) = 0;
protected:
	glm::vec2 position{0, 0};
private:
	Tex texture;
};

#endif
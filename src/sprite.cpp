#include "sprite.h"


Sprite::Sprite(glm::vec2 pos, Tex texture)
{
	this->position = pos;
	this->texture = texture;
	dim.x = texture.dim.x;
	dim.y = texture.dim.y;
}

void Sprite::Draw(Render &render)
{
	render.DrawSquare(rectangle(), 0, colour, texture.ID);
}
#include "sprite.h"


Sprite::Sprite(glm::vec2 pos, Tex texture)
{
	this->position = pos;
	this->texture = texture;
}

void Sprite::Draw(Render &render)
{
	render.DrawSquare(rectangle(), 0, texture.ID);
}
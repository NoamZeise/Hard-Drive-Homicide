#ifndef GAME_HELPER_H
#define GAME_HELPER_H

#include "_glm.h"
#include <utility>

struct gamehelper
{
	static bool contains(glm::vec2 point, glm::vec4 rect)
	{
		return !(point.x < rect.x || point.y < rect.y || point.x > rect.x + rect.z || point.y > rect.y + rect.w);
	}

	static bool colliding(glm::vec4 rect1, glm::vec4 rect2)
	{
		return (rect1.x < rect2.x + rect2.z &&
			rect1.x + rect1.z > rect2.x &&
			rect1.y < rect2.y + rect2.w &&
			rect1.y + rect1.w > rect2.y);
	}

	static glm::vec2 relativeVel(glm::vec2 location, glm::vec2 destination, float speed)
	{
		glm::vec2 unitV(destination.x - location.x, destination.y  - location.y);
		unitV /= sqrt((unitV.x * unitV.x) + (unitV.y * unitV.y));
		return unitV * speed;
	}
};

#endif
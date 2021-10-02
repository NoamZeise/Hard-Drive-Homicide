#ifndef CAMERA_H
#define CAMERA_H

#include "_glm.h"
#include "timer.h"
#include "simpleInput.h"
#include "consts.h"

class Camera
{
public:
	Camera() {}
	Camera(glm::vec4 worldSize);
	void resize(glm::vec4 worldSize);
	glm::vec2 getCameraOffset() { return offset; }
	glm::vec4 getRect()
	{
		return glm::vec4(-offset.x, -offset.y, TARGET_WIDTH, TARGET_HEIGHT);
	}
	void target(glm::vec4 target);
private:
	unsigned int width, height;
	glm::vec4 worldSize;
	glm::vec2 offset = glm::vec2(0, 0);

	void correct();
};

#endif
#pragma once

#ifndef APP_H
#define APP_H

#ifndef GLFW_INCLUDE_VULKAN
#define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

#include <iostream>

#include "framework/render.h"
#include "framework/input.h"
#include "framework/audio.h"
#include "framework/vkhelper.h"
#include "game/_glm.h"
#include "game/timer.h"
#include "game/simpleInput.h"
#include "messageManager.h"
#include "game/camera.h"
#include "game/gamehelper.h"
#include "game/gameRandom.h"
#include "map.h"
#include "sprite.h"
#include "actors.h"

class App
{
public:
	App();
	~App();
	void loadAssets();
	void run();
	void update();
	void draw();
	void resize(int windowWidth, int windowHeight);

#pragma region GLFW_CALLBACKS
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
	static void error_callback(int error, const char* description);
#pragma endregion

	Input input;
private:
	void preUpdate();
	void postUpdate();
	void playerUpdate();
	void enemyUpdate();
	void bulletUpdate();
	void collisionUpdate();
	void AddBullet(Actor &actor, glm::vec2 destination, bool isPlayer);
	void nextLevel();
	glm::vec2 correctedPos(glm::vec2 pos);
	glm::vec2 correctedMouse();

	Timer timer;
	Random random;
	Btn btn;
	GLFWwindow* mWindow;
	Render* mRender;
	int mWindowWidth, mWindowHeight;
	MessageManager msgManager;
	Camera camera;
	Map map;
	const int STARTING_LEVEL_SIZE = 10;
	int levelSize = STARTING_LEVEL_SIZE;
	float level = 0;

	Player* player;
	std::vector<Enemy> enemies;
	Enemy* enemy;
	std::vector<Bullet> bullets;
	Bullet* bullet;
	std::vector<Sprite> drops;
	Item* hp;
	Item* upgrade;
};

#endif

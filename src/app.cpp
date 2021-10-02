#include "app.h"


App::App()
{
	//set member variables
	mWindowWidth = TARGET_WIDTH * 3;
	mWindowHeight = TARGET_HEIGHT * 3;
	//init glfw window
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
			throw std::runtime_error("failed to initialise glfw!");
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //using vulkan not openGL
	mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, "Vulkan App", nullptr, nullptr);
	if(!mWindow)
	{
		glfwTerminate();
		throw std::runtime_error("failed to create glfw window!");
	}
	if(FIXED_RATIO)
		glfwSetWindowAspectRatio(mWindow, TARGET_WIDTH, TARGET_HEIGHT);
	glfwSetWindowUserPointer(mWindow, this);
	glfwSetFramebufferSizeCallback(mWindow, framebuffer_size_callback);
	glfwSetCursorPosCallback(mWindow, mouse_callback);
	glfwSetScrollCallback(mWindow, scroll_callback);
	glfwSetKeyCallback(mWindow, key_callback);
	glfwSetMouseButtonCallback(mWindow, mouse_button_callback);
	mRender = new Render(mWindow, glm::vec2(TARGET_WIDTH, TARGET_HEIGHT));
	loadAssets();
}


App::~App()
{
	delete hp;
	delete upgrade;
	delete enemy;
	delete player;
	delete bullet;
	delete mRender;
	//cleanup glfw
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void App::loadAssets()
{
	//TODO load assets
	msgManager.LoadTextures(mRender);
	map = Map(*mRender);
	player = new Player(glm::vec2(100, 100), mRender->LoadTexture("textures/sprites/player.png"));
	bullet = new Bullet(glm::vec2(0, 0), mRender->LoadTexture("textures/sprites/bullet.png"));
	enemy = new Enemy(glm::vec2(0, 0), mRender->LoadTexture("textures/sprites/enemy.png"));
	hp = new Item(glm::vec2(0, 0),  mRender->LoadTexture("textures/sprites/hp.png"), Item::Type::HP);
	upgrade = new Item(glm::vec2(0, 0),  mRender->LoadTexture("textures/sprites/hp.png"), Item::Type::Upgrade);
	nextLevel();

	mRender->endTextureLoad();
}

void App::run()
{
	while (!glfwWindowShouldClose(mWindow))
	{
		preUpdate();
		update();
		postUpdate();
		draw();
	}
}

void App::resize(int windowWidth, int windowHeight)
{
	mWindowWidth = windowWidth;
	mWindowHeight = windowHeight;
	if(mRender != nullptr)
		mRender->framebufferResized = true;
}

void App::preUpdate()
{
	glfwPollEvents();
	btn.press.Update(input.Keys);
}

void App::update()
{
	if(enemies.size() == 0)
		nextLevel();
	if(player->isRemoved())
	{
		enemies.clear();
		player->Reset();
		level = 0;
		levelSize = STARTING_LEVEL_SIZE;
		nextLevel();
	}
	msgManager.Update(timer, btn);
	playerUpdate();
	enemyUpdate();
	bulletUpdate();
	collisionUpdate();
}

void App::postUpdate()
{
	camera.target(player->rectangle());
	mRender->setCameraOffset(camera.getCameraOffset());
	timer.Update();
	btn.prev = btn.press;
}

void App::draw()
{
	mRender->startDraw();

	//TODO draw app	
	auto pos = camera.getCameraOffset();
	map.Draw(*mRender, glm::vec4(-pos.x, -pos.y, TARGET_WIDTH, TARGET_HEIGHT));
	for(auto& b : bullets)
		b.Draw(*mRender);
	for(auto& e : enemies)
		e.Draw(*mRender);
	player->Draw(*mRender);
	msgManager.Draw();

	mRender->endDraw();
}

glm::vec2 App::correctedPos(glm::vec2 pos)
{
	return glm::vec2(pos.x * ((float)TARGET_WIDTH / (float)mWindowWidth), pos.y * ((float)TARGET_HEIGHT / (float)mWindowHeight));
}

glm::vec2 App::correctedMouse()
{
	auto offset = camera.getCameraOffset();
	auto correct = correctedPos(glm::vec2(input.X, input.Y));
	return {correct.x - offset.x, correct.y - offset.y};
}

void App::playerUpdate()
{
	player->Control(btn);
	player->Update(timer);
	if(input.Buttons[GLFW_MOUSE_BUTTON_LEFT])
	{
		if(player->Shoot())
		{
			AddBullet(*player, correctedMouse(), true);
		}
	}
}

void App::enemyUpdate()
{
	for(int i = 0; i < enemies.size(); i++)
	{
		enemies[i].Movement(player->getPos());
		enemies[i].Update(timer);
		if(enemies[i].Shoot())
		{
			AddBullet(enemies[i], player->getPos(), false);
		}
		if(enemies[i].isRemoved())
			enemies.erase(enemies.begin() + i--);
	}
}
void App::bulletUpdate()
{
	for(int i = 0; i < bullets.size(); i++)
	{
		bullets[i].Update(timer);
		if(bullets[i].isRemoved())
			bullets.erase(bullets.begin() + i--);
	}
}

void App::collisionUpdate()
{
	//bullet collision
	for(auto &b : bullets)
	{
		if(b.isPlayer())
		{
			for(auto &e : enemies)
			{
				if(gamehelper::colliding(b.rectangle(), e.rectangle()))
				{
					b.Damage(1);
					e.Damage(1);
					continue;
				}
			}
		}
		else if(gamehelper::colliding(b.rectangle(), player->rectangle()))
		{
				player->Damage(1);
				b.Damage(1);
		}
		
		if(map.inWall(b.rectangle()))
			b.rollbackPos();
	}

	//map collisions
	if(map.inWall(player->rectangle()))
		player->rollbackPos();
	for(auto &e : enemies)
		if(map.inWall(e.rectangle()))
			e.rollbackPos();
}


void App::AddBullet(Actor &actor, glm::vec2 destination, bool isPlayer)
{
	bullets.push_back(*bullet);
	bullets.back().setOwner(isPlayer);
	glm::vec4 rect = actor.rectangle();
	glm::vec2 bPos
		{rect.x + (rect.z / 2) - (bullets.back().rectangle().z / 2),
		 rect.y + (rect.w / 2) - (bullets.back().rectangle().w / 2)};
	bullets.back().setVelocity( gamehelper::relativeVel(bPos, destination, actor.getBulletSpeed()));
	bullets.back().setPosition(bPos);
}

void App::nextLevel()
{
	level++;
	levelSize += 3;
	int width = levelSize;
	int height = levelSize;
	map.genMap(width, height, level);
	camera.resize(glm::vec4(0, 0, width * TILE_WIDTH, height * TILE_HEIGHT));
	player->setPosition(map.playerSpawn);
	bullets.clear();
	for(auto& s : map.enemySpawns)
	{
		Enemy e = *enemy;
		e.setPosition(s);
		e.setDtoPlayer(100 + (random.Real() * 50));
		e.setFireDelay(700 + (random.Real() * 100), random.PositiveReal());
		enemies.push_back(e);
	}
}


#pragma region GLFW_CALLBACKS


void App::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	app->resize(width, height);
}

void App::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	app->input.X = xpos;
	app->input.Y = ypos;
}
void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	app->input.offset = yoffset;
}

void App::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
	if (key == GLFW_KEY_F && action == GLFW_RELEASE)
	{
		if (glfwGetWindowMonitor(window) == nullptr)
		{
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
		}
		else
		{
			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowMonitor(window, NULL, 100, 100, app->mWindowWidth, app->mWindowHeight, mode->refreshRate);
		}
	}
	if(key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			app->input.Keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			app->input.Keys[key] = false;
		}
	}
}

void App::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));

	if (button >= 0 && button < 8)
	{
		if (action == GLFW_PRESS)
		{
			app->input.Buttons[button] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			app->input.Buttons[button] = false;
		}
	}
}

void App::error_callback(int error, const char* description)
{
    throw std::runtime_error(description);
}

#pragma endregion

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
	mWindow = glfwCreateWindow(mWindowWidth, mWindowHeight, "Hard Drive Homocide", nullptr, nullptr);
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
	transitionUpdate();

	main = Audio("sound/main.mp3");
	main.loop();
	main.setVolume(0.2f);
}


App::~App()
{
	delete onePtcl;
	delete zeroPtcl;
	delete hp;
	delete speedU;
	delete shootU;
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
	shootU = new Item(glm::vec2(0, 0),  mRender->LoadTexture("textures/sprites/upgrade.png"), Item::Type::Shoot);
	speedU = new Item(glm::vec2(0, 0),  mRender->LoadTexture("textures/sprites/speed.png"), Item::Type::Shoot);
	transitionTex = mRender->LoadTexture("textures/transition.png");
	onePtcl = new Particle({0, 0}, mRender->LoadTexture("textures/particles/1.png"));
	zeroPtcl = new Particle({0, 0}, mRender->LoadTexture("textures/particles/0.png"));
	nextLevel();
	msgManager.AddMessage("out of space again? This stupid thing! Since when have I become a slave to my own hard drive?");
	msgManager.AddMessage("i've had it! I'm going in there and teaching it a lesson...");

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
	msgManager.Update(timer, btn);
	if(!msgManager.Active())
	{
		transitionUpdate();
		mapUpdate();
		playerUpdate();
		enemyUpdate();
		bulletUpdate();
		particleUpdate();
		collisionUpdate();
	}
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
	for(auto& i : drops)
		i.Draw(*mRender);
	for(auto& e : enemies)
		e.Draw(*mRender);
	player->Draw(*mRender);
	for(auto& p : particles)
		p.Draw(*mRender);

	auto offset = camera.getCameraOffset();
	if(transitionTimer < transitionDuration)
	{
		mRender->DrawSquare(glm::vec4(0 - offset.x, transitionPos - offset.y, transitionTex.dim.x, transitionTex.dim.y),
		0,
		transitionTex.ID);
	}
	msgManager.Draw(offset);

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

void App::transitionUpdate()
{
	transitionTimer += timer.FrameElapsed();
		if(transitionTimer < transitionDuration)
		{
			if(transitionTimer < transitionDuration / 2)
				transitionPos = -TARGET_HEIGHT * (1 - (transitionTimer / (transitionDuration / 2)));
			else
				transitionPos = -TARGET_HEIGHT * ((transitionTimer - transitionDuration / 2)  / (transitionDuration / 2));
		}
}

void App::mapUpdate()
{
	if(enemies.size() == 0)
	{
		if(toUpdate && transitionTimer > transitionDuration / 2)
		{
			toUpdate = false;
			nextLevel();
		}
		else if(!toUpdate)
		{
			main.oneTime("sound/nextlvl.mp3", 0.5f);
			transitionTimer = 0;
			toUpdate = true;
		}
	}
	if(player->isRemoved())
	{
		if(toUpdate && transitionTimer > transitionDuration / 2)
		{
			toUpdate = false;
			enemies.clear();
			player->Reset();
			msgManager.AddMessage("you got to level " + std::to_string((int)level));
			level = 0;
			levelSize = STARTING_LEVEL_SIZE;
			nextLevel();
			auto randNum = random.PositiveReal();
			if(randNum < 0.1)
				msgManager.AddMessage("A little setback like this won't stop me, not even close...");
			else if(randNum < 0.2)
				msgManager.AddMessage("Just think of all the homework you'll be able to download after this is done.");
			else if(randNum < 0.3)
				msgManager.AddMessage("I can do better, I'm sure of it! I'm sure! I'm sure! I'm sure. I'm surree...?");
			else if(randNum < 0.4)
				msgManager.AddMessage("Each file destroyed gives someone, somewhere a smile... probably");
			else if(randNum < 0.5)
				msgManager.AddMessage("I'll play it one day, and when I do, it's best if it's still installed!");
			else if(randNum < 0.6 && level > 5)
				msgManager.AddMessage("bite some bits to store a bit more bytes");
			else if(randNum < 0.7)
				msgManager.AddMessage("This is worth it for inner peace...");
			else if(randNum < 0.8)
				msgManager.AddMessage("The pain I have been caused by this little hunk of metal!");
			else if(randNum < 0.9)
				msgManager.AddMessage("Future me will be happy I did this...Future me will be happy I did this... **** future me!");
			else
				msgManager.AddMessage("You want to save that cat video, don't you?");
		}
		else if(!toUpdate)
		{
			main.oneTime("sound/playerDeath.mp3", 0.5f);
			transitionTimer = 0;
			toUpdate = true;
		}
	}
	dropTimer += timer.FrameElapsed();
	if(dropTimer > dropDelay)
	{
		dropTimer = 0;
		auto item = *shootU;
		auto val = random.PositiveReal();
		if(val > 0.7)
			item = *hp;
		else if(val > 0.4)
			item = *speedU;
		
		while(true)
		{
			glm::vec2 loc = glm::vec2( (TILE_WIDTH * 2) +
				random.PositiveReal() * ((map.getWidth() * TILE_WIDTH) - (TILE_WIDTH * 2))
				 ,
				(TILE_HEIGHT * 2) + 
				random.PositiveReal() *((map.getHeight() * TILE_HEIGHT) - (TILE_HEIGHT * 2)));
			if(!map.inWall(glm::vec4(loc.x, loc.y, item.rectangle().z, item.rectangle().w)))
			{
				item.setPos(loc);
				break;
			}
		}
		drops.push_back(item);
	}
}

void App::playerUpdate()
{
	player->Control(btn);
	player->Update(timer);
	if(transitionTimer > transitionDuration)
	if(input.Buttons[GLFW_MOUSE_BUTTON_LEFT])
	{
		if(player->Shoot())
		{
			main.oneTime("sound/playerShoot.mp3", 0.2f);
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
		if(transitionTimer > transitionDuration)
		if(enemies[i].Shoot())
		{
			main.oneTime("sound/enemyShoot.mp3", 0.2f);
			for(int b = 0; b < enemies[i].bulletCount; b++)
			{
				if(b == 0)
					AddBullet(enemies[i], player->getPos(), false);
				else
				{
					AddBullet(enemies[i], glm::vec2(
						random.PositiveReal() * map.getWidth() * TILE_WIDTH,
						random.PositiveReal() * map.getHeight() * TILE_HEIGHT	
					), false);
				}
			}
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

 void App::particleUpdate()
 {
	 for(int i = 0; i < particles.size(); i++)
	{
		particles[i].Update(timer);
		if(particles[i].isRemoved())
			particles.erase(particles.begin() + i);
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
				if(gamehelper::colliding(b.rectangle(), e.rectangle()) && !b.isRemoved())
				{
					b.Damage(1);
					e.Damage(1);
					Emit(b.getPos());
					Emit(b.getPos());
					Emit(b.getPos());
					main.oneTime("sound/enemyDmg.mp3", 0.9f);
					if(e.isRemoved())
					{
						main.oneTime("sound/enemyDeath.mp3", 0.3f);
						for (int i = 0; i < 20; i++)
							Emit(b.getPos());
					}
					
				}
			}
		}
		else if(gamehelper::colliding(b.rectangle(), player->rectangle())&& !b.isRemoved())
		{
				player->Damage(1);
				b.Damage(1);
				for (int i = 0; i < 20; i++)
							Emit(b.getPos());
				main.oneTime("sound/playerDmg.mp3", 0.5f);
		}
		
		if(map.inWall(b.rectangle()))
		{
			b.rollbackPos();
			Emit(b.getPos());
		}
	}

	//map collisions
	if(map.inWall(player->rectangle()))
		player->rollbackPos();
	for(auto &e : enemies)
		if(map.inWall(e.rectangle()))
			e.rollbackPos();
	
	//item-player collision
	for(int i = 0; i < drops.size(); i++)
		if(gamehelper::colliding(drops[i].rectangle(), player->rectangle()))
		{
			if(drops[i].getType() == Item::Type::HP)
				main.oneTime("sound/health.mp3", 0.5f);
			else
				main.oneTime("sound/upgrade.mp3", 0.5f);
			player->Upgrade(drops[i].getType());
			drops.erase(drops.begin() + i--);
		}

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

void App::Emit(glm::vec2 pos)
{
	auto p = *onePtcl;
	if(random.PositiveReal() > 0.5)
		p = *zeroPtcl;
	p.setPosition(pos);
	p.setVelocity( gamehelper::relativeVel(pos, 
		glm::vec2(pos.x + random.Real(), pos.y + random.Real()), //random dir
		random.PositiveReal() * 0.1) //speed
	);
	particles.push_back(p);
}

void App::nextLevel()
{
	level++;
	levelSize += 3;
	int width = levelSize + (random.PositiveReal() * (levelSize / 4));
	int height = levelSize + (random.PositiveReal() * (levelSize / 4));
	map.genMap(width, height, level);
	camera.resize(glm::vec4(0, 0, width * TILE_WIDTH, height * TILE_HEIGHT));
	player->setPosition(map.playerSpawn);
	bullets.clear();
	for(auto &i : drops)
		player->Upgrade(i.getType());
	drops.clear();
	for(auto& s : map.enemySpawns)
	{
		Enemy e = *enemy;
		e.setPosition(s);
		e.setDtoPlayer(100 + (random.Real() * 50));
		e.setFireDelay((1000 - (level * 10)) + (random.Real() * 100), random.PositiveReal());
		e.setBulletSpeed(0.05 + (level * 0.025) + random.Real() * 0.005);
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

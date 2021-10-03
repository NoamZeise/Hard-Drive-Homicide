#ifndef MESSAGE_MANAGER_H
#define MESSAGE_MANAGER_H

#include <vector>
#include <string>
#include <iostream>
#include <stdexcept>

#include "framework/render.h"
#include "game/_glm.h"
#include "game/timer.h"
#include "game/simpleInput.h"
#include "game/consts.h"

struct Message
{
	std::vector<std::string> line; 
};


class MessageManager
{
public:
	MessageManager() {}
	~MessageManager();
	void LoadTextures(Render *render);
	void Draw(glm::vec2 offset);
	void Update(Timer &timer, Btn &btn);
	void AddMessage(std::string text);
	bool Active() { return messages.size() > 0;}

private:
	Tex boxTex;
	TexFont* font = nullptr;
	Render* render = nullptr;
	const glm::vec2 MSG_BOX_OFFSET = glm::vec2(30, 60);
	const float MSG_BOX_WIDTH = 190;
	const int TEXT_SIZE = 7;
	const int LINE_SPACING = 10;
	std::vector<Message> messages;
};


#endif
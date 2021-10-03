#include "messageManager.h"

MessageManager::~MessageManager()
{
	render = nullptr;

}

void MessageManager::LoadTextures(Render *render)
{
	this->render = render;
	boxTex = render->LoadTexture("textures/messageBox.png");
	font = render->LoadFont("textures/dogicapixel.otf");
}

void MessageManager::Update(Timer &timer, Btn &btn)
{
	if(btn.ABXY() && messages.size() > 0)
	{
		messages.erase(messages.begin());
	}
		
}

void MessageManager::Draw(glm::vec2 offset)
{
	if(render == nullptr)
		throw std::runtime_error("Message Manager: can't draw before LoadTextures has been called");
	if(messages.size() > 0)
	{
		for(int i = 0; i < messages[0].line.size(); i++)
		{
			if(i == 0)
				render->DrawSquare(glm::vec4(0 - offset.x, 0 - offset.y, boxTex.dim.x, boxTex.dim.y), 0, boxTex.ID);
			render->DrawString(font, messages[0].line[i], 
				glm::vec2(MSG_BOX_OFFSET.x - offset.x, (TARGET_HEIGHT - MSG_BOX_OFFSET.y) + ( i * LINE_SPACING) - offset.y),
			 	TEXT_SIZE, 0, glm::vec4(1, 1, 1, 1));
		}
	}
}

void MessageManager::AddMessage(std::string text)
{
	if(render == nullptr)
		throw std::runtime_error("Message Manager: can't add message before LoadTextures has been called");
	std::string msgLine = "";
	std::string lastWord = "";
	messages.push_back({});
	for(int i = 0; i < text.length(); i++ )
	{
		if(text[i] == ' ')
		{
			if(render->MeasureString(font, msgLine + lastWord, TEXT_SIZE) > MSG_BOX_WIDTH)
			{
				messages.back().line.push_back(msgLine);
				msgLine = lastWord.append(" ");
			}
			else
			{
				msgLine = msgLine.append(lastWord.append(" "));
			}
			lastWord = "";
		}
		else
		{
			lastWord += text[i];
		}
	}
	messages.back().line.push_back(msgLine.append(lastWord));
}
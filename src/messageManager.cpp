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
	if(btn.B())
		currentMsg = {};
}

void MessageManager::Draw()
{
	if(render == nullptr)
		throw std::runtime_error("Message Manager: can't draw before LoadTextures has been called");
	for(int i = 0; i < currentMsg.line.size(); i++)
	{
		if(i == 0)
			render->DrawSquare(glm::vec4(0, 0, boxTex.dim.x, boxTex.dim.y), 0, boxTex.ID);
		render->DrawString(font, currentMsg.line[i], 
			glm::vec2(MSG_BOX_OFFSET.x, (TARGET_HEIGHT - MSG_BOX_OFFSET.y) + ( i * LINE_SPACING)),
		 	TEXT_SIZE, 0, glm::vec4(1, 1, 1, 1));
	}
}

void MessageManager::AddMessage(std::string text)
{
	if(render == nullptr)
		throw std::runtime_error("Message Manager: can't add message before LoadTextures has been called");
	std::string msgLine = "";
	std::string lastWord = "";
	currentMsg.line.clear();
	for(int i = 0; i < text.length(); i++ )
	{
		if(text[i] == ' ')
		{
			if(render->MeasureString(font, msgLine + lastWord, TEXT_SIZE) > MSG_BOX_WIDTH)
			{
				currentMsg.line.push_back(msgLine);
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
	currentMsg.line.push_back(msgLine.append(lastWord));
}
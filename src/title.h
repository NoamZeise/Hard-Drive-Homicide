#pragma once
#ifndef TITLE_H
#define TITLE_H

#include "framework/render.h"
#include "game/timer.h"
#include "game/simpleInput.h"
#include "game/_glm.h"
#include "game/consts.h"

#include <vector>
#include <map>


class Title
{
public:
	enum class Menu
	{
		NoChoice,
		Play,
		Endless,
		Exit
	};
	Title(Render& render)
	{
		menuTex.insert(std::pair<Menu, Tex>(Menu::Play,  render.LoadTexture("textures/title/play.png")));
		menuTex.insert(std::pair<Menu, Tex>(Menu::Endless, render.LoadTexture("textures/title/endless.png")));
		menuTex.insert(std::pair<Menu, Tex>(Menu::Exit, render.LoadTexture("textures/title/exit.png")));
		selectedTex = render.LoadTexture("textures/title/selected.png");
		bgTex = render.LoadTexture("textures/title/screen.png");
	}
	Menu getChoice()
	{
		return choice;
	}

	void Update(Btn& btn)
	{
		if(active)
		{
			if (btn.Up())
			{
				selected--;
				if (selected < 0)  selected = max;
			}
			if (btn.Down())
			{
				selected++;
				if (selected > max)  selected = 0;
			}
			if (btn.ABXY())
			{
				choice = tm[selected];
			}
		}
	}

	void Draw(Render& render, glm::vec2 offset)
	{
		render.DrawSquare(glm::vec4(-offset.x, -offset.y, bgTex.dim.x, bgTex.dim.y), 0, bgTex.ID);
		for (int i = 0; i < tm.size(); i++)
		{
			render.DrawSquare(glm::vec4(TITLE_BEGIN.x - offset.x, TITLE_BEGIN.y + i * (TITLE_DIM.y + 5) - offset.y, TITLE_DIM.x, TITLE_DIM.y), 0,
				menuTex[tm[i]].ID);
			if (i == selected)
			{
				render.DrawSquare(glm::vec4(TITLE_BEGIN.x - ((CHOOSE_DIM.x - TITLE_DIM.x) / 2) - offset.x
					, TITLE_BEGIN.y + i * (TITLE_DIM.y + 2) - ((CHOOSE_DIM.y - TITLE_DIM.y)/ 2) - offset.y
					, CHOOSE_DIM.x, CHOOSE_DIM.y), 0,
					selectedTex.ID);
			}
		}
	}

	void Reset()
	{
		selected = 0;
		choice = Menu::NoChoice;
		active = true;
	}
	bool isActive() { return active; }
	void disable()
	{
		active = false;
	}

private:

	bool active = true;
	const glm::vec2 TITLE_BEGIN = glm::vec2((TARGET_WIDTH / 2) - (129 / 2), TARGET_HEIGHT - 110);
	const glm::vec2 TITLE_DIM = glm::vec2(129, 33);
	std::vector<Menu> tm = { Menu::Play, Menu::Endless, Menu::Exit};
	std::map<Menu, Tex> menuTex;
	Tex selectedTex;
	const glm::vec2 CHOOSE_DIM = glm::vec2(140, 44);
	Tex bgTex;
	Tex controlsTex;

	int selected = 0;
	int max = 3;
	Menu choice = Menu::NoChoice;
};


#endif

#ifndef MAP_H
#define MAP_H

#include <vector>
#include <map>
#include <iostream>

#include "framework/render.h"
#include "game/consts.h"
#include "game/gameRandom.h"
#include "game/gamehelper.h"

enum class LogicalTile
{
	None,
	Ground,
	Wall
};

enum class TextureTile
{
	None,
	Ground,
	Wall,
	FarWall,
	TopG,
	BotG,
	LeftG,
	RightG,
	TLG,
	TRG,
	BRG,
	BLG,
	Obstacle,
	Obstacle2,
	Obstacle3,
};

class Map
{
public:
	Map() {};
	Map(Render &render);
	bool isActive() { return map.size() > 0;};
	void genMap(int width, int height, float difficulty);
	bool inWall(glm::vec4 rect);
	void Draw(Render &render, glm::vec4 cameraRect);
	glm::vec2 playerSpawn{0, 0};
	std::vector<glm::vec2> enemySpawns;
	int getWidth() { return width; }
	int getHeight() { return height; }
private:
	Random random;

	struct Location { int x = 0; int y = 0;};
	int width = 0;
	int height = 0;
	std::map<TextureTile, Tex> textures;
	std::vector<TextureTile> map;

	LogicalTile logicalFromTex(TextureTile tile);
	TextureTile getTile(Location location);
	void setTile(Location location, TextureTile tile);
	bool Map::valid(Location location);
	glm::vec4 getTileRect(Location location);
};




#endif
#ifndef MAP_H
#define MAP_H

#include <vector>
#include <map>

#include "framework/render.h"
#include "game/consts.h"

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
	Wall
};

class Map
{
public:
	Map() {};
	Map(Render &render);
	bool isActive() { return map.size() > 0;};
	void genMap(int width, int height);
	bool inWall(glm::vec4 rect);
	void Draw(Render &render);
	void CompleteMap();
private:
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
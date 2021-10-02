#include "map.h"

Map::Map(Render &render)
{
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::Wall, render.LoadTexture("textures/tiles/wall.png")));
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::Ground, render.LoadTexture("textures/tiles/ground.png")));
}

void Map::genMap(int width, int height)
{
	this->width = width;
	this->height = height;
	map.resize(width * height);
	for(int y = 0; y < height; y++)
		for(int x = 0; x < width; x++)
		{
			if(x == 0 || x == width - 1 || y == 0 || y == height - 1)
				setTile(Location{x, y}, TextureTile::Wall);
			else
				setTile(Location{x, y}, TextureTile::Ground);
		}
}

void Map::Draw(Render &render)
{
	for(int y = 0; y < height; y++)
		for(int x = 0; x < width; x++)
		{
			render.DrawSquare(
				getTileRect(Location{x, y}),
				0, 
				textures[getTile(Location{x, y})].ID
			);
		}
}

void Map::CompleteMap()
{
	map.clear();
	width = 0;
	height = 0;
}

bool Map::inWall(glm::vec4 rect)
{
	//get location of each point of rect
	Location 
		tl{(int)(rect.x / TILE_WIDTH), 			  (int)(rect.y / TILE_WIDTH)},
		tr{(int)((rect.x + rect.z) / TILE_WIDTH), (int)(rect.y / TILE_WIDTH)},
		bl{(int)(rect.x / TILE_WIDTH),			  (int)((rect.y + rect.w) / TILE_WIDTH)},
		br{(int)((rect.x + rect.z) / TILE_WIDTH), (int)((rect.y + rect.w) / TILE_WIDTH)};

	//check if any of the corners are in a wall tile
	return (logicalFromTex(getTile(tl)) == LogicalTile::Wall
	|| logicalFromTex(getTile(tr)) == LogicalTile::Wall
	|| logicalFromTex(getTile(bl)) == LogicalTile::Wall
	|| logicalFromTex(getTile(br)) == LogicalTile::Wall);
}


LogicalTile Map::logicalFromTex(TextureTile tile)
{
	switch(tile)
	{
		case TextureTile::None:
			return LogicalTile::None;

		case TextureTile::Ground:
			return LogicalTile::Ground;

		case TextureTile::Wall:
			return LogicalTile::Wall;
	}
}

TextureTile Map::getTile(Location location)
{
	if (!valid(location)) 
		return TextureTile::None;
	return map[(location.y * width) + location.x];
}

void Map::setTile(Location location, TextureTile tile)
{
	if (!valid(location))
		return;
	map[(location.y * width) + location.x] = tile;
}

glm::vec4 Map::getTileRect(Location location)
{
	return glm::vec4(location.x * TILE_WIDTH, location.y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT);
}

bool Map::valid(Location location)
{
	return !(location.x > width || location.x < 0 || location.y  > height || location.y < 0);
}


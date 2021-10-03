#include "map.h"

Map::Map(Render &render)
{
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::Wall, render.LoadTexture("textures/tiles/wall.png")));
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::FarWall, render.LoadTexture("textures/tiles/furtherwall.png")));
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::Obstacle, render.LoadTexture("textures/tiles/obstacle.png")));
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::Obstacle2, render.LoadTexture("textures/tiles/obstacle2.png")));
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::Obstacle3, render.LoadTexture("textures/tiles/obstacle3.png")));
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::Ground, render.LoadTexture("textures/tiles/ground.png")));
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::LeftG, render.LoadTexture("textures/tiles/leftwalll.png")));
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::RightG, render.LoadTexture("textures/tiles/rightwalll.png")));
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::TRG, render.LoadTexture("textures/tiles/trwall.png")));
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::TLG, render.LoadTexture("textures/tiles/tlwall.png")));
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::TopG, render.LoadTexture("textures/tiles/topwall.png")));
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::BotG, render.LoadTexture("textures/tiles/botwall.png")));
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::BLG, render.LoadTexture("textures/tiles/blwall.png")));
	textures.insert(
		std::pair<TextureTile, Tex>(TextureTile::BRG, render.LoadTexture("textures/tiles/brwall.png")));
}

void Map::genMap(int width, int height, float difficulty)
{
	map.clear();
	enemySpawns.clear();
	playerSpawn = {0, 0};
	//int enemyCount = difficulty * 5;
	this->width = width;
	this->height = height;
	map.resize(width * height);
	for(int y = 0; y < height; y++)
		for(int x = 0; x < width; x++)
		{
			if(x == 0 || x == width - 1 || y == 0 || y == height - 1)
				setTile(Location{x, y}, TextureTile::FarWall);
			else if(x == 1 || x == width - 2 || y == 1 || y == height - 2)
				setTile(Location{x, y}, TextureTile::Wall);
			else if(x == 2 && y == 2)
				setTile(Location{x, y}, TextureTile::TLG);
			else if(x == width - 3 && y == 2)
				setTile(Location{x, y}, TextureTile::TRG);
			else if(x == 2 && y == height - 3)
				setTile(Location{x, y}, TextureTile::BLG);
			else if(x == width - 3 && y == height - 3)
				setTile(Location{x, y}, TextureTile::BRG);
			else if(y == 2)
				setTile(Location{x, y}, TextureTile::TopG);
			else if(x == 2)
				setTile(Location{x, y}, TextureTile::LeftG);
			else if(x == width - 3)
				setTile(Location{x, y}, TextureTile::RightG);
			else if(y == height - 3)
				setTile(Location{x, y}, TextureTile::BotG);
			else
			{
				setTile(Location{x, y}, TextureTile::Ground);
			}
		}
	int obsCount = ((width * height) / 40) + (difficulty * 3);
	while(obsCount > 0)
	{
		glm::vec2 loc = glm::vec2(
			random.PositiveReal() * width * TILE_WIDTH,
			random.PositiveReal() * height * TILE_HEIGHT);
		if(!inWall(glm::vec4(loc.x, loc.y, 4, 4)))
		{
			auto num = random.PositiveReal();
			if(num > 0.7)
				setTile(Location{(int)(loc.x / TILE_WIDTH), (int)(loc.y / TILE_HEIGHT)}, TextureTile::Obstacle);
			else if(num > 0.4)
				setTile(Location{(int)(loc.x / TILE_WIDTH), (int)(loc.y / TILE_HEIGHT)}, TextureTile::Obstacle2);
			else
				setTile(Location{(int)(loc.x / TILE_WIDTH), (int)(loc.y / TILE_HEIGHT)}, TextureTile::Obstacle3);
			obsCount--;
		}
	}
	while(true)
	{
		glm::vec2 loc = glm::vec2(
			random.PositiveReal() * width * TILE_WIDTH,
			random.PositiveReal() * height * TILE_HEIGHT);
		if(!inWall(glm::vec4(loc.x, loc.y, 7, 7)))
		{
			playerSpawn = loc;
			break;
		}
	}
	int enemyCount = 1 + (difficulty * 0.6);
	while(enemyCount > 0)
	{
		glm::vec2 loc = glm::vec2(
			random.PositiveReal() * width * TILE_WIDTH ,
			random.PositiveReal() * height * TILE_HEIGHT);
		if(!inWall(glm::vec4(loc.x, loc.y, 9, 9)) && !glm::distance(playerSpawn, loc) < 100)
		{
			enemySpawns.push_back(loc);
			enemyCount--;
		}
	}
}

void Map::Draw(Render &render, glm::vec4 cameraRect)
{
	for(int y = 0; y < height; y++)
		for(int x = 0; x < width; x++)
		{
			glm::vec4 tileRect = getTileRect(Location{x, y});
			if(gamehelper::colliding(cameraRect, tileRect))
				render.DrawSquare( tileRect, 0, textures[getTile(Location{x, y})].ID);
		}
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
		case TextureTile::LeftG:
		case TextureTile::RightG:
		case TextureTile::TopG:
		case TextureTile::BotG:
		case TextureTile::TLG:
		case TextureTile::TRG:
		case TextureTile::BLG:
		case TextureTile::BRG:
			return LogicalTile::Ground;

		case TextureTile::Wall:
		case TextureTile::FarWall:
		case TextureTile::Obstacle:
		case TextureTile::Obstacle2:
		case TextureTile::Obstacle3:
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


#ifndef __MAP_H__
#define __MAP_H__

#include "Core/Module.h"
#include "Core/Pathfinding.h"
#include "Utils/List.h"
#include "Utils/Point.h"

#include <pugixml.hpp>
#include <SDL.h>

#include "Utils/Defs.h"
#include "Utils/Log.h"
#include <unordered_map>
#include <variant>

enum MapOrientation
{
	ORTOGRAPHIC = 0,
	ISOMETRIC
};

class Map;

// Ignore Terrain Types and Tile Types for now, but we want the image!
struct TileSet
{
	SString	name;
	int	firstgid;
	int margin;
	int	spacing;
	int	tileWidth;
	int	tileHeight;
	int columns;
	int tilecount;

	std::unordered_map<int, int> tilePivotY;

	SDL_Texture* texture;
	SDL_Rect GetTileRect(int gid) const;
};

struct Colliders
{
	int x;
	int y;
	int width;
	int height;
};

//  We create an enum for map type, just for convenience,
// NOTE: Platformer game will be of type ORTHOGONAL
enum MapTypes
{
	MAPTYPE_UNKNOWN = 0,
	MAPTYPE_ORTHOGONAL,
	MAPTYPE_ISOMETRIC,
	MAPTYPE_STAGGERED
};

struct Properties
{
	struct Property
	{
		SString name;
		std::variant<bool, int> value; // We can add more types if needed
	};

	~Properties()
	{
		//...
		ListItem<Property*>* item;
		item = list.start;

		while (item != NULL)
		{
			RELEASE(item->data);
			item = item->next;
		}

		list.Clear();
	}

	Property* GetProperty(const char* name);

	List<Property*> list;
};

struct MapLayer
{
	SString	name;
	int id;
	int width;
	int height;
	float parallaxFactor;
	uint* data;

	Properties properties;

	MapLayer() : data(NULL)
	{}

	~MapLayer()
	{
		RELEASE(data);
	}

	inline uint Get(int x, int y) const
	{
		return data[(y * width) + x];
	}
};

struct MapData
{
	int width;
	int	height;
	int	tileWidth;
	int	tileHeight;
	List<TileSet*> tilesets;
	MapTypes type;

	List<MapLayer*> maplayers;
};

class Map : public Module {

public:

	Map();

	Map(bool startEnabled);

	// Destructor
	virtual ~Map();

	// Called before render is available
	bool Awake(pugi::xml_node& conf);

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool Update(float dt);

	// Called before quitting
	bool CleanUp();

	// Load new map
	bool Load(SString mapFileName);

	iPoint MapToWorld(int x, int y) const;
	iPoint WorldToMap(int x, int y);

	// L13: Create navigation map for pathfinding
    void CreateNavigationMap(int& width, int& height, uchar** buffer) const;

private:

	// clipping margin to prevent seeing the tiles generating in the border of the screen
	const int clippingMargin = 2;

	bool LoadMap(pugi::xml_node mapFile);
	bool LoadTileSet(pugi::xml_node mapFile);
	bool LoadLayer(pugi::xml_node& node, MapLayer* layer);
	bool LoadAllLayers(pugi::xml_node mapNode);
	bool LoadColliders(pugi::xml_node mapFile);
	bool LoadLights(pugi::xml_node mapFile);
	TileSet* GetTilesetFromTileId(int gid) const;
	bool LoadProperties(pugi::xml_node& node, Properties& properties);

public:

	MapData mapData;
	SString name;
	SString path;

	uchar* navigationMap;

private:
	bool mapLoaded;
	MapLayer* navigationLayer;
	int blockedGid = 287; //!!!! make sure that you assign blockedGid according to your map
};

#endif // __MAP_H__
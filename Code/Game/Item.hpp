#ifndef include_Item
#define include_Item
#pragma once

//-----------------------------------------------------------------------------------------------
#include "Entity.hpp"
#include "ItemBlueprint.hpp"


//-----------------------------------------------------------------------------------------------
class Item : public Entity
{
public:
	Item();
	Item( const std::string& itemBlueprintName );
	ItemBlueprint* GetBlueprint();
	Item* GetSpawnedItem();
	void Update( float deltaSeconds );

	ItemBlueprint*					m_blueprint;
	std::vector< HexTileCoords >	m_movementPath;
};


#endif // include_Item
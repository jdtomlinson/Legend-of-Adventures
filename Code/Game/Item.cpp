#include "Item.hpp"


//-----------------------------------------------------------------------------------------------
Item::Item()
{
	m_blueprint = nullptr;
}


//-----------------------------------------------------------------------------------------------
Item::Item( const std::string& itemBlueprintName )
{
	EntityBlueprint* blueprint = EntityBlueprint::GetEntityBlueprintByName( itemBlueprintName );
	m_blueprint = static_cast< ItemBlueprint* >( blueprint );
}


//-----------------------------------------------------------------------------------------------
ItemBlueprint* Item::GetBlueprint()
{
	return m_blueprint;
}


//-----------------------------------------------------------------------------------------------
Item* Item::GetSpawnedItem()
{
	if( m_blueprint == nullptr )
		return nullptr;

	Item* spawnItem = new Item( m_blueprint->m_nameItemTypeToSpawn );
	if( spawnItem->m_blueprint == nullptr )
	{
		delete spawnItem;
		spawnItem = nullptr;
		return nullptr;
	}

	return spawnItem;
}


//-----------------------------------------------------------------------------------------------
void Item::Update( float )
{

}
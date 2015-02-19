#ifndef include_Actor
#define include_Actor
#pragma once

//-----------------------------------------------------------------------------------------------
#include "Item.hpp"
#include "Entity.hpp"
#include "ActorBlueprint.hpp"
#include "../Engine/Color.hpp"


//-----------------------------------------------------------------------------------------------
const float ATTACK_STAT_INCREASE_PER_LEVEL = 1.5f;
const float DEFENSE_STAT_INCREASE_PER_LEVEL = 1.f;
const Color HEALTH_BAR_BACKGROUND_COLOR = Color::Black;
const Color HEALTH_BAR_COLOR = Color::Red;


//-----------------------------------------------------------------------------------------------
class Actor : public Entity
{
public:
	Actor();
	Actor( const std::string& actorBlueprintName );
	~Actor();
	ActorBlueprint* GetBlueprint();
	void AddItemToInventory( Item* itemToAdd );
	Item* DropAndReturnItem( const std::string& itemTypeName );
	Item* GetAndUnequipItemOfType( const ItemBlueprint* itemType );
	bool CheckIfPlayerHasEquippedItemOfType( const ItemBlueprint* itemType );
	void SetRightHandEquippedItemToNextItem();
	void SetLeftHandEquippedItemToNextItem();
	void SetBodyEquippedItemToNextItem();
	attitudeTowardPlayer GetCurrentAttitude();
	void AddHealth( float amountToAdd );
	void RemoveHealth( float amountToRemove );
	void Update( float deltaSeconds );
	void Render();

	ActorBlueprint*			m_blueprint;
	int						m_currentLevel;
	float					m_currentHealth;
	float					m_currentExperience;
	float					m_secondsSinceLastMove;
	float					m_secondsSinceLastHealthRegen;
	HexTileCoords			m_coordMovingTo;
	HexTileCoords			m_previousPosition;
	Item*					m_rightHandEquippedItem;
	Item*					m_leftHandEquippedItem;
	Item*					m_bodyEquippedItem;
	std::vector< Item* >	m_inventory;

private:
	void RenderHealthBar();
};


#endif // include_Actor
#ifndef include_EntityBlueprint
#define include_EntityBlueprint
#pragma once

//-----------------------------------------------------------------------------------------------
#include <map>
#include <string>
#include "../Engine/Texture.hpp"
#include "../Engine/Vector2.hpp"
#include "../Engine/XMLParsingFunctions.hpp"


//-----------------------------------------------------------------------------------------------
enum statTypes
{
	STAT_ATTACK,
	STAT_DEFENSE,
	STAT_DEXTERITY,
	STAT_DODGE,
	STAT_SPEED,
	STAT_HEALTH_REGEN,
	STAT_SIGHT,
};


//-----------------------------------------------------------------------------------------------
const int NUM_STAT_TYPES = 7;


//-----------------------------------------------------------------------------------------------
class EntityBlueprint
{
public:
	virtual ~EntityBlueprint() {}
	static void InitalizeEntityBlueprints( const std::string& entityBlueprintFileName );
	static EntityBlueprint* GetEntityBlueprintByName( const std::string& blueprintName );

	std::string		m_name;
	Vector2			m_spriteCoords;
	static Vector2	s_texCoordOffset;
	static Texture*	s_entityTextureMap;

protected:
	static std::map< std::string, EntityBlueprint* >	s_entityBlueprints;

private:
	static void LoadEntityBlueprints( const XMLDocument& xmlDocument, const IntVector2& maxCoordsAllowed );
	static void LoadActorBlueprints( const XMLDocument& xmlDocument, const IntVector2& maxCoordsAllowed );
	static void LoadItemBlueprints( const XMLDocument& xmlDocument, const IntVector2& maxCoordsAllowed );
};


#endif // include_EntityBlueprint
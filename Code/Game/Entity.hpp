#ifndef include_Entity
#define include_Entity
#pragma once

//-----------------------------------------------------------------------------------------------
#include "GameCommon.hpp"
#include "EntityBlueprint.hpp"


//-----------------------------------------------------------------------------------------------
class Entity
{
public:
	Entity();
	Entity( const std::string& entityBlueprintName );
	virtual ~Entity();
	static Entity* GetEntityByID( int entityID );
	static void SetEntity( Entity* entity );
	virtual EntityBlueprint* GetBlueprint();
	virtual void Update( float deltaSeconds );
	virtual void Render();

	EntityBlueprint*	m_blueprint;
	HexTileCoords		m_positionCoords;
	int					m_entityID;
	static int			s_nexEntityID;

protected:
	static std::map< int, Entity* >		s_entities;
};


#endif // include_Entity
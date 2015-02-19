#ifndef include_HexMap
#define include_HexMap
#pragma once

//-----------------------------------------------------------------------------------------------
#include <set>
#include "Actor.hpp"
#include "HexTile.hpp"
#include "GameCommon.hpp"
#include "../Engine/Texture.hpp"
#include "../Engine/Vector2.hpp"
#include "../Engine/BitmapFont.hpp"


//-----------------------------------------------------------------------------------------------
const float ENTITY_SPRITE_HEIGHT_BY_HEX_TILE_HEIGHT_PERCENTAGE = 0.55f;
const std::string ENTITY_FONT_GLYPH_SHEET_FILE_NAME = "Data/Fonts/MainFont_EN_00.png";
const std::string ENTITY_FONT_META_DATA_FILE_NAME = "Data/Fonts/MainFont_EN.FontDef.xml";


//-----------------------------------------------------------------------------------------------
enum neighborDirection
{
	EAST,
	NORTHEAST,
	NORTHWEST,
	WEST,
	SOUTHWEST,
	SOUTHEAST,
};


//-----------------------------------------------------------------------------------------------
struct AStarHexNode 
{
	AStarHexNode() {}
	AStarHexNode( const HexTileCoords& coords ) : tileCoords( coords ) {}

	HexTileCoords					tileCoords;
	std::vector< HexTileCoords >	pathToHere;
	float							fValue;
	float							gValue;
	float							hValue;
};


//-----------------------------------------------------------------------------------------------
class HexMap
{
public:
	HexMap();
	HexMap( const std::string& mapFileName, float tileHeightPixels );
	~HexMap();
	std::string GetMapName();
	Actor* GetPlayerInMap();
	float GetHexTileHeightPixels();
	WorldCoords ConvertHexTileCoordsToWorldCoords( const HexTileCoords& tileCoords );
	HexTileCoords ConvertWorldCoordsToHexTileCoords( const WorldCoords& worldCoords );
	std::vector< HexTileCoords > GetAStarShortestPath( const HexTileCoords& start, const HexTileCoords& goal );
	std::vector< HexTileCoords > GetLineOfTilesBetweenTwoTiles( const HexTileCoords& start, const HexTileCoords& goal, int maxLineDistance );
	std::vector< HexTileCoords > GetCoordsOfTilesInRangeOfActor( const Actor* actor );
	std::vector< HexTileCoords > GetRingOfTilesAroundActor( const Actor* actor, int numberStepsOut );
	std::vector< HexTileCoords > GetSpiralOfTilesAroundActor( const Actor* actor, int numberStepsOut );
	std::vector< HexTileCoords > GetTilesInLineOfSightOfActor( const Actor* actor );
	WorldCoords GetPlayerPositionInWorldCoords();
	float GetDirectDistanceBetweenTileCoords( const HexTileCoords& tileCoords1, const HexTileCoords& tileCoords2 );
	bool CheckIfTileIsPlaceable( const HexTileCoords& tileCoords );
	bool CheckIfTileIsNeighborOfActor( const Actor* actor, const HexTileCoords& tileCoords );
	Entity* GetEntityLocatedAtCoordPosition( const HexTileCoords& tileCoords );
	HexTile* GetTileFromMapCoords( const HexTileCoords& tileCoords );
	void ChangeTileToTransformType( const HexTileCoords& tileCoords );
	void HighlightHexTile( const HexTileCoords& tileCoords );
	void RemoveEntityFromMapAndDelete( Entity* entity );
	void ChangeTileType( const HexTileCoords& tileCoords, const std::string& tileBlueprintName );
	void AddEntityID( int entityID );
	void Update( float deltaSeconds );
	void Render();

private:
	void LoadTilesIntoMap( const std::string& mapFileName );
	bool CheckIfActorIsLocatedInGivenCoords( const Actor* actor, const std::vector< HexTileCoords > tileCoords );
	HexTileCoords GetRoundedHexTileCoords( const Vector2& unroundedHexCoords );
	HexTileCoords GetNeighborTileCoords( const HexTileCoords& tileCoords, neighborDirection direction );
	HexTileCoords GetRandomNeighborTileCoords( const HexTileCoords& tileCoords );
	float GetActorEquippedItemsSumStat( const Actor* actor, statTypes modifierType );
	void CalculateAndApplyAttack( Actor* attacker, Actor* defender );
	void UpdateTiles();
	void UpdateEntitiesInMap( float deltaSeconds );
	void UpdatePlayerSight();
	void UpdateWhereActorIsMoving( Actor* actor );
	void RenderTiles();
	void RenderEntitiesInMap();

	HexTile*			m_tileGrid;
	std::string			m_name;
	int					m_numRows;
	int					m_numColumns;
	float				m_tileHeightPixels;
	float				m_tileWidthPixels;
	std::set<int>		m_entityIDs;
	Actor*				m_player;
	BitmapFont			m_glyphFont;
};


#endif // include_HexMap
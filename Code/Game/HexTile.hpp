#ifndef include_HexTile
#define include_HexTile
#pragma once

//-----------------------------------------------------------------------------------------------
#include "GameCommon.hpp"
#include "HexTileBlueprint.hpp"
#include "../Engine/NamedProperties.hpp"


//-----------------------------------------------------------------------------------------------
const float HEX_TILE_WIDTH_HEIGHT_RATIO = 0.875f;


//-----------------------------------------------------------------------------------------------
enum
{
	HEX_TILE_HAS_BEEN_SEEN = 0x01,
	HEX_TILE_IS_IN_VISIBLE_RANGE = 0x02,
	HEX_TILE_IS_HIGHLIGHTED = 0x04,
};


//-----------------------------------------------------------------------------------------------
class HexTile
{
public:
	HexTile();
	HexTile( const std::string& tilBlueprintName );
	~HexTile();
	void CheckForMapLoad( const NamedProperties& params );
	bool CheckIfTileHasBeenSeen();
	bool CheckIfTileIsInVisibleRange();
	bool CheckIfTileIsHighlighted();
	void SetIfTileHasBeenSeen( bool hasBeenSeen );
	void SetIfTileIsInVisibleRange( bool isVisible );
	void SetIfTileIsHighlighted( bool isHighlighted );
	void Render();
	static void AddLoadMapTile( HexTile* tile, const std::string& mapName );
	static std::string GetLoadMapName( HexTile* tile );

	HexTileBlueprint*	m_blueprint;

private:
	unsigned char		m_renderFlags;
	static std::map< HexTile*, std::string > s_loadMaps;
};


#endif // include_HexTile
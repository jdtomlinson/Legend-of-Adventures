#ifndef include_HexTileBlueprint
#define include_HexTileBlueprint
#pragma once

//-----------------------------------------------------------------------------------------------
#include <map>
#include <string>
#include "ItemBlueprint.hpp"
#include "../Engine/Texture.hpp"
#include "../Engine/Vector2.hpp"


//-----------------------------------------------------------------------------------------------
class HexTileBlueprint
{
public:
	HexTileBlueprint();
	static void InitalizeTileBlueprints( const std::string& tileBlueprintFileName );
	static HexTileBlueprint* GetHexTileBlueprintByName( const std::string& blueprintName );

	std::string			m_name;
	bool				m_isLighted;
	bool				m_isSolid;
	bool				m_isImpassable;
	Vector2				m_spriteCoords;
	HexTileBlueprint*	m_tileTransformType;
	ItemBlueprint*		m_itemNeededToTransform;
	static Vector2		s_texCoordOffset;
	static Texture*		s_hexTileTextureMap;

private:
	static std::map< std::string, HexTileBlueprint* >	s_tileBlueprints;
};


#endif // include_HexTileBlueprint
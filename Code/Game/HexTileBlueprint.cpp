#include "HexTileBlueprint.hpp"
#include "GameCommon.hpp"
#include "GameErrorFunctions.hpp"
#include "../Engine/XMLParsingFunctions.hpp"

//-----------------------------------------------------------------------------------------------
STATIC std::map< std::string, HexTileBlueprint* > HexTileBlueprint::s_tileBlueprints;
STATIC Vector2 HexTileBlueprint::s_texCoordOffset;
STATIC Texture* HexTileBlueprint::s_hexTileTextureMap;


//-----------------------------------------------------------------------------------------------
HexTileBlueprint::HexTileBlueprint()
	: m_name( "" )
	, m_isSolid( true )
	, m_isImpassable( true )
	, m_spriteCoords( 0, 0 )
	, m_tileTransformType( nullptr )
	, m_itemNeededToTransform( nullptr )
{

}


//-----------------------------------------------------------------------------------------------
STATIC void HexTileBlueprint::InitalizeTileBlueprints( const std::string& tileBlueprintFileName )
{
	std::map< HexTileBlueprint*, std::string > tileTransforms;

	XMLDocument tileDoc( tileBlueprintFileName );
	ValidateXMLChildElements( tileDoc, "HexTileDefinitions", "" );
	tileDoc.SetCurrentNodeToChild( "HexTileDefinitions" );
	ValidateXMLChildElements( tileDoc, "SpriteSheetInfo, Tiles", "" );
	tileDoc.SetCurrentNodeToChild( "SpriteSheetInfo" );
	ValidateXMLAttributes( tileDoc, "fileName, numberOfRows, numberOfColumns", "" );
	std::string textureFileName = GetStringXMLAttribute( tileDoc, "fileName", "" );
	s_hexTileTextureMap = Texture::CreateOrGetTexture( textureFileName );
	if( s_hexTileTextureMap->m_openglTextureID == 0 )
		ErrorTextureFileNotFound( tileDoc, textureFileName );

	int numberOfRows = GetIntXMLAttribute( tileDoc, "numberOfRows", 0 );
	int numberOfColumns = GetIntXMLAttribute( tileDoc, "numberOfColumns", 0 );
	if( numberOfRows <= 0 || numberOfColumns <= 0 )
		ErrorNonPositiveNumberOfRowsOrColumnsForSpriteSheet( tileDoc, IntVector2( numberOfRows, numberOfColumns ) );
	
	s_texCoordOffset.x = 1.f / (float) numberOfRows;
	s_texCoordOffset.y = 1.f / (float) numberOfColumns;

	tileDoc.SetCurrentNodeToParent();
	tileDoc.SetCurrentNodeToChild( "Tiles" );
	ValidateXMLChildElements( tileDoc, "Tile", "" );
	tileDoc.SetCurrentNodeToFirstChild();

	while( !tileDoc.GetCurrentNode().IsNull() )
	{
		ValidateXMLAttributes( tileDoc, "name, isSolid, spriteCoords", "isLighted, isImpassable, tileTransformType, itemNeededToTransform" );

		HexTileBlueprint* tileBlueprint = new HexTileBlueprint;
		tileBlueprint->m_name = GetStringXMLAttribute( tileDoc, "name", "null" );
		tileBlueprint->m_isLighted = GetBoolXMLAttribute( tileDoc, "isLighted", false );
		tileBlueprint->m_isSolid = GetBoolXMLAttribute( tileDoc, "isSolid", true );
		tileBlueprint->m_isImpassable = GetBoolXMLAttribute( tileDoc, "isImpassable", tileBlueprint->m_isSolid );

		tileTransforms[ tileBlueprint ] = GetStringXMLAttribute( tileDoc, "tileTransformType", "" );
		std::string itemName = GetStringXMLAttribute( tileDoc, "itemNeededToTransform", "" );
		EntityBlueprint* entityBlueprint = EntityBlueprint::GetEntityBlueprintByName( itemName );
		ItemBlueprint* itemBlueprint = dynamic_cast< ItemBlueprint* >( entityBlueprint );
		if( itemBlueprint != nullptr )
		{
			tileBlueprint->m_itemNeededToTransform = itemBlueprint;
		}

		IntVector2 spriteIndexes = GetIntVector2XMLAttribute( tileDoc, "spriteCoords", IntVector2( 0, 0 ) );

		if( spriteIndexes.x < 0 || spriteIndexes.y < 0 )
			ErrorTexCoordsNotValid( tileDoc, spriteIndexes, IntVector2( numberOfRows, numberOfColumns ) );
		if( spriteIndexes.x >= numberOfRows || spriteIndexes.y >= numberOfColumns )
			ErrorTexCoordsNotValid( tileDoc, spriteIndexes, IntVector2( numberOfRows, numberOfColumns ) );

		tileBlueprint->m_spriteCoords.x = (float) spriteIndexes.y * s_texCoordOffset.y;
		tileBlueprint->m_spriteCoords.y = (float) spriteIndexes.x * s_texCoordOffset.x;

		HexTileBlueprint::s_tileBlueprints[ tileBlueprint->m_name ] = tileBlueprint;

		tileDoc.SetCurrentNodeToNextSibling();
	}

	std::map< HexTileBlueprint*, std::string >::iterator transformIter;
	for( transformIter = tileTransforms.begin(); transformIter != tileTransforms.end(); ++transformIter )
	{
		std::string transformTypeName = transformIter->second;
		if( transformTypeName == "" )
			continue;

		HexTileBlueprint* transformType = GetHexTileBlueprintByName( transformTypeName );
		if( transformType != nullptr )
		{
			transformIter->first->m_tileTransformType = transformType;
		}
	}
}


//-----------------------------------------------------------------------------------------------
HexTileBlueprint* HexTileBlueprint::GetHexTileBlueprintByName( const std::string& blueprintName )
{
	std::map< std::string, HexTileBlueprint* >::iterator blueprintIter = s_tileBlueprints.find( blueprintName );
	if( blueprintIter == s_tileBlueprints.end() )
		return nullptr;

	return blueprintIter->second;
}
#include "HexTile.hpp"
#include "../Engine/EventSystem.hpp"
#include "../Engine/OpenGLRenderer.hpp"


//-----------------------------------------------------------------------------------------------
STATIC std::map< HexTile*, std::string > HexTile::s_loadMaps;


//-----------------------------------------------------------------------------------------------
HexTile::HexTile()
{
	m_blueprint = nullptr;
	m_renderFlags = 0;
}


//-----------------------------------------------------------------------------------------------
HexTile::HexTile( const std::string& tileBlueprintName )
{
	m_blueprint = HexTileBlueprint::GetHexTileBlueprintByName( tileBlueprintName );
	m_renderFlags = 0;
}


//-----------------------------------------------------------------------------------------------
HexTile::~HexTile()
{
	EventSystem::UnregisterEventCallbackForObject( this );
}


//-----------------------------------------------------------------------------------------------
void HexTile::CheckForMapLoad( const NamedProperties& params )
{
	HexTile* tileMovedTo;
	PropertyGetResult results = params.GetProperty( "TileMovedTo", tileMovedTo );
	if( results != PROPERTY_GET_SUCCESS )
		return;

	if( tileMovedTo == this )
	{
		std::string loadMapName = HexTile::GetLoadMapName( this );
		NamedProperties params;
		params.SetProperty( "MapName", loadMapName );
		EventSystem::FireEvent( "LoadMap", params );
	}
}


//-----------------------------------------------------------------------------------------------
bool HexTile::CheckIfTileHasBeenSeen()
{
	return ( ( m_renderFlags & HEX_TILE_HAS_BEEN_SEEN ) != 0 );
}


//-----------------------------------------------------------------------------------------------
bool HexTile::CheckIfTileIsInVisibleRange()
{
	return ( ( m_renderFlags & HEX_TILE_IS_IN_VISIBLE_RANGE ) != 0 );
}

//-----------------------------------------------------------------------------------------------
bool HexTile::CheckIfTileIsHighlighted()
{
	return ( ( m_renderFlags & HEX_TILE_IS_HIGHLIGHTED ) != 0 );
}


//-----------------------------------------------------------------------------------------------
void HexTile::SetIfTileHasBeenSeen( bool hasBeenSeen )
{
	if( hasBeenSeen )
		m_renderFlags |= HEX_TILE_HAS_BEEN_SEEN;
	else
		m_renderFlags &= ~HEX_TILE_HAS_BEEN_SEEN;
}


//-----------------------------------------------------------------------------------------------
void HexTile::SetIfTileIsInVisibleRange( bool isVisible )
{
	if( isVisible )
		m_renderFlags |= HEX_TILE_IS_IN_VISIBLE_RANGE;
	else
		m_renderFlags &= ~HEX_TILE_IS_IN_VISIBLE_RANGE;
}


//-----------------------------------------------------------------------------------------------
void HexTile::SetIfTileIsHighlighted( bool isHighlighted )
{
	if( isHighlighted )
		m_renderFlags |= HEX_TILE_IS_HIGHLIGHTED;
	else
		m_renderFlags &= ~HEX_TILE_IS_HIGHLIGHTED;
}


//-----------------------------------------------------------------------------------------------
void HexTile::Render()
{
	if( m_blueprint == nullptr )
		return;

	if( HexTileBlueprint::s_hexTileTextureMap == nullptr )
		return;

	OpenGLRenderer::EnableTexture2D();
	OpenGLRenderer::BindTexture2D( HexTileBlueprint::s_hexTileTextureMap->m_openglTextureID );

	if( !m_blueprint->m_isLighted && !CheckIfTileIsInVisibleRange() )
	{
		OpenGLRenderer::SetColor3f( 0.3f, 0.3f, 0.3f );
	}
	else if( CheckIfTileIsHighlighted() )
	{
		OpenGLRenderer::SetColor3f( 0.93f, 0.91f, 0.67f );
	}
	else
	{
		OpenGLRenderer::SetColor3f( 1.f, 1.f, 1.f );
	}

	OpenGLRenderer::LoadMatrix();
	OpenGLRenderer::BeginRender( QUADS );
	{
		OpenGLRenderer::SetTexCoords2f( m_blueprint->m_spriteCoords.x, m_blueprint->m_spriteCoords.y + HexTileBlueprint::s_texCoordOffset.y );
		OpenGLRenderer::SetVertex2f( -0.5f * HEX_TILE_WIDTH_HEIGHT_RATIO, -0.5f );

		OpenGLRenderer::SetTexCoords2f( m_blueprint->m_spriteCoords.x + HexTileBlueprint::s_texCoordOffset.x, m_blueprint->m_spriteCoords.y + HexTileBlueprint::s_texCoordOffset.y );
		OpenGLRenderer::SetVertex2f( 0.5f * HEX_TILE_WIDTH_HEIGHT_RATIO, -0.5f );

		OpenGLRenderer::SetTexCoords2f( m_blueprint->m_spriteCoords.x + HexTileBlueprint::s_texCoordOffset.x, m_blueprint->m_spriteCoords.y );
		OpenGLRenderer::SetVertex2f( 0.5f * HEX_TILE_WIDTH_HEIGHT_RATIO, 0.5f );

		OpenGLRenderer::SetTexCoords2f( m_blueprint->m_spriteCoords.x, m_blueprint->m_spriteCoords.y );
		OpenGLRenderer::SetVertex2f( -0.5f * HEX_TILE_WIDTH_HEIGHT_RATIO, 0.5f );
	}
	OpenGLRenderer::EndRender();
}


//-----------------------------------------------------------------------------------------------
STATIC void HexTile::AddLoadMapTile( HexTile* tile, const std::string& mapName )
{
	s_loadMaps[ tile ] = mapName;
}


//-----------------------------------------------------------------------------------------------
STATIC std::string HexTile::GetLoadMapName( HexTile* tile )
{
	std::map< HexTile*, std::string >::iterator nameIter = s_loadMaps.find( tile );
	if( nameIter == s_loadMaps.end() )
		return "";

	return nameIter->second;
}
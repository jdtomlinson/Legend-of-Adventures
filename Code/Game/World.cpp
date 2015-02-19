#include "World.hpp"
#include <io.h>
#include "ItemBlueprint.hpp"
#include "ActorBlueprint.hpp"
#include "HexTileBlueprint.hpp"
#include "GameErrorFunctions.hpp"
#include "../Engine/EventSystem.hpp"


//-----------------------------------------------------------------------------------------------
World::World( float worldWidth, float worldHeight )
	: m_size( worldWidth, worldHeight )
{
	
}


//-----------------------------------------------------------------------------------------------
void World::Initialize()
{
	m_isGameOver = false;
	m_hudFont = BitmapFont( HUD_FONT_GLYPH_SHEET_FILE_NAME, HUD_FONT_META_DATA_FILE_NAME );
	EntityBlueprint::InitalizeEntityBlueprints( ENTITY_DEFINITIONS_XML_FILE );
	HexTileBlueprint::InitalizeTileBlueprints( HEX_TILE_DEFINITIONS_XML_FILE );
	LoadMapsFromFile();
	std::map< std::string, HexMap* >::iterator mapIter = m_maps.begin();
	if( mapIter == m_maps.end() )
		ErrorNoMapsLoaded();

	m_currentMap = mapIter->second;

	EventSystem::RegisterEventCallbackForObject( "LoadMap", this, &World::ChangeCurrentMap );
}


//-----------------------------------------------------------------------------------------------
void World::AddEntityToMap( HexMap* gameMap, const std::string& entityTypeName, const HexTileCoords& entityLocation )
{
	if( gameMap == nullptr )
		return;

	if( !gameMap->CheckIfTileIsPlaceable( entityLocation ) )
		return;

	Entity* newEntity = new Entity( entityTypeName );
	newEntity->m_positionCoords = entityLocation;
	gameMap->AddEntityID( newEntity->m_entityID );
}


//-----------------------------------------------------------------------------------------------
bool World::CheckIfGameOver()
{
	return m_isGameOver;
}


//-----------------------------------------------------------------------------------------------
void World::Update( float deltaSeconds, const Keyboard& keyboard, const Mouse& mouse )
{
	if( m_currentMap == nullptr )
		return;

	Actor* player = m_currentMap->GetPlayerInMap();
	if( player != nullptr && player->m_currentHealth <= 0.f )
		m_isGameOver = true;
	else
		m_isGameOver = false;

	m_currentMap->Update( deltaSeconds );
	WorldCoords playerWorldPos = m_currentMap->GetPlayerPositionInWorldCoords();
	m_camera.m_position = Vector3( playerWorldPos.x, playerWorldPos.y, 0.f );

	UpdateFromInput( keyboard, mouse );
}


//-----------------------------------------------------------------------------------------------
void World::RenderObjects3D()
{
	
}


//-----------------------------------------------------------------------------------------------
void World::RenderObjects2D()
{
	OpenGLRenderer::PushMatrix();

	OpenGLRenderer::Translatef( ( m_size.x * 0.5f ) - m_camera.m_position.x, ( m_size.y * 0.5f ) - m_camera.m_position.y, m_camera.m_position.z );
	m_currentMap->Render();

	OpenGLRenderer::PopMatrix();

	RenderHUD();
}


//-----------------------------------------------------------------------------------------------
void World::RenderPause()
{
	OpenGLRenderer::PushMatrix();

	OpenGLRenderer::SetColor4f( PAUSE_SCREEN_COLOR.r, PAUSE_SCREEN_COLOR.g, PAUSE_SCREEN_COLOR.b, PAUSE_SCREEN_COLOR.a );
	OpenGLRenderer::LoadMatrix();

	OpenGLRenderer::BeginRender( QUADS );
	{
		OpenGLRenderer::SetVertex2f( 0.f, 0.f );
		OpenGLRenderer::SetVertex2f( m_size.x, 0.f );
		OpenGLRenderer::SetVertex2f( m_size.x, m_size.y );
		OpenGLRenderer::SetVertex2f( 0.f, m_size.y );
	}
	OpenGLRenderer::EndRender();

	float halfTextWidth = 0.5f * OpenGLRenderer::CalcTextWidth( "PAUSED", m_hudFont, PAUSE_FONT_HEIGHT_PIXELS );
	Vector2 pauseScreenPosition( ( ( m_size.x * 0.5f ) - halfTextWidth ), ( ( m_size.y * 0.5f ) - ( 0.5f * PAUSE_FONT_HEIGHT_PIXELS ) ) );
	OpenGLRenderer::RenderText( "PAUSED", m_hudFont, PAUSE_FONT_HEIGHT_PIXELS, pauseScreenPosition, PAUSE_FONT_COLOR );

	OpenGLRenderer::PopMatrix();
}


//-----------------------------------------------------------------------------------------------
void World::LoadMapsFromFile()
{
	int error = 0;
	struct _finddata_t fileInfo;
	std::string mapPathPatter = GAME_MAPS_FILE_PATH + "*.xml";
	intptr_t searchHandle = _findfirst( mapPathPatter.c_str(), &fileInfo );

	while( searchHandle != -1 && !error )
	{
		std::string mapFileName = GAME_MAPS_FILE_PATH + fileInfo.name;
		HexMap* gameMap = new HexMap( mapFileName, HEX_TILE_HEIGHT_PIXELS );
		std::string mapName = gameMap->GetMapName();
		m_maps[ mapName ] = gameMap;
		m_mapXMLFiles[ gameMap ] = mapFileName;
		error = _findnext( searchHandle, &fileInfo );
	}
}


//-----------------------------------------------------------------------------------------------
void World::ChangeCurrentMap( const NamedProperties& params )
{
	std::string mapName;
	PropertyGetResult result = params.GetProperty( "MapName", mapName );
	if( result != PROPERTY_GET_SUCCESS )
		return;

	std::map< std::string, HexMap* >::iterator mapIter = m_maps.find( mapName );
	if( mapIter != m_maps.end() )
	{
		m_currentMap = mapIter->second;
	}
}


//-----------------------------------------------------------------------------------------------
void World::UpdateFromKeyboard( const Keyboard& keyboard )
{
	Actor* player = m_currentMap->GetPlayerInMap();
	if( player == nullptr )
		return;

	if( m_isGameOver )
	{
		if( keyboard.IsKeyPressedDownAndWasNotBefore( KEY_SPACE ) )
		{
			std::string mapFileName = m_mapXMLFiles[ m_currentMap ];
			delete m_currentMap;
			m_currentMap = new HexMap( mapFileName, HEX_TILE_HEIGHT_PIXELS );
		}

		return;
	}

	if( keyboard.IsKeyPressedDown( KEY_A ) )
	{
		player->m_coordMovingTo = HexTileCoords( player->m_positionCoords.x - 1, player->m_positionCoords.y );
	}
	else if( keyboard.IsKeyPressedDown( KEY_D ) )
	{
		player->m_coordMovingTo = HexTileCoords( player->m_positionCoords.x + 1, player->m_positionCoords.y );
	}

	if( keyboard.IsKeyPressedDown( KEY_Q ) )
	{
		player->m_coordMovingTo = HexTileCoords( player->m_positionCoords.x - 1, player->m_positionCoords.y + 1 );
	}
	else if( keyboard.IsKeyPressedDown( KEY_C ) )
	{
		player->m_coordMovingTo = HexTileCoords( player->m_positionCoords.x + 1, player->m_positionCoords.y - 1 );
	}

	if( keyboard.IsKeyPressedDown( KEY_E ) )
	{
		player->m_coordMovingTo = HexTileCoords( player->m_positionCoords.x, player->m_positionCoords.y + 1 );
	}
	else if( keyboard.IsKeyPressedDown( KEY_Z ) )
	{
		player->m_coordMovingTo = HexTileCoords( player->m_positionCoords.x, player->m_positionCoords.y - 1 );
	}

	if( keyboard.IsKeyPressedDownAndWasNotBefore( KEY_J ) )
	{
		player->SetLeftHandEquippedItemToNextItem();
	}
	else if( keyboard.IsKeyPressedDownAndWasNotBefore( KEY_K ) )
	{
		player->SetBodyEquippedItemToNextItem();
	}
	else if( keyboard.IsKeyPressedDownAndWasNotBefore( KEY_L ) )
	{
		player->SetRightHandEquippedItemToNextItem();
	}
}


//-----------------------------------------------------------------------------------------------
void World::UpdateFromMouse( const Mouse& mouse )
{
	Actor* player = m_currentMap->GetPlayerInMap();
	if( player == nullptr )
		return;

	WorldCoords playerWorldPosition = m_currentMap->GetPlayerPositionInWorldCoords();

	Vector2 mouseWindowPosition = mouse.GetCursorPositionOnWindow();
	WorldCoords mouseWorldPosition = mouseWindowPosition + ( playerWorldPosition - ( m_size * 0.5f ) );
	HexTileCoords highlightCoords = m_currentMap->ConvertWorldCoordsToHexTileCoords( mouseWorldPosition );
	m_currentMap->HighlightHexTile( highlightCoords );
	m_infoTileCoords = highlightCoords;

	if( mouse.IsLeftButtonClicked() && m_currentMap->CheckIfTileIsNeighborOfActor( player, highlightCoords ) )
	{
		m_currentMap->ChangeTileToTransformType( highlightCoords );
	}

	if( mouse.IsLeftButtonDown() )
	{
		Item* leftHandItem = player->m_leftHandEquippedItem;
		if( leftHandItem != nullptr && leftHandItem->m_blueprint != nullptr && leftHandItem->m_blueprint->m_range > 0 )
		{
			std::vector< HexTileCoords > line = m_currentMap->GetLineOfTilesBetweenTwoTiles( player->m_positionCoords, highlightCoords, leftHandItem->m_blueprint->m_range );
			for( unsigned int coordIndex = 0; coordIndex < line.size(); ++coordIndex )
			{
				m_currentMap->HighlightHexTile( line[ coordIndex ] );
			}
		}
	}

	if( mouse.IsRightButtonDown() )
	{
		Item* rightHandItem = player->m_rightHandEquippedItem;
		if( rightHandItem != nullptr && rightHandItem->m_blueprint != nullptr && rightHandItem->m_blueprint->m_range > 0 )
		{
			std::vector< HexTileCoords > line = m_currentMap->GetLineOfTilesBetweenTwoTiles( player->m_positionCoords, highlightCoords, rightHandItem->m_blueprint->m_range );
			for( unsigned int coordIndex = 0; coordIndex < line.size(); ++coordIndex )
			{
				m_currentMap->HighlightHexTile( line[ coordIndex ] );
			}
		}
	}

	/*if( mouse.IsLeftButtonReleased() )
	{
		Item* leftHandItem = player->m_leftHandEquippedItem;
		if( leftHandItem != nullptr && leftHandItem->m_blueprint != nullptr )
		{
			Vector2 mouseWindowPos = mouse.GetCursorPositionOnWindow();
			WorldCoords highlightCoords = mouseWindowPos + ( playerWorldPos - ( m_size * 0.5f ) );
			Item* spawnItem = leftHandItem->GetSpawnedItem();
			if( spawnItem != nullptr )
			{
				spawnItem->m_movementPath = m_currentMap->GetLineOfTilesBetweenTwoTiles( player->m_positionCoords, m_currentMap->ConvertWorldCoordsToHexTileCoords( highlightCoords ), leftHandItem->m_blueprint->m_range );
				spawnItem->m_positionCoords = spawnItem->m_movementPath[0];
				m_currentMap->AddEntityID( spawnItem->m_entityID );
			}
		}
	}

	if( mouse.IsRightButtonReleased() )
	{
		Item* rightHandItem = player->m_rightHandEquippedItem;
		if( rightHandItem != nullptr && rightHandItem->m_blueprint != nullptr )
		{
			Vector2 mouseWindowPos = mouse.GetCursorPositionOnWindow();
			WorldCoords highlightCoords = mouseWindowPos + ( playerWorldPos - ( m_size * 0.5f ) );
			Item* spawnItem = rightHandItem->GetSpawnedItem();
			if( spawnItem != nullptr )
			{
				spawnItem->m_movementPath = m_currentMap->GetLineOfTilesBetweenTwoTiles( player->m_positionCoords, m_currentMap->ConvertWorldCoordsToHexTileCoords( highlightCoords ), rightHandItem->m_blueprint->m_range );
				spawnItem->m_positionCoords = spawnItem->m_movementPath[0];
				m_currentMap->AddEntityID( spawnItem->m_entityID );
			}
		}
	}*/
}


//-----------------------------------------------------------------------------------------------
void World::UpdateFromInput( const Keyboard& keyboard, const Mouse& mouse )
{
	UpdateFromKeyboard( keyboard );
	UpdateFromMouse( mouse );
}


//-----------------------------------------------------------------------------------------------
void World::RenderPlayerHealthBar()
{
	OpenGLRenderer::DisableTexture2D();
	OpenGLRenderer::SetColor3f( HUD_BACKGROUND_COLOR.r, HUD_BACKGROUND_COLOR.g, HUD_BACKGROUND_COLOR.b );
	OpenGLRenderer::LoadMatrix();

	OpenGLRenderer::BeginRender( QUADS );
	{
		OpenGLRenderer::SetVertex2f( m_size.x * 0.02f, m_size.y * 0.9f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.3f, m_size.y * 0.9f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.3f, m_size.y * 0.95f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.02f, m_size.y * 0.95f );
	}
	OpenGLRenderer::EndRender();

	OpenGLRenderer::RenderText( "Health", m_hudFont, HUD_FONT_HEIGHT_PIXELS, Vector2( m_size.x * 0.02f, m_size.y * 0.95f ), HUD_HEALTH_BAR_COLOR );

	Actor* player = m_currentMap->GetPlayerInMap();
	if( player == nullptr || player->m_blueprint == nullptr )
		return;

	OpenGLRenderer::SetColor3f( HUD_HEALTH_BAR_COLOR.r, HUD_HEALTH_BAR_COLOR.g, HUD_HEALTH_BAR_COLOR.b );
	float percentageHealth = player->m_currentHealth / player->m_blueprint->m_maxHP;

	OpenGLRenderer::BeginRender( QUADS );
	{
		OpenGLRenderer::SetVertex2f( m_size.x * 0.025f, m_size.y * 0.905f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.025f + ( m_size.x * 0.27f * percentageHealth ), m_size.y * 0.905f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.025f + ( m_size.x * 0.27f * percentageHealth ), m_size.y * 0.945f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.025f, m_size.y * 0.945f );
	}
	OpenGLRenderer::EndRender();
}


//-----------------------------------------------------------------------------------------------
void World::RenderPlayerExperienceBar()
{
	OpenGLRenderer::DisableTexture2D();
	OpenGLRenderer::SetColor3f( HUD_BACKGROUND_COLOR.r, HUD_BACKGROUND_COLOR.g, HUD_BACKGROUND_COLOR.b );
	OpenGLRenderer::LoadMatrix();

	OpenGLRenderer::BeginRender( QUADS );
	{
		OpenGLRenderer::SetVertex2f( m_size.x * 0.02f, m_size.y * 0.8f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.3f, m_size.y * 0.8f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.3f, m_size.y * 0.85f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.02f, m_size.y * 0.85f );
	}
	OpenGLRenderer::EndRender();

	OpenGLRenderer::RenderText( "EXP", m_hudFont, HUD_FONT_HEIGHT_PIXELS, Vector2( m_size.x * 0.02f, m_size.y * 0.85f ), HUD_EXPERIENCE_BAR_COLOR );

	Actor* player = m_currentMap->GetPlayerInMap();
	if( player == nullptr || player->m_blueprint == nullptr )
		return;

	OpenGLRenderer::SetColor3f( HUD_EXPERIENCE_BAR_COLOR.r, HUD_EXPERIENCE_BAR_COLOR.g, HUD_EXPERIENCE_BAR_COLOR.b );

	float currentLevel = (float)( player->m_currentLevel );
	float nextLevel = currentLevel + 1.f;
	float experienceForCurrentLevel = ( currentLevel * currentLevel ) + ( currentLevel - 1.f ) * 10.f;
	float experienceForNextLevel = ( nextLevel * nextLevel ) + ( nextLevel - 1.f ) * 10.f;
	float percentageExperience = ( player->m_currentExperience - experienceForCurrentLevel ) / ( experienceForNextLevel - experienceForCurrentLevel );
	percentageExperience = ClampFloat( percentageExperience, 0.f, 1.f );

	OpenGLRenderer::BeginRender( QUADS );
	{
		OpenGLRenderer::SetVertex2f( m_size.x * 0.025f, m_size.y * 0.805f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.025f + ( m_size.x * 0.27f * percentageExperience ), m_size.y * 0.805f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.025f + ( m_size.x * 0.27f * percentageExperience ), m_size.y * 0.845f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.025f, m_size.y * 0.845f );
	}
	OpenGLRenderer::EndRender();
}


//-----------------------------------------------------------------------------------------------
void World::RenderPlayerLevel()
{
	OpenGLRenderer::DisableTexture2D();
	OpenGLRenderer::LoadMatrix();

	OpenGLRenderer::BeginRender( QUADS );
	{
		OpenGLRenderer::SetColor3f( HUD_BACKGROUND_COLOR.r, HUD_BACKGROUND_COLOR.g, HUD_BACKGROUND_COLOR.b );

		OpenGLRenderer::SetVertex2f( m_size.x * 0.02f, m_size.y * 0.62f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.08f, m_size.y * 0.62f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.08f, m_size.y * 0.75f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.02f, m_size.y * 0.75f );

		OpenGLRenderer::SetColor3f( 0.f, 0.f, 0.f );

		OpenGLRenderer::SetVertex2f( m_size.x * 0.025f, m_size.y * 0.63f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.075f, m_size.y * 0.63f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.075f, m_size.y * 0.74f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.025f, m_size.y * 0.74f );
	}
	OpenGLRenderer::EndRender();

	OpenGLRenderer::RenderText( "Level", m_hudFont, HUD_FONT_HEIGHT_PIXELS, Vector2( m_size.x * 0.02f, m_size.y * 0.75f ), Color::White );

	Actor* player = m_currentMap->GetPlayerInMap();
	if( player == nullptr )
		return;

	std::string levelAsString = ConvertNumberToString( player->m_currentLevel );
	float levelNumberOffset = OpenGLRenderer::CalcTextWidth( levelAsString, m_hudFont, HUD_FONT_HEIGHT_PIXELS * 2.5f );
	OpenGLRenderer::RenderText( levelAsString, m_hudFont, HUD_FONT_HEIGHT_PIXELS * 2.5f, Vector2( ( m_size.x * 0.05f ) - ( levelNumberOffset * 0.5f ), ( m_size.y * 0.675f ) - ( HUD_FONT_HEIGHT_PIXELS * 1.15f ) ), Color::White );

	OpenGLRenderer::BindTexture2D( 0 );
	OpenGLRenderer::DisableTexture2D();
}


//-----------------------------------------------------------------------------------------------
void World::RenderInventory()
{
	OpenGLRenderer::DisableTexture2D();
	OpenGLRenderer::LoadMatrix();

	OpenGLRenderer::BeginRender( QUADS );
	{
		OpenGLRenderer::SetColor3f( HUD_BACKGROUND_COLOR.r, HUD_BACKGROUND_COLOR.g, HUD_BACKGROUND_COLOR.b );

		OpenGLRenderer::SetVertex2f( m_size.x * 0.39f, m_size.y * 0.84f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.45f, m_size.y * 0.84f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.45f, m_size.y * 0.97f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.39f, m_size.y * 0.97f );

		OpenGLRenderer::SetVertex2f( m_size.x * 0.47f, m_size.y * 0.84f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.53f, m_size.y * 0.84f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.53f, m_size.y * 0.97f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.47f, m_size.y * 0.97f );

		OpenGLRenderer::SetVertex2f( m_size.x * 0.55f, m_size.y * 0.84f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.61f, m_size.y * 0.84f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.61f, m_size.y * 0.97f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.55f, m_size.y * 0.97f );

		OpenGLRenderer::SetColor3f( 0.f, 0.f, 0.f );

		OpenGLRenderer::SetVertex2f( m_size.x * 0.395f, m_size.y * 0.85f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.445f, m_size.y * 0.85f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.445f, m_size.y * 0.96f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.395f, m_size.y * 0.96f );

		OpenGLRenderer::SetVertex2f( m_size.x * 0.475f, m_size.y * 0.85f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.525f, m_size.y * 0.85f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.525f, m_size.y * 0.96f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.475f, m_size.y * 0.96f );

		OpenGLRenderer::SetVertex2f( m_size.x * 0.555f, m_size.y * 0.85f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.605f, m_size.y * 0.85f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.605f, m_size.y * 0.96f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.555f, m_size.y * 0.96f );
	}
	OpenGLRenderer::EndRender();

	OpenGLRenderer::RenderText( "Left Hand", m_hudFont, HUD_FONT_HEIGHT_PIXELS * 0.65f, Vector2( m_size.x * 0.39f, m_size.y * 0.97f ), HUD_INVENTORY_COLOR );
	OpenGLRenderer::RenderText( "Body", m_hudFont, HUD_FONT_HEIGHT_PIXELS * 0.65f, Vector2( m_size.x * 0.47f, m_size.y * 0.97f ), HUD_INVENTORY_COLOR );
	OpenGLRenderer::RenderText( "Right Hand", m_hudFont, HUD_FONT_HEIGHT_PIXELS * 0.65f, Vector2( m_size.x * 0.55f, m_size.y * 0.97f ), HUD_INVENTORY_COLOR );

	Actor* player = m_currentMap->GetPlayerInMap();
	if( player == nullptr )
		return;

	if( EntityBlueprint::s_entityTextureMap == nullptr )
		return;

	Item* rightHandItem = player->m_rightHandEquippedItem;
	Item* leftHandItem = player->m_leftHandEquippedItem;
	Item* bodyItem = player->m_bodyEquippedItem;

	OpenGLRenderer::EnableTexture2D();
	OpenGLRenderer::BindTexture2D( EntityBlueprint::s_entityTextureMap->m_openglTextureID );
	OpenGLRenderer::SetColor3f( 1.f, 1.f, 1.f );

	OpenGLRenderer::BeginRender( QUADS );
	{
		if( leftHandItem != nullptr && leftHandItem->GetBlueprint() != nullptr )
		{
			OpenGLRenderer::SetTexCoords2f( leftHandItem->GetBlueprint()->m_spriteCoords.x, leftHandItem->GetBlueprint()->m_spriteCoords.y + EntityBlueprint::s_texCoordOffset.y );
			OpenGLRenderer::SetVertex2f( m_size.x * 0.405f, m_size.y * 0.87f );

			OpenGLRenderer::SetTexCoords2f( leftHandItem->GetBlueprint()->m_spriteCoords.x + EntityBlueprint::s_texCoordOffset.x, leftHandItem->GetBlueprint()->m_spriteCoords.y + EntityBlueprint::s_texCoordOffset.y );
			OpenGLRenderer::SetVertex2f( m_size.x * 0.435f, m_size.y * 0.87f );

			OpenGLRenderer::SetTexCoords2f( leftHandItem->GetBlueprint()->m_spriteCoords.x + EntityBlueprint::s_texCoordOffset.x, leftHandItem->GetBlueprint()->m_spriteCoords.y );
			OpenGLRenderer::SetVertex2f( m_size.x * 0.435f, m_size.y * 0.94f );

			OpenGLRenderer::SetTexCoords2f( leftHandItem->GetBlueprint()->m_spriteCoords.x, leftHandItem->GetBlueprint()->m_spriteCoords.y );
			OpenGLRenderer::SetVertex2f( m_size.x * 0.405f, m_size.y * 0.94f );
		}

		if( bodyItem != nullptr && bodyItem->GetBlueprint() != nullptr )
		{
			OpenGLRenderer::SetTexCoords2f( bodyItem->GetBlueprint()->m_spriteCoords.x, bodyItem->GetBlueprint()->m_spriteCoords.y + EntityBlueprint::s_texCoordOffset.y );
			OpenGLRenderer::SetVertex2f( m_size.x * 0.485f, m_size.y * 0.87f );

			OpenGLRenderer::SetTexCoords2f( bodyItem->GetBlueprint()->m_spriteCoords.x + EntityBlueprint::s_texCoordOffset.x, bodyItem->GetBlueprint()->m_spriteCoords.y + EntityBlueprint::s_texCoordOffset.y );
			OpenGLRenderer::SetVertex2f( m_size.x * 0.515f, m_size.y * 0.87f );

			OpenGLRenderer::SetTexCoords2f( bodyItem->GetBlueprint()->m_spriteCoords.x + EntityBlueprint::s_texCoordOffset.x, bodyItem->GetBlueprint()->m_spriteCoords.y );
			OpenGLRenderer::SetVertex2f( m_size.x * 0.515f, m_size.y * 0.94f );

			OpenGLRenderer::SetTexCoords2f( bodyItem->GetBlueprint()->m_spriteCoords.x, bodyItem->GetBlueprint()->m_spriteCoords.y );
			OpenGLRenderer::SetVertex2f( m_size.x * 0.485f, m_size.y * 0.94f );
		}

		if( rightHandItem != nullptr && rightHandItem->GetBlueprint() != nullptr )
		{
			OpenGLRenderer::SetTexCoords2f( rightHandItem->GetBlueprint()->m_spriteCoords.x, rightHandItem->GetBlueprint()->m_spriteCoords.y + EntityBlueprint::s_texCoordOffset.y );
			OpenGLRenderer::SetVertex2f( m_size.x * 0.565f, m_size.y * 0.87f );

			OpenGLRenderer::SetTexCoords2f( rightHandItem->GetBlueprint()->m_spriteCoords.x + EntityBlueprint::s_texCoordOffset.x, rightHandItem->GetBlueprint()->m_spriteCoords.y + EntityBlueprint::s_texCoordOffset.y );
			OpenGLRenderer::SetVertex2f( m_size.x * 0.595f, m_size.y * 0.87f );

			OpenGLRenderer::SetTexCoords2f( rightHandItem->GetBlueprint()->m_spriteCoords.x + EntityBlueprint::s_texCoordOffset.x, rightHandItem->GetBlueprint()->m_spriteCoords.y );
			OpenGLRenderer::SetVertex2f( m_size.x * 0.595f, m_size.y * 0.94f );

			OpenGLRenderer::SetTexCoords2f( rightHandItem->GetBlueprint()->m_spriteCoords.x, rightHandItem->GetBlueprint()->m_spriteCoords.y );
			OpenGLRenderer::SetVertex2f( m_size.x * 0.565f, m_size.y * 0.94f );
		}
	}
	OpenGLRenderer::EndRender();

	OpenGLRenderer::BindTexture2D( 0 );
	OpenGLRenderer::DisableTexture2D();
}


//-----------------------------------------------------------------------------------------------
void World::RenderTileInfo()
{
	HexTile* tile = m_currentMap->GetTileFromMapCoords( m_infoTileCoords );
	if( tile == nullptr || tile->m_blueprint == nullptr )
		return;

	if( !tile->CheckIfTileHasBeenSeen() )
		return;

	OpenGLRenderer::DisableTexture2D();
	OpenGLRenderer::LoadMatrix();

	OpenGLRenderer::BeginRender( QUADS );
	{
		OpenGLRenderer::SetColor3f( HUD_BACKGROUND_COLOR.r, HUD_BACKGROUND_COLOR.g, HUD_BACKGROUND_COLOR.b );

		OpenGLRenderer::SetVertex2f( m_size.x * 0.02f, m_size.y * 0.05f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.25f, m_size.y * 0.05f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.25f, m_size.y * 0.215f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.02f, m_size.y * 0.215f );

		OpenGLRenderer::SetColor3f( 0.f, 0.f, 0.f );

		OpenGLRenderer::SetVertex2f( m_size.x * 0.025f, m_size.y * 0.055f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.245f, m_size.y * 0.055f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.245f, m_size.y * 0.21f );
		OpenGLRenderer::SetVertex2f( m_size.x * 0.025f, m_size.y * 0.21f );
	}
	OpenGLRenderer::EndRender();

	OpenGLRenderer::RenderText( "Tile Info", m_hudFont, HUD_FONT_HEIGHT_PIXELS * 0.85f, Vector2( m_size.x * 0.02f, m_size.y * 0.21f ), Color::White );

	std::string tileName = "Tile Name: " + tile->m_blueprint->m_name;
	OpenGLRenderer::RenderText( tileName, m_hudFont, TILE_INFO_FONT_HEIGHT_PIXELS, Vector2( m_size.x * 0.028f, m_size.y * 0.18f ), Color::White );

	std::string tileCoordsString = "Hex Coords: " + ConvertNumberToString( m_infoTileCoords.x ) + ", " + ConvertNumberToString( m_infoTileCoords.y );
	OpenGLRenderer::RenderText( tileCoordsString, m_hudFont, TILE_INFO_FONT_HEIGHT_PIXELS, Vector2( m_size.x * 0.028f, m_size.y * 0.155f ), Color::White );

	std::string tileIsSolidString = "Is Solid: ";
	if( tile->m_blueprint->m_isSolid )
		tileIsSolidString += "true";
	else
		tileIsSolidString += "false";
	OpenGLRenderer::RenderText( tileIsSolidString, m_hudFont, TILE_INFO_FONT_HEIGHT_PIXELS, Vector2( m_size.x * 0.028f, m_size.y * 0.13f ), Color::White );

	std::string tileIsImpassableString = "Is Impassable: ";
	if( tile->m_blueprint->m_isSolid )
		tileIsImpassableString += "true";
	else
		tileIsImpassableString += "false";
	OpenGLRenderer::RenderText( tileIsImpassableString, m_hudFont, TILE_INFO_FONT_HEIGHT_PIXELS, Vector2( m_size.x * 0.028f, m_size.y * 0.105f ), Color::White );

	std::string tileTransformInto = "Transform Type: ";
	if( tile->m_blueprint->m_tileTransformType == nullptr )
		tileTransformInto += "none";
	else
		tileTransformInto += tile->m_blueprint->m_tileTransformType->m_name;
	OpenGLRenderer::RenderText( tileTransformInto, m_hudFont, TILE_INFO_FONT_HEIGHT_PIXELS, Vector2( m_size.x * 0.028f, m_size.y * 0.08f ), Color::White );

	std::string itemNeededName = "Item to Transform: ";
	if( tile->m_blueprint->m_itemNeededToTransform == nullptr )
		itemNeededName += "none";
	else
		itemNeededName += tile->m_blueprint->m_itemNeededToTransform->m_name;
	OpenGLRenderer::RenderText( itemNeededName, m_hudFont, TILE_INFO_FONT_HEIGHT_PIXELS, Vector2( m_size.x * 0.028f, m_size.y * 0.055f ), Color::White );
}


//-----------------------------------------------------------------------------------------------
void World::RenderPlayerDeath()
{
	OpenGLRenderer::PushMatrix();

	OpenGLRenderer::SetColor4f( PAUSE_SCREEN_COLOR.r, PAUSE_SCREEN_COLOR.g, PAUSE_SCREEN_COLOR.b, PAUSE_SCREEN_COLOR.a );
	OpenGLRenderer::LoadMatrix();

	OpenGLRenderer::BeginRender( QUADS );
	{
		OpenGLRenderer::SetVertex2f( 0.f, 0.f );
		OpenGLRenderer::SetVertex2f( m_size.x, 0.f );
		OpenGLRenderer::SetVertex2f( m_size.x, m_size.y );
		OpenGLRenderer::SetVertex2f( 0.f, m_size.y );
	}
	OpenGLRenderer::EndRender();

	float halfTextWidth = 0.5f * OpenGLRenderer::CalcTextWidth( "YOU HAVE DIED", m_hudFont, PAUSE_FONT_HEIGHT_PIXELS );
	float halfInstructionWidth = 0.5f * OpenGLRenderer::CalcTextWidth( "Press Spacebar to Restart", m_hudFont, PAUSE_FONT_HEIGHT_PIXELS * 0.5f );
	Vector2 deathPosition( ( ( m_size.x * 0.5f ) - halfTextWidth ), ( ( m_size.y * 0.55f ) - ( 0.5f * PAUSE_FONT_HEIGHT_PIXELS ) ) );
	Vector2 instructionPosition( ( ( m_size.x * 0.5f ) - halfInstructionWidth ), ( m_size.y * 0.45f ) - ( 0.25f * PAUSE_FONT_HEIGHT_PIXELS ) );
	OpenGLRenderer::RenderText( "YOU HAVE DIED", m_hudFont, PAUSE_FONT_HEIGHT_PIXELS, deathPosition, PAUSE_FONT_COLOR );
	OpenGLRenderer::RenderText( "Press Spacebar to Restart", m_hudFont, PAUSE_FONT_HEIGHT_PIXELS * 0.5f, instructionPosition, PAUSE_FONT_COLOR );

	OpenGLRenderer::PopMatrix();
}


//-----------------------------------------------------------------------------------------------
void World::RenderHUD()
{
	RenderPlayerHealthBar();
	RenderPlayerExperienceBar();
	RenderPlayerLevel();
	RenderInventory();
	RenderTileInfo();

	if( m_isGameOver )
		RenderPlayerDeath();
}
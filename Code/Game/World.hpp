#ifndef include_World
#define include_World
#pragma once

//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>
#include "Actor.hpp"
#include "HexMap.hpp"
#include "GameCommon.hpp"
#include "../Engine/Mouse.hpp"
#include "../Engine/Camera.hpp"
#include "../Engine/Vertex.hpp"
#include "../Engine/pugixml.hpp"
#include "../Engine/Texture.hpp"
#include "../Engine/Vector2.hpp"
#include "../Engine/Keyboard.hpp"
#include "../Engine/Material.hpp"
#include "../Engine/BitmapFont.hpp"
#include "../Engine/DebugGraphics.hpp"
#include "../Engine/OpenGLRenderer.hpp"
#include "../Engine/NamedProperties.hpp"
#include "../Engine/ConsoleCommandArgs.hpp"
#include "../Engine/XMLParsingFunctions.hpp"


//-----------------------------------------------------------------------------------------------
const float HEX_TILE_HEIGHT_PIXELS = 86.f;
const float HUD_FONT_HEIGHT_PIXELS = 32.f;
const float PAUSE_FONT_HEIGHT_PIXELS = 128.f;
const float TILE_INFO_FONT_HEIGHT_PIXELS = 26.f;
const Color HUD_BACKGROUND_COLOR = Color( 0.4f, 0.4f, 0.4f );
const Color HUD_HEALTH_BAR_COLOR = Color::Red;
const Color HUD_EXPERIENCE_BAR_COLOR = Color::Green;
const Color HUD_INVENTORY_COLOR = Color( 0.f, 0.6f, 1.f );
const Color PAUSE_SCREEN_COLOR = Color( 0.f, 0.f, 0.f, 0.6f );
const Color PAUSE_FONT_COLOR = Color::White;
const std::string HUD_FONT_GLYPH_SHEET_FILE_NAME = "Data/Fonts/MainFont_EN_00.png";
const std::string HUD_FONT_META_DATA_FILE_NAME = "Data/Fonts/MainFont_EN.FontDef.xml";
const std::string ENTITY_DEFINITIONS_XML_FILE = "Data/XML Files/Blueprint Definitions/EntityDefinitions.xml";
const std::string HEX_TILE_DEFINITIONS_XML_FILE = "Data/XML Files/Blueprint Definitions/HexTileDefinitions.xml";
const std::string GAME_MAPS_FILE_PATH = "Data/XML Files/Maps/";


//-----------------------------------------------------------------------------------------------
class World
{
public:
	World( float worldWidth, float worldHeight );
	void Initialize();
	void AddEntityToMap( HexMap* gameMap, const std::string& entityTypeName, const HexTileCoords& entityLocation );
	bool CheckIfGameOver();
	void Update( float deltaSeconds, const Keyboard& keyboard, const Mouse& mouse );
	void RenderObjects3D();
	void RenderObjects2D();
	void RenderPause();

private:
	void LoadMapsFromFile();
	void ChangeCurrentMap( const NamedProperties& params );
	void UpdateFromKeyboard( const Keyboard& keyboard );
	void UpdateFromMouse( const Mouse& mouse );
	void UpdateFromInput( const Keyboard& keyboard, const Mouse& mouse );
	void RenderPlayerHealthBar();
	void RenderPlayerExperienceBar();
	void RenderPlayerLevel();
	void RenderInventory();
	void RenderTileInfo();
	void RenderPlayerDeath();
	void RenderHUD();

	Camera								m_camera;
	Vector2								m_size;
	BitmapFont							m_hudFont;
	HexTileCoords						m_infoTileCoords;
	HexMap*								m_currentMap;
	bool								m_isGameOver;
	std::map< std::string, HexMap* >	m_maps;
	std::map< HexMap*, std::string >	m_mapXMLFiles;
};


#endif // include_World
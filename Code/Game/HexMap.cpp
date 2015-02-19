#include "HexMap.hpp"
#include <math.h>
#include "GameErrorFunctions.hpp"
#include "../Engine/EventSystem.hpp"
#include "../Engine/OpenGLRenderer.hpp"
#include "../Engine/XMLParsingFunctions.hpp"


//-----------------------------------------------------------------------------------------------
HexMap::HexMap()
{
	m_tileGrid = new HexTile[0];
}


//-----------------------------------------------------------------------------------------------
HexMap::HexMap( const std::string& mapFileName, float tileHeightPixels )
	: m_tileHeightPixels( tileHeightPixels )
	, m_tileWidthPixels( tileHeightPixels * HEX_TILE_WIDTH_HEIGHT_RATIO )
	, m_glyphFont( ENTITY_FONT_GLYPH_SHEET_FILE_NAME, ENTITY_FONT_META_DATA_FILE_NAME )
{
	LoadTilesIntoMap( mapFileName );
}


//-----------------------------------------------------------------------------------------------
HexMap::~HexMap()
{
	delete [] m_tileGrid;
}


//-----------------------------------------------------------------------------------------------
std::string HexMap::GetMapName()
{
	return m_name;
}


//-----------------------------------------------------------------------------------------------
Actor* HexMap::GetPlayerInMap()
{
	return m_player;
}


//-----------------------------------------------------------------------------------------------
WorldCoords HexMap::ConvertHexTileCoordsToWorldCoords( const HexTileCoords& tileCoords )
{
	WorldCoords returnWorldCoords;
	returnWorldCoords.x = ( m_tileWidthPixels * tileCoords.x ) + ( m_tileWidthPixels * tileCoords.y * 0.5f );
	returnWorldCoords.y = 0.75f * m_tileHeightPixels * tileCoords.y;

	return returnWorldCoords;
}


//-----------------------------------------------------------------------------------------------
HexTileCoords HexMap::ConvertWorldCoordsToHexTileCoords( const WorldCoords& worldCoords )
{
	WorldCoords adjustedCoords = worldCoords;
	adjustedCoords.x -= m_tileWidthPixels * 0.5f;

	Vector2 unroundedHexTileCoords;
	unroundedHexTileCoords.x = ( ( adjustedCoords.x * ONE_OVER_THREE * SQRT_OF_THREE ) - ( ONE_OVER_THREE * adjustedCoords.y ) ) / ( 0.5f * m_tileHeightPixels );
	unroundedHexTileCoords.y = ( 2.f * ONE_OVER_THREE * adjustedCoords.y ) / ( 0.5f * m_tileHeightPixels );

	HexTileCoords returnHexTileCoords = GetRoundedHexTileCoords( unroundedHexTileCoords );
	return returnHexTileCoords;
}


//-----------------------------------------------------------------------------------------------
std::vector< HexTileCoords > HexMap::GetAStarShortestPath( const HexTileCoords& startCoords, const HexTileCoords& goalCoords )
{
	std::vector< AStarHexNode > openList;
	std::vector< AStarHexNode > closeList;

	AStarHexNode startNode( startCoords );
	startNode.gValue = 0.f;
	startNode.hValue = GetDirectDistanceBetweenTileCoords( startCoords, goalCoords );
	startNode.fValue = startNode.gValue + startNode.hValue;
	openList.push_back( startNode );

	while( openList.size() != 0 )
	{
		AStarHexNode currentNode;
		currentNode.fValue = -1.f;
		int currentNodeIndex = -1;
		for( unsigned int nodeIndex = 0; nodeIndex < openList.size(); ++nodeIndex )
		{
			AStarHexNode node = openList[ nodeIndex ];
			if( node.fValue < currentNode.fValue || currentNode.fValue == -1.f )
			{
				currentNode = node;
				currentNodeIndex = nodeIndex;
			}
		}

		if( currentNodeIndex != -1 )
		{
			openList.erase( openList.begin() + currentNodeIndex );
		}

		std::vector< AStarHexNode > neighbors;
		neighbors.push_back( AStarHexNode( HexTileCoords( currentNode.tileCoords.x - 1, currentNode.tileCoords.y ) ) );
		neighbors.push_back( AStarHexNode( HexTileCoords( currentNode.tileCoords.x + 1, currentNode.tileCoords.y ) ) );
		neighbors.push_back( AStarHexNode( HexTileCoords( currentNode.tileCoords.x, currentNode.tileCoords.y - 1 ) ) );
		neighbors.push_back( AStarHexNode( HexTileCoords( currentNode.tileCoords.x, currentNode.tileCoords.y + 1 ) ) );
		neighbors.push_back( AStarHexNode( HexTileCoords( currentNode.tileCoords.x - 1, currentNode.tileCoords.y + 1 ) ) );
		neighbors.push_back( AStarHexNode( HexTileCoords( currentNode.tileCoords.x + 1, currentNode.tileCoords.y - 1 ) ) );

		for( unsigned int nodeIndex = 0; nodeIndex < neighbors.size(); ++nodeIndex )
		{
			AStarHexNode node = neighbors[ nodeIndex ];
			node.pathToHere = currentNode.pathToHere;
			node.pathToHere.push_back( currentNode.tileCoords );

			if( node.tileCoords == goalCoords )
			{
				node.pathToHere.push_back( node.tileCoords );
				return node.pathToHere;
			}

			HexTile* tile = GetTileFromMapCoords( node.tileCoords );
			if( tile == nullptr || tile->m_blueprint->m_isImpassable )
				continue;

			Entity* entityInSpace = GetEntityLocatedAtCoordPosition( node.tileCoords );
			if( entityInSpace != nullptr )
				continue;

			node.gValue = currentNode.gValue + 1;
			node.hValue = GetDirectDistanceBetweenTileCoords( node.tileCoords, goalCoords );
			node.fValue = node.gValue + node.hValue;

			bool nodeFoundInList = false;
			for ( unsigned int openIndex = 0; openIndex < openList.size(); ++openIndex )
			{
				if( openList[ openIndex ].tileCoords == node.tileCoords )
				{
					nodeFoundInList = true;
					break;
				}
			}

			for ( unsigned int closeIndex = 0; closeIndex < closeList.size(); ++closeIndex )
			{
				if( closeList[ closeIndex ].tileCoords == node.tileCoords )
				{
					nodeFoundInList = true;
					break;
				}
			}

			if( !nodeFoundInList )
			{
				openList.push_back( node );
			}
		}
		closeList.push_back( currentNode );
	}

	std::vector< HexTileCoords > emptyReturnVector;
	return emptyReturnVector;
}


//-----------------------------------------------------------------------------------------------
std::vector< HexTileCoords > HexMap::GetLineOfTilesBetweenTwoTiles( const HexTileCoords& start, const HexTileCoords& goal, int maxLineDistance )
{
	std::vector< HexTileCoords > tileLine;
	float hexDistance = GetDirectDistanceBetweenTileCoords( start, goal );
	float oneOverHexDistance = 1.f / hexDistance;
	int numberOfTiles = min( (int) hexDistance, maxLineDistance );

	for( int tileCount = 0; tileCount <= numberOfTiles; ++tileCount )
	{
		HexTileCoords tileCoords = GetRoundedHexTileCoords( Vector2( (float) start.x, (float) start.y ) * ( 1.f - ( tileCount * oneOverHexDistance ) ) + ( Vector2( (float) goal.x, (float) goal.y ) * ( tileCount * oneOverHexDistance ) ) );
		tileLine.push_back( tileCoords );

		HexTile* tile = GetTileFromMapCoords( tileCoords );
		if( tile == nullptr || tile->m_blueprint == nullptr || tile->m_blueprint->m_isSolid )
			break;
	}

	return tileLine;
}


//-----------------------------------------------------------------------------------------------
std::vector< HexTileCoords > HexMap::GetCoordsOfTilesInRangeOfActor( const Actor* actor )
{
	std::vector< HexTileCoords > tilesInSight;

	if( actor == nullptr )
		return tilesInSight;

	int actorSightDistance = (int) GetActorEquippedItemsSumStat( actor, STAT_SIGHT );
	for( int cubeCoordX = -actorSightDistance; cubeCoordX <= actorSightDistance; ++cubeCoordX )
	{
		int initalCoordY = max( -actorSightDistance, -actorSightDistance - cubeCoordX );
		int finalCoordY = min( actorSightDistance, actorSightDistance - cubeCoordX );

		for( int cubeCoordY = initalCoordY; cubeCoordY <= finalCoordY; ++cubeCoordY )
		{
			int cubeCoordZ = -cubeCoordX - cubeCoordY;
			HexTileCoords tileCoords( actor->m_positionCoords.x - cubeCoordX, actor->m_positionCoords.y - cubeCoordZ );
			HexTile* tile = GetTileFromMapCoords( tileCoords );
			if( tile != nullptr && tile->m_blueprint != nullptr )
				tilesInSight.push_back( tileCoords );
		}
	}

	return tilesInSight;
}


//-----------------------------------------------------------------------------------------------
std::vector< HexTileCoords > HexMap::GetRingOfTilesAroundActor( const Actor* actor, int numberStepsOut )
{
	std::vector< HexTileCoords > returnVector;
	HexTileCoords tileCoords( actor->m_positionCoords.x - numberStepsOut, actor->m_positionCoords.y + numberStepsOut );

	for( int hexSideIndex = 0; hexSideIndex < 6; ++hexSideIndex )
	{
		for( int tileIndex = 0; tileIndex < numberStepsOut; ++tileIndex )
		{
			returnVector.push_back( tileCoords );
			tileCoords = GetNeighborTileCoords( tileCoords, neighborDirection( hexSideIndex ) );
		}
	}

	return returnVector;
}


//-----------------------------------------------------------------------------------------------
std::vector< HexTileCoords > HexMap::GetSpiralOfTilesAroundActor( const Actor* actor, int numberStepsOut )
{
	std::vector< HexTileCoords > returnVector;

	for( int ringLayer = 1; ringLayer <= numberStepsOut; ++ringLayer )
	{
		std::vector< HexTileCoords > ringAroundActor = GetRingOfTilesAroundActor( actor, ringLayer );
		returnVector.insert( returnVector.end(), ringAroundActor.begin(), ringAroundActor.end() );
	}

	return returnVector;
}


//-----------------------------------------------------------------------------------------------
std::vector< HexTileCoords > HexMap::GetTilesInLineOfSightOfActor( const Actor* actor )
{
	std::vector< HexTileCoords > returnVector;
	int maxSightDistance = (int) GetActorEquippedItemsSumStat( actor, STAT_SIGHT );;
	std::vector< HexTileCoords > ringCoords = GetRingOfTilesAroundActor( actor, maxSightDistance );

	for( unsigned int ringIndex = 0; ringIndex < ringCoords.size(); ++ringIndex )
	{
		HexTileCoords endOfLine = ringCoords[ ringIndex ];
		std::vector< HexTileCoords > lineCoords = GetLineOfTilesBetweenTwoTiles( actor->m_positionCoords, endOfLine, maxSightDistance );

		for( unsigned int lineIndex = 0; lineIndex < lineCoords.size(); ++lineIndex )
		{
			HexTileCoords tileInLine = lineCoords[ lineIndex ];
			HexTile* tile = GetTileFromMapCoords( tileInLine );
			if( tile != nullptr )
			{
				returnVector.push_back( tileInLine );
			}
		}
	}

	return returnVector;
}


//-----------------------------------------------------------------------------------------------
WorldCoords HexMap::GetPlayerPositionInWorldCoords()
{
	if( m_player == nullptr )
		return WorldCoords( 0.f, 0.f );

	return ConvertHexTileCoordsToWorldCoords( m_player->m_positionCoords );
}


//-----------------------------------------------------------------------------------------------
float HexMap::GetDirectDistanceBetweenTileCoords( const HexTileCoords& tileCoords1, const HexTileCoords& tileCoords2 )
{
	float distance = 0.5f * ( abs( tileCoords1.x - tileCoords2.x ) + abs( tileCoords1.y - tileCoords2.y ) + abs( tileCoords1.x + tileCoords1.y - tileCoords2.x - tileCoords2.y ) );
	return distance;
}


//-----------------------------------------------------------------------------------------------
bool HexMap::CheckIfTileIsPlaceable( const HexTileCoords& tileCoords )
{
	HexTile* tile = GetTileFromMapCoords( tileCoords );
	if( tile == nullptr )
		return false;

	if( tile->m_blueprint == nullptr )
		return false;

	if( tile->m_blueprint->m_isImpassable )
		return false;

	return true;
}


//-----------------------------------------------------------------------------------------------
bool HexMap::CheckIfTileIsNeighborOfActor( const Actor* actor, const HexTileCoords& tileCoords )
{
	
	for( int hexSideIndex = 0; hexSideIndex < 6; ++hexSideIndex )
	{
		HexTileCoords neighborCoords = GetNeighborTileCoords( tileCoords, neighborDirection( hexSideIndex ) );
		if( neighborCoords == actor->m_positionCoords )
			return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
Entity* HexMap::GetEntityLocatedAtCoordPosition( const HexTileCoords& tileCoords )
{
	std::set<int>::iterator entityIter;
	for( entityIter = m_entityIDs.begin(); entityIter != m_entityIDs.end(); ++entityIter )
	{
		Entity* entity = Entity::GetEntityByID( *entityIter );
		if( entity != nullptr && entity->m_positionCoords == tileCoords )
			return entity;
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
HexTile* HexMap::GetTileFromMapCoords( const HexTileCoords& tileCoords )
{
	if( tileCoords.x < 0 || tileCoords.y < 0 )
		return nullptr;

	if( tileCoords.x >= m_numColumns || tileCoords.y >= m_numRows )
		return nullptr;

	int tileIndex = tileCoords.x + ( tileCoords.y * m_numColumns );
	return &m_tileGrid[ tileIndex ];
}


//-----------------------------------------------------------------------------------------------
void HexMap::ChangeTileToTransformType( const HexTileCoords& tileCoords )
{
	HexTile* tileToTransform = GetTileFromMapCoords( tileCoords );
	if( tileToTransform == nullptr || tileToTransform->m_blueprint == nullptr )
		return;

	Entity* entityAtTile = GetEntityLocatedAtCoordPosition( tileCoords );
	if( entityAtTile != nullptr )
		return;

	ItemBlueprint* itemNeeded = tileToTransform->m_blueprint->m_itemNeededToTransform;
	if( itemNeeded != nullptr && !m_player->CheckIfPlayerHasEquippedItemOfType( itemNeeded ) )
		return;

	HexTileBlueprint* transformType = tileToTransform->m_blueprint->m_tileTransformType;
	if( transformType == nullptr )
		return;

	tileToTransform->m_blueprint = transformType;
	Item* droppedItem = m_player->GetAndUnequipItemOfType( itemNeeded );
	if( droppedItem != nullptr )
	{
		RemoveEntityFromMapAndDelete( droppedItem );
	}
}


//-----------------------------------------------------------------------------------------------
void HexMap::HighlightHexTile( const HexTileCoords& tileCoords )
{
	HexTile* highlightTile = GetTileFromMapCoords( tileCoords );
	if( highlightTile == nullptr )
		return;

	highlightTile->SetIfTileIsHighlighted( true );
}


//-----------------------------------------------------------------------------------------------
void HexMap::RemoveEntityFromMapAndDelete( Entity* entity )
{
	m_entityIDs.erase( entity->m_entityID );

	Actor* actor = dynamic_cast< Actor* >( entity );
	if( actor != nullptr )
	{
		Item* droppedItem = nullptr;
		if( actor->m_bodyEquippedItem != nullptr )
			droppedItem = actor->m_bodyEquippedItem;
		else if( actor->m_rightHandEquippedItem != nullptr )
			droppedItem = actor->m_rightHandEquippedItem;
		else if( actor->m_leftHandEquippedItem != nullptr )
			droppedItem = actor->m_leftHandEquippedItem;

		if( droppedItem != nullptr )
		{
			AddEntityID( droppedItem->m_entityID );
			droppedItem->m_positionCoords = actor->m_positionCoords;
		}
	}

	delete entity;
	entity = nullptr;
}


//-----------------------------------------------------------------------------------------------
void HexMap::ChangeTileType( const HexTileCoords& tileCoords, const std::string& tileBlueprintName )
{
	HexTile* tile = GetTileFromMapCoords( tileCoords );
	if( tile == nullptr )
		return;

	HexTileBlueprint* tileBlueprint = HexTileBlueprint::GetHexTileBlueprintByName( tileBlueprintName );
	if( tileBlueprint == nullptr )
		return;

	tile->m_blueprint = tileBlueprint;
}


//-----------------------------------------------------------------------------------------------
void HexMap::AddEntityID( int entityID )
{
	m_entityIDs.insert( entityID );
}


//-----------------------------------------------------------------------------------------------
void HexMap::Update( float deltaSeconds )
{
	UpdateTiles();
	UpdateEntitiesInMap( deltaSeconds );
	UpdatePlayerSight();
}


//-----------------------------------------------------------------------------------------------
void HexMap::Render()
{
	OpenGLRenderer::PushMatrix();

	OpenGLRenderer::LoadMatrix();
	RenderTiles();
	RenderEntitiesInMap();

	OpenGLRenderer::PopMatrix();
}


//-----------------------------------------------------------------------------------------------
void HexMap::LoadTilesIntoMap( const std::string& mapFileName )
{
	bool playerHasBeenDefined = false;
	char playerGlyph = 0;
	std::set< char > usedAsciiSymbols;
	std::map< char, std::string > tileLegend;
	std::map< char, std::string > entityLegend;
	std::map< char, std::string > actorLegend;
	std::map< char, std::string > itemLegend;
	std::map< char, std::string > eventLegend;
	std::map< char, std::string > functionLegend;
	std::map< char, std::string > mapLegend;
	std::map< char, std::vector< std::string > > actorInventory;

	XMLDocument mapDoc( mapFileName );
	ValidateXMLChildElements( mapDoc, "MapDefinition", "" );
	mapDoc.SetCurrentNodeToChild( "MapDefinition" );
	ValidateXMLChildElements( mapDoc, "Legend, Map", "" );
	mapDoc.SetCurrentNodeToChild( "Legend" );
	ValidateXMLChildElements( mapDoc, "TileDefinition, PlayerDefinition", "ActorDefinition, ItemDefinition, EntityDefinition" );

	mapDoc.SetCurrentNodeToFirstChild();
	while( !mapDoc.GetCurrentNode().IsNull() )
	{
		ValidateXMLAttributes( mapDoc, "asciiSymbol, tileTypeName", "entityTypeName, actorTypeName, itemTypeName, eventToListenFor, functionToFireForEvent, loadMapName" );
		unsigned char asciiSymbolIndex = GetCharXMLAttribute( mapDoc, "asciiSymbol", 0 );
		std::set< char >::iterator setIter = usedAsciiSymbols.find( asciiSymbolIndex );
		if( setIter != usedAsciiSymbols.end() )
			ErrorAsciiSymbolUsedInMultipleDefinitions( mapDoc, asciiSymbolIndex );

		usedAsciiSymbols.insert( asciiSymbolIndex );
		std::string tileTypeName = GetStringXMLAttribute( mapDoc, "tileTypeName", "" );
		if( HexTileBlueprint::GetHexTileBlueprintByName( tileTypeName ) == nullptr )
			ErrorBlueprintNotFound( mapDoc, tileTypeName );

		tileLegend[ asciiSymbolIndex ] = tileTypeName;

		std::string eventToListenFor = GetStringXMLAttribute( mapDoc, "eventToListenFor", "" );
		if( eventToListenFor != "" )
		{
			eventLegend[ asciiSymbolIndex ] = eventToListenFor;
		}

		std::string functionToFire = GetStringXMLAttribute( mapDoc, "functionToFireForEvent", "" );
		if( functionToFire != "" )
		{
			functionLegend[ asciiSymbolIndex ] = functionToFire;
		}

		std::string loadMapName = GetStringXMLAttribute( mapDoc, "loadMapName", "" );
		if( loadMapName != "" )
		{
			mapLegend[ asciiSymbolIndex ] = loadMapName;
		}

		if( mapDoc.GetCurrentNode().GetNodeName() == "PlayerDefinition" )
		{
			if( playerHasBeenDefined )
				ErrorMultiplePlayerDefinitionInMapLegend( mapDoc );

			ValidateXMLAttributes( mapDoc, "asciiSymbol, tileTypeName, actorTypeName", "eventToListenFor, functionToFireForEvent, loadMapName" );
			std::string playerTypeName = GetStringXMLAttribute( mapDoc, "actorTypeName", "" );
			if( EntityBlueprint::GetEntityBlueprintByName( playerTypeName ) == nullptr )
				ErrorBlueprintNotFound( mapDoc, playerTypeName );

			actorLegend[ asciiSymbolIndex ] = playerTypeName;
			playerGlyph = asciiSymbolIndex;
			playerHasBeenDefined = true;
		}

		if( mapDoc.GetCurrentNode().GetNodeName() == "EntityDefinition" )
		{
			ValidateXMLAttributes( mapDoc, "asciiSymbol, tileTypeName, entityTypeName", "eventToListenFor, functionToFireForEvent, loadMapName" );
			std::string entityTypeName = GetStringXMLAttribute( mapDoc, "entityTypeName", "" );
			if( EntityBlueprint::GetEntityBlueprintByName( entityTypeName ) == nullptr )
				ErrorBlueprintNotFound( mapDoc, entityTypeName );

			entityLegend[ asciiSymbolIndex ] = entityTypeName;
		}

		if( mapDoc.GetCurrentNode().GetNodeName() == "ActorDefinition" )
		{
			ValidateXMLAttributes( mapDoc, "asciiSymbol, tileTypeName, actorTypeName", "eventToListenFor, functionToFireForEvent, loadMapName" );
			std::string actorTypeName = GetStringXMLAttribute( mapDoc, "actorTypeName", "" );
			if( EntityBlueprint::GetEntityBlueprintByName( actorTypeName ) == nullptr )
				ErrorBlueprintNotFound( mapDoc, actorTypeName );

			actorLegend[ asciiSymbolIndex ] = actorTypeName;
		}

		if( mapDoc.GetCurrentNode().GetNodeName() == "ItemDefinition" )
		{
			ValidateXMLAttributes( mapDoc, "asciiSymbol, tileTypeName, itemTypeName", "eventToListenFor, functionToFireForEvent, loadMapName" );
			std::string itemTypeName = GetStringXMLAttribute( mapDoc, "itemTypeName", "" );
			if( EntityBlueprint::GetEntityBlueprintByName( itemTypeName ) == nullptr )
				ErrorBlueprintNotFound( mapDoc, itemTypeName );

			itemLegend[ asciiSymbolIndex ] = itemTypeName;
		}

		if( mapDoc.GetCurrentNode().GetNodeName() == "ActorDefinition" || mapDoc.GetCurrentNode().GetNodeName() == "PlayerDefinition" )
		{
			ValidateXMLChildElements( mapDoc, "", "Inventory" );
			mapDoc.SetCurrentNodeToChild( "Inventory" );
			if( mapDoc.GetCurrentNode().IsNull() )
			{
				mapDoc.SetCurrentNodeToPreviousNode();
			}
			else
			{
				ValidateXMLChildElements( mapDoc, "Item", "" );
				mapDoc.SetCurrentNodeToFirstChild();
				std::vector< std::string > items;

				while( !mapDoc.GetCurrentNode().IsNull() )
				{
					ValidateXMLAttributes( mapDoc, "itemTypeName", "" );
					std::string itemTypeName = GetStringXMLAttribute( mapDoc, "itemTypeName", "" );
					items.push_back( itemTypeName );
					mapDoc.SetCurrentNodeToNextSibling();
				}

				actorInventory[ asciiSymbolIndex ] = items;
				mapDoc.SetCurrentNodeToPreviousNode();
				mapDoc.SetCurrentNodeToParent();
				mapDoc.SetCurrentNodeToParent();
			}
		}

		mapDoc.SetCurrentNodeToNextSibling();
	}

	mapDoc.SetCurrentNodeToRoot();
	mapDoc.SetCurrentNodeToChild( "MapDefinition" );
	mapDoc.SetCurrentNodeToChild( "Map" );
	ValidateXMLAttributes( mapDoc, "name", "" );
	ValidateXMLChildElements( mapDoc, "Row", "" );

	m_name = GetStringXMLAttribute( mapDoc, "name", "" );
	m_numRows = mapDoc.GetCurrentNode().GetNumberOfChildNodes();
	mapDoc.SetCurrentNodeToChild( "Row" );
	ValidateXMLAttributes( mapDoc, "characters", "" );
	std::string firstRow = GetStringXMLAttribute( mapDoc, "characters", "" );
	m_numColumns = GetStringWithNoWhitespace( firstRow ).size() + (int) floor( m_numRows * 0.5f );

	m_tileGrid = new HexTile[ m_numRows * m_numColumns ];
	bool playerPlacedInMap = false;

	int rIndex = m_numRows - 1;
	while( !mapDoc.GetCurrentNode().IsNull() )
	{
		ValidateXMLAttributes( mapDoc, "characters", "" );
		std::string rowString =  GetStringWithNoWhitespace( GetStringXMLAttribute( mapDoc, "characters", "" ) );
		for( unsigned int charIndex = 0; charIndex < rowString.size(); ++charIndex )
		{
			char tileChar = rowString[ charIndex ];
			int qIndex = charIndex + (int) floor( ( m_numRows - rIndex ) * 0.5f );

			std::map< char, std::string >::iterator legendIter = tileLegend.find( tileChar );
			if( legendIter != tileLegend.end() )
			{
				ChangeTileType( HexTileCoords( qIndex, rIndex ), legendIter->second );

				legendIter = eventLegend.find( tileChar );
				if( legendIter != eventLegend.end() )
				{
					std::string eventName = legendIter->second;
					HexTile* tile = GetTileFromMapCoords( HexTileCoords( qIndex, rIndex ) );

					legendIter = functionLegend.find( tileChar );
					if( legendIter != functionLegend.end() && GetLowercaseString( legendIter->second ) == "loadnextmap" )
					{
						legendIter = mapLegend.find( tileChar );
						if( legendIter != mapLegend.end() )
						{
							HexTile::AddLoadMapTile( tile, legendIter->second );
							EventSystem::RegisterEventCallbackForObject( eventName, tile, &HexTile::CheckForMapLoad );
						}
					}
				}
			}

			legendIter = entityLegend.find( tileChar );
			if( legendIter != entityLegend.end() )
			{
				Entity* entity = new Entity( legendIter->second );
				entity->m_positionCoords = HexTileCoords( qIndex, rIndex );
				Entity::SetEntity( entity );
				AddEntityID( entity->m_entityID );
			}

			legendIter = actorLegend.find( tileChar );
			if( legendIter != actorLegend.end() )
			{
				Actor* actor = new Actor( legendIter->second );
				actor->m_positionCoords = HexTileCoords( qIndex, rIndex );
				Entity::SetEntity( actor );
				AddEntityID( actor->m_entityID );

				std::map< char, std::vector< std::string > >::iterator inventoryIter = actorInventory.find( tileChar );
				if( inventoryIter != actorInventory.end() )
				{
					std::vector< std::string > inventoryItems = inventoryIter->second;
					for( unsigned int itemIndex = 0; itemIndex < inventoryItems.size(); ++itemIndex )
					{
						Item* inventoryItem = new Item( inventoryItems[ itemIndex ] );
						actor->AddItemToInventory( inventoryItem );
					}
				}

				if( tileChar == playerGlyph )
				{
					if( playerPlacedInMap )
						ErrorMultiplePlayersInMap( mapDoc, m_name );

					m_player = actor;
					playerPlacedInMap = true;
				}
			}

			legendIter = itemLegend.find( tileChar );
			if( legendIter != itemLegend.end() )
			{
				Item* item = new Item( legendIter->second );
				item->m_positionCoords = HexTileCoords( qIndex, rIndex );
				Entity::SetEntity( item );
				AddEntityID( item->m_entityID );
			}
		}

		--rIndex;
		mapDoc.SetCurrentNodeToNextSibling();
	}

	if( !playerPlacedInMap )
		ErrorNoPlayersInMap( mapDoc, m_name );
}


//-----------------------------------------------------------------------------------------------
bool HexMap::CheckIfActorIsLocatedInGivenCoords( const Actor* actor, const std::vector< HexTileCoords > tileCoords )
{
	if( actor == nullptr || actor->m_currentHealth <= 0.f )
		return false;

	for( unsigned int tileIndex = 0; tileIndex < tileCoords.size(); ++tileIndex )
	{
		if( actor->m_positionCoords == tileCoords[ tileIndex ] )
			return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
HexTileCoords HexMap::GetRoundedHexTileCoords( const Vector2& unroundedHexCoords )
{
	Vector3 unroundedCubeCoords;
	unroundedCubeCoords.x = unroundedHexCoords.x;
	unroundedCubeCoords.z = unroundedHexCoords.y;
	unroundedCubeCoords.y = -unroundedCubeCoords.x - unroundedCubeCoords.z;

	int roundX = (int) floor( unroundedCubeCoords.x + 0.5f );
	int roundY = (int) floor( unroundedCubeCoords.y + 0.5f );
	int roundZ = (int) floor( unroundedCubeCoords.z + 0.5f );

	float diffX = abs( roundX - unroundedCubeCoords.x );
	float diffY = abs( roundY - unroundedCubeCoords.y );
	float diffZ = abs( roundZ - unroundedCubeCoords.z );

	if( diffX > diffY && diffX > diffZ )
	{
		roundX = -roundY - roundZ;
	}
	else if( diffY > diffZ )
	{
		roundY = -roundX - roundZ;
	}
	else
	{
		roundZ = -roundX - roundY;
	}

	return HexTileCoords( roundX, roundZ );
}


//-----------------------------------------------------------------------------------------------
HexTileCoords HexMap::GetNeighborTileCoords( const HexTileCoords& tileCoords, neighborDirection direction )
{
	if( direction == EAST )
	{
		return HexTileCoords( tileCoords.x + 1, tileCoords.y );
	}
	else if( direction == NORTHEAST )
	{
		return HexTileCoords( tileCoords.x + 1, tileCoords.y - 1 );
	}
	else if( direction == NORTHWEST )
	{
		return HexTileCoords( tileCoords.x, tileCoords.y - 1 );
	}
	else if( direction == WEST )
	{
		return HexTileCoords( tileCoords.x - 1, tileCoords.y );
	}
	else if( direction == SOUTHWEST )
	{
		return HexTileCoords( tileCoords.x - 1, tileCoords.y + 1 );
	}
	else
	{
		return HexTileCoords( tileCoords.x, tileCoords.y + 1 );
	}
}


//-----------------------------------------------------------------------------------------------
HexTileCoords HexMap::GetRandomNeighborTileCoords( const HexTileCoords& tileCoords )
{
	neighborDirection randomDirection = neighborDirection( rand() % 6 );
	return GetNeighborTileCoords( tileCoords, randomDirection );
}


//-----------------------------------------------------------------------------------------------
float HexMap::GetActorEquippedItemsSumStat( const Actor* actor, statTypes modifierType )
{
	if( actor == nullptr || actor->m_blueprint == nullptr )
		return 0.f;
	
	float sumStatModifier = actor->m_blueprint->m_stats[ modifierType ];
	if( modifierType == STAT_SPEED )
		sumStatModifier *= -1.f;
	if( modifierType == STAT_ATTACK )
		sumStatModifier += (float)( actor->m_currentLevel - 1 ) * ATTACK_STAT_INCREASE_PER_LEVEL;
	if( modifierType == STAT_DEFENSE )
		sumStatModifier += (float)( actor->m_currentLevel - 1 ) * DEFENSE_STAT_INCREASE_PER_LEVEL;
	
	Item* rightHandItem = actor->m_rightHandEquippedItem;
	if( rightHandItem != nullptr && rightHandItem->m_blueprint != nullptr )
	{
		sumStatModifier += rightHandItem->m_blueprint->m_statModifiers[ modifierType ];
	}
	
	Item* leftHandItem = actor->m_leftHandEquippedItem;
	if( leftHandItem != nullptr && leftHandItem->m_blueprint != nullptr )
	{
		sumStatModifier += leftHandItem->m_blueprint->m_statModifiers[ modifierType ];
	}
	
	Item* bodyItem = actor->m_bodyEquippedItem;
	if( bodyItem != nullptr && bodyItem->m_blueprint != nullptr )
	{
		sumStatModifier += bodyItem->m_blueprint->m_statModifiers[ modifierType ];
	}
	
	if( modifierType == STAT_DEXTERITY || modifierType == STAT_DODGE )
		return ClampFloat( sumStatModifier, 0.f, 1.f );

	if( modifierType == STAT_SPEED )
		sumStatModifier *= -1.f;

	if( modifierType != STAT_HEALTH_REGEN && sumStatModifier < 0.f )
		sumStatModifier = 0.f;
	
	return sumStatModifier;
}


//-----------------------------------------------------------------------------------------------
void HexMap::CalculateAndApplyAttack( Actor* attacker, Actor* defender )
{
	if( attacker == nullptr || defender == nullptr )
		return;

	if( attacker->m_blueprint == nullptr || defender->m_blueprint == nullptr )
		return;

	float attackerChanceToHit = GetActorEquippedItemsSumStat( attacker, STAT_DEXTERITY );
	float defenderChanceToDodge = GetActorEquippedItemsSumStat( defender, STAT_DODGE );

	float chanceToHit = 0.5f * ( attackerChanceToHit + ( 1.f - defenderChanceToDodge ) );
	if( chanceToHit < GetRandomPercentage() )
		return;
	
	float attackDamage = GetActorEquippedItemsSumStat( attacker, STAT_ATTACK );
	float defenseConstitution = GetActorEquippedItemsSumStat( defender, STAT_DEFENSE );

	float damage = ceil( attackDamage - ( defenseConstitution * 0.5f ) );
	if( damage < 0.f )
	{
		damage = 0.f;
	}
	damage += ceil( 3.f * GetRandomPercentage() );

	defender->RemoveHealth( damage );
	if( defender->m_currentHealth == 0.f )
		attacker->m_currentExperience += defender->m_blueprint->m_experienceToGive;
}


//-----------------------------------------------------------------------------------------------
void HexMap::UpdateTiles()
{
	for( int rIndex = 0; rIndex < m_numRows; ++rIndex )
	{
		for( int qIndex = 0; qIndex < m_numColumns; ++qIndex )
		{
			HexTile* tile = GetTileFromMapCoords( HexTileCoords( qIndex, rIndex ) );
			if( tile == nullptr || tile->m_blueprint == nullptr )
				continue;

			tile->SetIfTileIsInVisibleRange( false );
			tile->SetIfTileIsHighlighted( false );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void HexMap::UpdateEntitiesInMap( float deltaSeconds )
{
	std::set<int>::iterator entityIter;
	for( entityIter = m_entityIDs.begin(); entityIter != m_entityIDs.end(); ++entityIter )
	{
		Entity* entity = Entity::GetEntityByID( *entityIter );
		entity->Update( deltaSeconds );

		Actor* actor = dynamic_cast< Actor* >( entity );
		if( actor != nullptr )
		{
			if( actor != m_player && actor->m_currentHealth == 0.f )
			{
				int iteratorIndex = std::distance( m_entityIDs.begin(), entityIter ) - 1;
				RemoveEntityFromMapAndDelete( actor );
				entityIter = m_entityIDs.begin();
				for( int iterIncrementCount = 0; iterIncrementCount < iteratorIndex; ++iterIncrementCount )
				{
					++entityIter;
				}
				continue;
			}

			if( actor->m_blueprint != nullptr && actor->m_currentHealth < actor->m_blueprint->m_maxHP )
			{
				actor->m_secondsSinceLastHealthRegen += deltaSeconds;
			}
			else
			{
				actor->m_secondsSinceLastHealthRegen = 0.f;
			}

			if( actor->m_secondsSinceLastHealthRegen > 1.f && actor->m_currentHealth > 0.f )
			{
				actor->m_secondsSinceLastHealthRegen = 0.f;
				if( actor->m_blueprint != nullptr )
					actor->AddHealth( GetActorEquippedItemsSumStat( actor, STAT_HEALTH_REGEN ) );
			}

			UpdateWhereActorIsMoving( actor );

			float secondsPerMove = GetActorEquippedItemsSumStat( actor, STAT_SPEED );
			if( CheckIfTileIsPlaceable( actor->m_coordMovingTo ) && ( actor->m_secondsSinceLastMove > secondsPerMove ) )
			{
				Entity* entityAtCoordMovingTo = GetEntityLocatedAtCoordPosition( actor->m_coordMovingTo );
				if( entityAtCoordMovingTo == nullptr )
				{
					actor->m_previousPosition = actor->m_positionCoords;
					actor->m_positionCoords = actor->m_coordMovingTo;
					actor->m_secondsSinceLastMove = 0.f;

					if( actor == m_player )
					{
						NamedProperties params;
						HexTile* tileMovedTo = GetTileFromMapCoords( actor->m_positionCoords );
						params.SetProperty( "TileMovedTo", tileMovedTo );
						EventSystem::FireEvent( "PlayerMove", params );
					}
				}
				else
				{
					Actor* actorAtCoordsMovingTo = dynamic_cast< Actor* >( entityAtCoordMovingTo );
					if( actor != actorAtCoordsMovingTo && actorAtCoordsMovingTo != nullptr )
					{
						actor->m_coordMovingTo = actor->m_positionCoords;
						if( actor == m_player || actorAtCoordsMovingTo == m_player )
						{
							CalculateAndApplyAttack( actor, actorAtCoordsMovingTo );
							actor->m_secondsSinceLastMove = 0.f;
						}
					}

					Item* itemAtCoordsMovingTo = dynamic_cast< Item* >( entityAtCoordMovingTo );
					if( itemAtCoordsMovingTo != nullptr && actor == m_player )
					{
						actor->m_previousPosition = actor->m_positionCoords;
						actor->m_positionCoords = actor->m_coordMovingTo;
						actor->AddItemToInventory( itemAtCoordsMovingTo );
						m_entityIDs.erase( itemAtCoordsMovingTo->m_entityID );
						actor->m_secondsSinceLastMove = 0.f;
					}
				}
			}
			else
			{
				actor->m_coordMovingTo = actor->m_positionCoords;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void HexMap::UpdatePlayerSight()
{
	if( m_player == nullptr || m_player->m_blueprint == nullptr )
		return;

	std::vector< HexTileCoords > tilesInSight = GetTilesInLineOfSightOfActor( m_player );
	for( unsigned int tileIndex = 0; tileIndex < tilesInSight.size(); ++tileIndex )
	{
		HexTile* tile = GetTileFromMapCoords( tilesInSight[ tileIndex ] );
		if( tile != nullptr )
		{
			tile->SetIfTileHasBeenSeen( true );
			tile->SetIfTileIsInVisibleRange( true );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void HexMap::UpdateWhereActorIsMoving( Actor* actor )
{
	if( actor == nullptr || actor->m_blueprint == nullptr )
		return;

	if( actor == m_player )
		return;

	if( actor->m_secondsSinceLastMove < actor->m_blueprint->m_stats[ STAT_SPEED ] )
		return;

	attitudeTowardPlayer attitude = actor->GetCurrentAttitude();
	if( attitude != NEUTRAL_TO_PLAYER )
	{
		std::vector< HexTileCoords > tilesInSight = GetTilesInLineOfSightOfActor( actor );
		if( CheckIfActorIsLocatedInGivenCoords( m_player, tilesInSight ) )
		{
			if( attitude == HATES_PLAYER )
			{
				std::vector< HexTileCoords > aStarPath = GetAStarShortestPath( actor->m_positionCoords, m_player->m_positionCoords );
				if( aStarPath.size() > 1 )
				{
					actor->m_coordMovingTo = aStarPath[1];
				}
			}
			else if( attitude == FEARS_PLAYER )
			{
				HexTileCoords tileCoordsToRunTo = actor->m_positionCoords;
				float farthestDistance = 0.f;

				std::vector< HexTileCoords > spiralAroundPlayer = GetSpiralOfTilesAroundActor( actor, 1 );
				for( unsigned int tileIndex = 0; tileIndex < spiralAroundPlayer.size(); ++tileIndex )
				{
					HexTileCoords tileCoords = spiralAroundPlayer[ tileIndex ];
					HexTile* tile = GetTileFromMapCoords( tileCoords );
					if( tile->m_blueprint->m_isImpassable )
						continue;

					Entity* entityInTile = GetEntityLocatedAtCoordPosition( tileCoords );
					if( entityInTile != nullptr )
						continue;

					float distanceBetweenPlayerAndCoords = GetDirectDistanceBetweenTileCoords( tileCoords, m_player->m_positionCoords );
					if( distanceBetweenPlayerAndCoords > farthestDistance )
					{
						farthestDistance = distanceBetweenPlayerAndCoords;
						tileCoordsToRunTo = tileCoords;
					}
				}

				actor->m_previousPosition = actor->m_positionCoords;
				actor->m_coordMovingTo = tileCoordsToRunTo;
			}

			return;
		}
	}
	
	HexTileCoords randomNeighbor = GetRandomNeighborTileCoords( actor->m_positionCoords );
	if( randomNeighbor != actor->m_previousPosition )
	{
		actor->m_coordMovingTo = randomNeighbor;
	}
}


//-----------------------------------------------------------------------------------------------
void HexMap::RenderTiles()
{
	for( int rIndex = 0; rIndex < m_numRows; ++rIndex )
	{
		for( int qIndex = 0; qIndex < m_numColumns; ++qIndex )
		{
			HexTile* tile = GetTileFromMapCoords( HexTileCoords( qIndex, rIndex ) );
			if( tile == nullptr || tile->m_blueprint == nullptr )
				continue;

			if( !tile->m_blueprint->m_isLighted && !tile->CheckIfTileHasBeenSeen() )
				continue;

			WorldCoords tileCenterPosition = ConvertHexTileCoordsToWorldCoords( HexTileCoords( qIndex, rIndex ) );
			OpenGLRenderer::PushMatrix();
			{
				OpenGLRenderer::Translatef( tileCenterPosition.x, tileCenterPosition.y, 0.f );
				OpenGLRenderer::Scalef( m_tileHeightPixels, m_tileHeightPixels, m_tileHeightPixels );
				tile->Render();
			}
			OpenGLRenderer::PopMatrix();
		}
	}
}


//-----------------------------------------------------------------------------------------------
void HexMap::RenderEntitiesInMap()
{
	if( EntityBlueprint::s_entityTextureMap == nullptr )
		return;

	std::set<int>::iterator entityIter;
	for( entityIter = m_entityIDs.begin(); entityIter != m_entityIDs.end(); ++entityIter )
	{
		Entity* entity = Entity::GetEntityByID( *entityIter );
		if( entity == nullptr )
			continue;

		HexTile* tile = GetTileFromMapCoords( entity->m_positionCoords );
		if( !tile->CheckIfTileHasBeenSeen() || !tile->CheckIfTileIsInVisibleRange() )
			continue;

		WorldCoords entityCenterPosition = ConvertHexTileCoordsToWorldCoords( entity->m_positionCoords );
		OpenGLRenderer::PushMatrix();
		{
			OpenGLRenderer::Translatef( entityCenterPosition.x, entityCenterPosition.y, 0.f );
			float scaleValue = m_tileHeightPixels * ENTITY_SPRITE_HEIGHT_BY_HEX_TILE_HEIGHT_PERCENTAGE;
			OpenGLRenderer::Scalef( scaleValue, scaleValue, scaleValue );
			entity->Render();
		}
		OpenGLRenderer::PopMatrix();
	}
}
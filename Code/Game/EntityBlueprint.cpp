#include "EntityBlueprint.hpp"
#include "GameCommon.hpp"
#include "ItemBlueprint.hpp"
#include "ActorBlueprint.hpp"
#include "GameErrorFunctions.hpp"
#include "../Engine/MathFunctions.hpp"


//-----------------------------------------------------------------------------------------------
STATIC std::map< std::string, EntityBlueprint* > EntityBlueprint::s_entityBlueprints;
STATIC Vector2 EntityBlueprint::s_texCoordOffset;
STATIC Texture* EntityBlueprint::s_entityTextureMap;


//-----------------------------------------------------------------------------------------------
STATIC void EntityBlueprint::InitalizeEntityBlueprints( const std::string& entityBlueprintFileName )
{
	XMLDocument entityDoc( entityBlueprintFileName );
	ValidateXMLChildElements( entityDoc, "EntityDefinitions", "" );
	entityDoc.SetCurrentNodeToChild( "EntityDefinitions" );
	ValidateXMLChildElements( entityDoc, "SpriteSheetInfo", "Entities, Actors, Items" );
	entityDoc.SetCurrentNodeToChild( "SpriteSheetInfo" );
	ValidateXMLAttributes( entityDoc, "fileName, numberOfRows, numberOfColumns", "" );
	std::string textureFileName = GetStringXMLAttribute( entityDoc, "fileName", "" );
	s_entityTextureMap = Texture::CreateOrGetTexture( textureFileName );
	if( s_entityTextureMap->m_openglTextureID == 0 )
		ErrorTextureFileNotFound( entityDoc, textureFileName );

	int numberOfRows = GetIntXMLAttribute( entityDoc, "numberOfRows", 0 );
	int numberOfColumns = GetIntXMLAttribute( entityDoc, "numberOfColumns", 0 );
	if( numberOfRows <= 0 || numberOfColumns <= 0 )
		ErrorNonPositiveNumberOfRowsOrColumnsForSpriteSheet( entityDoc, IntVector2( numberOfRows, numberOfColumns ) );

	s_texCoordOffset.x = 1.f / (float) numberOfRows;
	s_texCoordOffset.y = 1.f / (float) numberOfColumns;

	entityDoc.SetCurrentNodeToParent();
	entityDoc.SetCurrentNodeToChild( "Entities" );
	if( !entityDoc.GetCurrentNode().IsNull() )
	{
		LoadEntityBlueprints( entityDoc, IntVector2( numberOfRows, numberOfColumns ) );
	}
	entityDoc.SetCurrentNodeToPreviousNode();

	entityDoc.SetCurrentNodeToChild( "Actors" );
	if( !entityDoc.GetCurrentNode().IsNull() )
	{
		LoadActorBlueprints( entityDoc, IntVector2( numberOfRows, numberOfColumns ) );
	}
	entityDoc.SetCurrentNodeToPreviousNode();

	entityDoc.SetCurrentNodeToChild( "Items" );
	if( !entityDoc.GetCurrentNode().IsNull() )
	{
		LoadItemBlueprints( entityDoc, IntVector2( numberOfRows, numberOfColumns ) );
	}
	entityDoc.SetCurrentNodeToPreviousNode();
}


//-----------------------------------------------------------------------------------------------
STATIC EntityBlueprint* EntityBlueprint::GetEntityBlueprintByName( const std::string& blueprintName )
{
	std::map< std::string, EntityBlueprint* >::iterator blueprintIter = s_entityBlueprints.find( blueprintName );
	if( blueprintIter == s_entityBlueprints.end() )
		return nullptr;

	return blueprintIter->second;
}


//-----------------------------------------------------------------------------------------------
STATIC void EntityBlueprint::LoadEntityBlueprints( const XMLDocument& xmlDocument, const IntVector2& maxCoordsAllowed )
{
	XMLDocument entityDoc( xmlDocument.GetFileName().c_str() );
	entityDoc.SetCurrentNode( xmlDocument.GetCurrentNode() );
	entityDoc.SetCurrentNodeToFirstChild();

	while( !entityDoc.GetCurrentNode().IsNull() )
	{
		ValidateXMLAttributes( entityDoc, "name, spriteCoords", "" );

		EntityBlueprint* entityBlueprint = new EntityBlueprint;
		entityBlueprint->m_name = GetStringXMLAttribute( entityDoc, "name", "null" );
		IntVector2 spriteIndexes = GetIntVector2XMLAttribute( entityDoc, "spriteCoords", IntVector2( 0, 0 ) );

		if( spriteIndexes.x < 0 || spriteIndexes.y < 0 )
			ErrorTexCoordsNotValid( xmlDocument, spriteIndexes, maxCoordsAllowed );
		if( spriteIndexes.x >= maxCoordsAllowed.x || spriteIndexes.y >= maxCoordsAllowed.y )
			ErrorTexCoordsNotValid( xmlDocument, spriteIndexes, maxCoordsAllowed );

		entityBlueprint->m_spriteCoords.x = (float) spriteIndexes.y * s_texCoordOffset.y;
		entityBlueprint->m_spriteCoords.y = (float) spriteIndexes.x * s_texCoordOffset.x;

		s_entityBlueprints[ entityBlueprint->m_name ] = entityBlueprint;

		entityDoc.SetCurrentNodeToNextSibling();
	}
}


//-----------------------------------------------------------------------------------------------
STATIC void EntityBlueprint::LoadActorBlueprints( const XMLDocument& xmlDocument, const IntVector2& maxCoordsAllowed )
{
	XMLDocument actorDoc( xmlDocument.GetFileName().c_str() );
	actorDoc.SetCurrentNode( xmlDocument.GetCurrentNode() );
	ValidateXMLChildElements( actorDoc, "Actor", "" );
	actorDoc.SetCurrentNodeToFirstChild();

	while( !actorDoc.GetCurrentNode().IsNull() )
	{
		ValidateXMLAttributes( actorDoc, "name, maxHP, secondsPerMove, sight, attackStrength, defenseConstitution, experienceToGive, spriteCoords", "attitude, attitudeWhenAttacked, chanceToHitPercent, chanceToDodgePercent, hpRegenPerSecond" );

		ActorBlueprint* actorBlueprint = new ActorBlueprint;
		actorBlueprint->m_name = GetStringXMLAttribute( actorDoc, "name", "null" );
		actorBlueprint->m_maxHP = GetFloatXMLAttribute( actorDoc, "maxHP", 0.f );
		actorBlueprint->m_experienceToGive = GetFloatXMLAttribute( actorDoc, "experienceToGive", 0.f );
		actorBlueprint->m_stats[ STAT_SPEED ] = GetFloatXMLAttribute( actorDoc, "secondsPerMove", 0.f );
		actorBlueprint->m_stats[ STAT_HEALTH_REGEN ] = GetFloatXMLAttribute( actorDoc, "hpRegenPerSecond", 0.f );

		actorBlueprint->m_stats[ STAT_ATTACK ] = GetFloatXMLAttribute( actorDoc, "attackStrength", 0.f );
		if( actorBlueprint->m_stats[ STAT_ATTACK ] < 0.f )
			actorBlueprint->m_stats[ STAT_ATTACK ] = 0.f;
		actorBlueprint->m_stats[ STAT_DEFENSE ] = GetFloatXMLAttribute( actorDoc, "defenseConstitution", 0.f );
		if( actorBlueprint->m_stats[ STAT_DEFENSE ] < 0.f )
			actorBlueprint->m_stats[ STAT_DEFENSE ] = 0.f;

		float unclampedHitPercent = GetFloatXMLAttribute( actorDoc, "chanceToHitPercent", 1.f );
		actorBlueprint->m_stats[ STAT_DEXTERITY ] = ClampFloat( unclampedHitPercent, 0.f, 1.f );
		float unclampedDodgePercent = GetFloatXMLAttribute( actorDoc, "chanceToDodgePercent", 0.f );
		actorBlueprint->m_stats[ STAT_DODGE ] = ClampFloat( unclampedDodgePercent, 0.f, 1.f );

		actorBlueprint->m_stats[ STAT_SIGHT ] = GetFloatXMLAttribute( actorDoc, "sight", 1.f );
		IntVector2 spriteIndexes = GetIntVector2XMLAttribute( actorDoc, "spriteCoords", IntVector2( 0, 0 ) );

		if( spriteIndexes.x < 0 || spriteIndexes.y < 0 )
			ErrorTexCoordsNotValid( xmlDocument, spriteIndexes, maxCoordsAllowed );
		if( spriteIndexes.x >= maxCoordsAllowed.x || spriteIndexes.y >= maxCoordsAllowed.y )
			ErrorTexCoordsNotValid( xmlDocument, spriteIndexes, maxCoordsAllowed );

		actorBlueprint->m_spriteCoords.x = (float) spriteIndexes.y * s_texCoordOffset.y;
		actorBlueprint->m_spriteCoords.y = (float) spriteIndexes.x * s_texCoordOffset.x;

		std::string attitudeString = GetLowercaseString( GetStringXMLAttribute( actorDoc, "attitude", "neutral" ) );
		if( attitudeString == "hate" )
		{
			actorBlueprint->m_attitudeTowardPlayer = HATES_PLAYER;
		}
		else if( attitudeString == "fear" )
		{
			actorBlueprint->m_attitudeTowardPlayer = FEARS_PLAYER;
		}
		else
		{
			actorBlueprint->m_attitudeTowardPlayer = NEUTRAL_TO_PLAYER;
		}

		std::string attitudeWhenAttackedString = GetLowercaseString( GetStringXMLAttribute( actorDoc, "attitudeWhenAttacked", attitudeString ) );
		if( attitudeWhenAttackedString == "hate" )
		{
			actorBlueprint->m_attitudeWhenAttacked = HATES_PLAYER;
		}
		else if( attitudeWhenAttackedString == "fear" )
		{
			actorBlueprint->m_attitudeWhenAttacked = FEARS_PLAYER;
		}
		else
		{
			actorBlueprint->m_attitudeWhenAttacked = NEUTRAL_TO_PLAYER;
		}

		s_entityBlueprints[ actorBlueprint->m_name ] = actorBlueprint;

		actorDoc.SetCurrentNodeToNextSibling();
	}
}


//-----------------------------------------------------------------------------------------------
STATIC void EntityBlueprint::LoadItemBlueprints( const XMLDocument& xmlDocument, const IntVector2& maxCoordsAllowed )
{
	XMLDocument itemDoc( xmlDocument.GetFileName().c_str() );
	itemDoc.SetCurrentNode( xmlDocument.GetCurrentNode() );
	itemDoc.SetCurrentNodeToFirstChild();

	while( !itemDoc.GetCurrentNode().IsNull() )
	{
		ValidateXMLAttributes( itemDoc, "name, spriteCoords", "equipType, spawnItem, range, secondsPerMove, attackModifier, defenseModifier, dexterityModifier, dodgeModifier, speedModifier, healthRegenModifier, sightModifier" );

		ItemBlueprint* itemBlueprint = new ItemBlueprint;
		itemBlueprint->m_name = GetStringXMLAttribute( itemDoc, "name", "null" );
		itemBlueprint->m_nameItemTypeToSpawn = GetStringXMLAttribute( itemDoc, "spawnItem", "" );
		itemBlueprint->m_range = GetIntXMLAttribute( itemDoc, "range", 0 );
		itemBlueprint->m_secondsPerMove = GetFloatXMLAttribute( itemDoc, "secondsPerMove", 0.f );
		itemBlueprint->m_statModifiers[ STAT_ATTACK ] = GetFloatXMLAttribute( itemDoc, "attackModifier", 0.f );
		itemBlueprint->m_statModifiers[ STAT_DEFENSE ] = GetFloatXMLAttribute( itemDoc, "defenseModifier", 0.f );

		float unclampedDexModifier = GetFloatXMLAttribute( itemDoc, "dexterityModifier", 0.f );
		itemBlueprint->m_statModifiers[ STAT_DEXTERITY ] = ClampFloat( unclampedDexModifier, -1.f, 1.f );
		float unclampedDodgeModifier = GetFloatXMLAttribute( itemDoc, "dodgeModifier", 0.f );
		itemBlueprint->m_statModifiers[ STAT_DODGE ] = ClampFloat( unclampedDodgeModifier, -1.f, 1.f );

		itemBlueprint->m_statModifiers[ STAT_SPEED ] = GetFloatXMLAttribute( itemDoc, "speedModifier", 0.f );
		itemBlueprint->m_statModifiers[ STAT_HEALTH_REGEN ] = GetFloatXMLAttribute( itemDoc, "healthRegenModifier", 0.f );
		itemBlueprint->m_statModifiers[ STAT_SIGHT ] = GetFloatXMLAttribute( itemDoc, "sightModifier", 0.f );

		IntVector2 spriteIndexes = GetIntVector2XMLAttribute( itemDoc, "spriteCoords", IntVector2( 0, 0 ) );

		if( spriteIndexes.x < 0 || spriteIndexes.y < 0 )
			ErrorTexCoordsNotValid( xmlDocument, spriteIndexes, maxCoordsAllowed );
		if( spriteIndexes.x >= maxCoordsAllowed.x || spriteIndexes.y >= maxCoordsAllowed.y )
			ErrorTexCoordsNotValid( xmlDocument, spriteIndexes, maxCoordsAllowed );

		itemBlueprint->m_spriteCoords.x = (float) spriteIndexes.y * s_texCoordOffset.y;
		itemBlueprint->m_spriteCoords.y = (float) spriteIndexes.x * s_texCoordOffset.x;

		std::string equipTypeString = GetLowercaseString( GetStringXMLAttribute( itemDoc, "equipType", "" ) );
		if(	equipTypeString == "hand" )
		{
			itemBlueprint->m_equipType = EQUIP_TO_HAND;
		}
		else if( equipTypeString == "body" )
		{
			itemBlueprint->m_equipType = EQUIP_TO_BODY;
		}
		else if( equipTypeString == "all" )
		{
			itemBlueprint->m_equipType = EQUIP_TO_ALL;
		}
		else
		{
			itemBlueprint->m_equipType = EQUIP_TO_NONE;
		}

		s_entityBlueprints[ itemBlueprint->m_name ] = itemBlueprint;

		itemDoc.SetCurrentNodeToNextSibling();
	}
}
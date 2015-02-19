#include "Actor.hpp"
#include "../Engine/OpenGLRenderer.hpp"


//-----------------------------------------------------------------------------------------------
Actor::Actor()
	: m_blueprint( nullptr )
	, m_currentLevel( 0 )
	, m_currentHealth( 0.f )
	, m_currentExperience( 1.f )
	, m_secondsSinceLastMove( 0.f )
	, m_coordMovingTo( 0, 0 )
	, m_previousPosition( 0, 0 )
	, m_rightHandEquippedItem( nullptr )
	, m_leftHandEquippedItem( nullptr )
	, m_bodyEquippedItem( nullptr )
{

}


//-----------------------------------------------------------------------------------------------
Actor::Actor( const std::string& actorBlueprintName )
	: m_currentLevel( 0 )
	, m_currentExperience( 1.f )
	, m_coordMovingTo( 0, 0 )
	, m_previousPosition( 0, 0 )
	, m_rightHandEquippedItem( nullptr )
	, m_leftHandEquippedItem( nullptr )
	, m_bodyEquippedItem( nullptr )
{
	m_blueprint = dynamic_cast< ActorBlueprint* >( EntityBlueprint::GetEntityBlueprintByName( actorBlueprintName ) );
	if( m_blueprint == nullptr )
		return;

	m_currentHealth = m_blueprint->m_maxHP;
	m_secondsSinceLastMove = m_blueprint->m_stats[ STAT_SPEED ];
}


//-----------------------------------------------------------------------------------------------
Actor::~Actor()
{
	bool droppedItemSaved = false;
	for( unsigned int itemIndex = 0; itemIndex < m_inventory.size(); ++itemIndex )
	{
		Item* item = m_inventory[ itemIndex ];
		if( !droppedItemSaved && item == m_bodyEquippedItem )
		{
			droppedItemSaved = true;
			continue;
		}

		if( !droppedItemSaved && item == m_rightHandEquippedItem )
		{
			droppedItemSaved = true;
			continue;
		}

		if( !droppedItemSaved && item == m_leftHandEquippedItem )
		{
			droppedItemSaved = true;
			continue;
		}

		delete item;
		item = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
ActorBlueprint* Actor::GetBlueprint()
{
	return m_blueprint;
}


//-----------------------------------------------------------------------------------------------
void Actor::AddItemToInventory( Item* itemToAdd )
{
	if( itemToAdd == nullptr || itemToAdd->m_blueprint == nullptr )
		return;

	m_inventory.push_back( itemToAdd );
	if( itemToAdd->m_blueprint->m_equipType == EQUIP_TO_HAND )
	{
		if( m_rightHandEquippedItem == nullptr )
		{
			m_rightHandEquippedItem = itemToAdd;
		}
		else if( m_leftHandEquippedItem == nullptr )
		{
			m_leftHandEquippedItem = itemToAdd;
		}
	}

	if( itemToAdd->m_blueprint->m_equipType == EQUIP_TO_BODY )
	{
		if( m_bodyEquippedItem == nullptr )
		{
			m_bodyEquippedItem = itemToAdd;
		}
	}

	if( itemToAdd->m_blueprint->m_equipType == EQUIP_TO_ALL )
	{
		if( m_rightHandEquippedItem == nullptr )
		{
			m_rightHandEquippedItem = itemToAdd;
		}
		else if( m_leftHandEquippedItem == nullptr )
		{
			m_leftHandEquippedItem = itemToAdd;
		}
		else if( m_bodyEquippedItem == nullptr )
		{
			m_bodyEquippedItem = itemToAdd;
		}
	}
}


//-----------------------------------------------------------------------------------------------
Item* Actor::DropAndReturnItem( const std::string& itemTypeName )
{
	for( unsigned int itemIndex = 0; itemIndex < m_inventory.size(); ++itemIndex )
	{
		Item* item = m_inventory[ itemIndex ];
		if( item->m_blueprint->m_name == itemTypeName )
		{
			m_inventory.erase( m_inventory.begin() + itemIndex );
			return item;
		}
	}
	
	return nullptr;
}


//-----------------------------------------------------------------------------------------------
Item* Actor::GetAndUnequipItemOfType( const ItemBlueprint* itemType )
{
	for( unsigned int itemIndex = 0; itemIndex < m_inventory.size(); ++itemIndex )
	{
		Item* item = m_inventory[ itemIndex ];
		if( item->m_blueprint == itemType )
		{
			if( item == m_rightHandEquippedItem )
			{
				SetRightHandEquippedItemToNextItem();
				if( m_rightHandEquippedItem == item )
					m_rightHandEquippedItem = nullptr;

				m_inventory.erase( m_inventory.begin() + itemIndex );
				return item;
			}

			if( item == m_leftHandEquippedItem )
			{
				SetLeftHandEquippedItemToNextItem();
				if( m_leftHandEquippedItem == item )
					m_leftHandEquippedItem = nullptr;

				m_inventory.erase( m_inventory.begin() + itemIndex );
				return item;
			}

			if( item == m_bodyEquippedItem )
			{
				SetBodyEquippedItemToNextItem();
				if( m_bodyEquippedItem == item )
					m_bodyEquippedItem = nullptr;

				m_inventory.erase( m_inventory.begin() + itemIndex );
				return item;
			}
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
bool Actor::CheckIfPlayerHasEquippedItemOfType( const ItemBlueprint* itemType )
{
	if( m_leftHandEquippedItem != nullptr && m_leftHandEquippedItem->m_blueprint == itemType )
		return true;

	if( m_rightHandEquippedItem != nullptr && m_rightHandEquippedItem->m_blueprint == itemType )
		return true;

	if( m_bodyEquippedItem != nullptr && m_bodyEquippedItem->m_blueprint == itemType )
		return true;

	return false;
}


//-----------------------------------------------------------------------------------------------
void Actor::SetRightHandEquippedItemToNextItem()
{
	for( unsigned int itemIndex = 0; itemIndex < m_inventory.size(); ++itemIndex )
	{
		Item* item = m_inventory[ itemIndex ];
		if( item == m_rightHandEquippedItem )
		{
			Item* itemToEquipt = nullptr;
			while( itemToEquipt == nullptr )
			{
				if( itemIndex == m_inventory.size() - 1 )
					itemIndex = 0;
				else
					++itemIndex;

				Item* nextItem = m_inventory[ itemIndex ];

				if( nextItem != m_leftHandEquippedItem && nextItem != m_bodyEquippedItem )
				{
					if( nextItem != nullptr && nextItem->m_blueprint != nullptr )
					{
						if( nextItem->m_blueprint->m_equipType == EQUIP_TO_HAND || nextItem->m_blueprint->m_equipType == EQUIP_TO_ALL )
							itemToEquipt = nextItem;
					}
				}

				if( nextItem == m_rightHandEquippedItem )
					return;
			}

			m_rightHandEquippedItem = itemToEquipt;
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Actor::SetLeftHandEquippedItemToNextItem()
{
	for( unsigned int itemIndex = 0; itemIndex < m_inventory.size(); ++itemIndex )
	{
		Item* item = m_inventory[ itemIndex ];
		if( item == m_leftHandEquippedItem )
		{
			Item* itemToEquipt = nullptr;
			while( itemToEquipt == nullptr )
			{
				if( itemIndex == m_inventory.size() - 1 )
					itemIndex = 0;
				else
					++itemIndex;

				Item* nextItem = m_inventory[ itemIndex ];

				if( nextItem != m_rightHandEquippedItem && nextItem != m_bodyEquippedItem )
				{
					if( nextItem != nullptr && nextItem->m_blueprint != nullptr )
					{
						if( nextItem->m_blueprint->m_equipType == EQUIP_TO_HAND || nextItem->m_blueprint->m_equipType == EQUIP_TO_ALL )
							itemToEquipt = nextItem;
					}
				}

				if( nextItem == m_leftHandEquippedItem )
					return;
			}

			m_leftHandEquippedItem = itemToEquipt;
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Actor::SetBodyEquippedItemToNextItem()
{
	for( unsigned int itemIndex = 0; itemIndex < m_inventory.size(); ++itemIndex )
	{
		Item* item = m_inventory[ itemIndex ];
		if( item == m_bodyEquippedItem )
		{
			Item* itemToEquipt = nullptr;
			while( itemToEquipt == nullptr )
			{
				if( itemIndex == m_inventory.size() - 1 )
					itemIndex = 0;
				else
					++itemIndex;

				Item* nextItem = m_inventory[ itemIndex ];

				if( nextItem != m_rightHandEquippedItem && nextItem != m_leftHandEquippedItem )
				{
					if( nextItem != nullptr && nextItem->m_blueprint != nullptr )
					{
						if( nextItem->m_blueprint->m_equipType == EQUIP_TO_BODY || nextItem->m_blueprint->m_equipType == EQUIP_TO_ALL )
							itemToEquipt = nextItem;
					}
				}

				if( nextItem == m_bodyEquippedItem )
					return;
			}

			m_bodyEquippedItem = itemToEquipt;
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
attitudeTowardPlayer Actor::GetCurrentAttitude()
{
	if( m_blueprint == nullptr )
		return NEUTRAL_TO_PLAYER;

	if( m_currentHealth != m_blueprint->m_maxHP )
		return m_blueprint->m_attitudeWhenAttacked;

	return m_blueprint->m_attitudeTowardPlayer;
}


//-----------------------------------------------------------------------------------------------
void Actor::AddHealth( float amountToAdd )
{
	m_currentHealth += amountToAdd;
	if( m_blueprint != nullptr && m_currentHealth > m_blueprint->m_maxHP )
	{
		m_currentHealth = m_blueprint->m_maxHP;
	}
}


//-----------------------------------------------------------------------------------------------
void Actor::RemoveHealth( float amountToRemove )
{
	m_currentHealth -= amountToRemove;
	if( m_currentHealth < 0.f )
	{
		m_currentHealth = 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
void Actor::Update( float deltaSeconds )
{
	m_secondsSinceLastMove += deltaSeconds;

	float nextLevel = (float)( m_currentLevel + 1 );
	float experienceNeededForNextLevel = ( nextLevel * nextLevel ) + ( nextLevel - 1.f ) * 10.f;
	if( m_currentExperience >= experienceNeededForNextLevel )
	{
		m_currentLevel += 1;
	}
}


//-----------------------------------------------------------------------------------------------
void Actor::Render()
{
	if( GetBlueprint() == nullptr )
		return;

	if( EntityBlueprint::s_entityTextureMap == nullptr )
		return;

	OpenGLRenderer::EnableTexture2D();
	OpenGLRenderer::BindTexture2D( EntityBlueprint::s_entityTextureMap->m_openglTextureID );
	OpenGLRenderer::SetColor3f( 1.f, 1.f, 1.f );
	OpenGLRenderer::LoadMatrix();

	OpenGLRenderer::BeginRender( QUADS );
	{
		OpenGLRenderer::SetTexCoords2f( GetBlueprint()->m_spriteCoords.x, GetBlueprint()->m_spriteCoords.y + EntityBlueprint::s_texCoordOffset.y );
		OpenGLRenderer::SetVertex2f( -0.5f, -0.35f );

		OpenGLRenderer::SetTexCoords2f( GetBlueprint()->m_spriteCoords.x + EntityBlueprint::s_texCoordOffset.x, GetBlueprint()->m_spriteCoords.y + EntityBlueprint::s_texCoordOffset.y );
		OpenGLRenderer::SetVertex2f( 0.5f, -0.35f );

		OpenGLRenderer::SetTexCoords2f( GetBlueprint()->m_spriteCoords.x + EntityBlueprint::s_texCoordOffset.x, GetBlueprint()->m_spriteCoords.y );
		OpenGLRenderer::SetVertex2f( 0.5f, 0.55f );

		OpenGLRenderer::SetTexCoords2f( GetBlueprint()->m_spriteCoords.x, GetBlueprint()->m_spriteCoords.y );
		OpenGLRenderer::SetVertex2f( -0.5f, 0.55f );
	}
	OpenGLRenderer::EndRender();

	OpenGLRenderer::DisableTexture2D();

	RenderHealthBar();
}


//-----------------------------------------------------------------------------------------------
void Actor::RenderHealthBar()
{
	if( GetBlueprint() == nullptr )
		return;

	if( m_currentHealth == m_blueprint->m_maxHP )
		return;

	float percentageHealth = m_currentHealth / m_blueprint->m_maxHP;

	OpenGLRenderer::BeginRender( QUADS );
	{
		OpenGLRenderer::SetColor4f( HEALTH_BAR_BACKGROUND_COLOR.r, HEALTH_BAR_BACKGROUND_COLOR.g, HEALTH_BAR_BACKGROUND_COLOR.b, HEALTH_BAR_BACKGROUND_COLOR.a );
		OpenGLRenderer::SetVertex2f( -0.5f, -0.6f );
		OpenGLRenderer::SetVertex2f( 0.5f, -0.6f );
		OpenGLRenderer::SetVertex2f( 0.5f, -0.4f );
		OpenGLRenderer::SetVertex2f( -0.5f, -0.4f );

		OpenGLRenderer::SetColor4f( HEALTH_BAR_COLOR.r, HEALTH_BAR_COLOR.g, HEALTH_BAR_COLOR.b, HEALTH_BAR_COLOR.a );
		OpenGLRenderer::SetVertex2f( -0.45f, -0.55f );
		OpenGLRenderer::SetVertex2f( -0.45f + ( 0.9f * percentageHealth ), -0.55f );
		OpenGLRenderer::SetVertex2f( -0.45f + ( 0.9f * percentageHealth ), -0.45f );
		OpenGLRenderer::SetVertex2f( -0.45f, -0.45f );
	}
	OpenGLRenderer::EndRender();
}
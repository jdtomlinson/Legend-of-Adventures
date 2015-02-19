#include "Entity.hpp"
#include "../Engine/EventSystem.hpp"
#include "../Engine/OpenGLRenderer.hpp"


//-----------------------------------------------------------------------------------------------
int Entity::s_nexEntityID = 0;
std::map< int, Entity* > Entity::s_entities;


//-----------------------------------------------------------------------------------------------
Entity::Entity()
	: m_blueprint( nullptr )
	, m_entityID( s_nexEntityID )
{
	++s_nexEntityID;
	s_entities[ m_entityID ] = this;
}


//-----------------------------------------------------------------------------------------------
Entity::Entity( const std::string& entityBlueprintName )
	: m_blueprint( EntityBlueprint::GetEntityBlueprintByName( entityBlueprintName ) )
	, m_entityID( s_nexEntityID )
{
	++s_nexEntityID;
	s_entities[ m_entityID ] = this;
}


//-----------------------------------------------------------------------------------------------
Entity::~Entity()
{
	std::map< int, Entity* >::iterator eraseIter = Entity::s_entities.find( m_entityID );
	Entity::s_entities.erase( eraseIter );
	EventSystem::UnregisterEventCallbackForObject( this );
}


//-----------------------------------------------------------------------------------------------
STATIC Entity* Entity::GetEntityByID( int entityID )
{
	std::map< int, Entity* >::iterator entityIter = s_entities.find( entityID );
	if( entityIter == s_entities.end() )
		return nullptr;

	return entityIter->second;
}


//-----------------------------------------------------------------------------------------------
STATIC void Entity::SetEntity( Entity* entity )
{
	s_entities[ entity->m_entityID ] = entity;
}


//-----------------------------------------------------------------------------------------------
EntityBlueprint* Entity::GetBlueprint()
{
	return m_blueprint;
}


//-----------------------------------------------------------------------------------------------
void Entity::Update( float )
{

}


//-----------------------------------------------------------------------------------------------
void Entity::Render()
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
		OpenGLRenderer::SetVertex2f( -0.5f, -0.5f );

		OpenGLRenderer::SetTexCoords2f( GetBlueprint()->m_spriteCoords.x + EntityBlueprint::s_texCoordOffset.x, GetBlueprint()->m_spriteCoords.y + EntityBlueprint::s_texCoordOffset.y );
		OpenGLRenderer::SetVertex2f( 0.5f, -0.5f );

		OpenGLRenderer::SetTexCoords2f( GetBlueprint()->m_spriteCoords.x + EntityBlueprint::s_texCoordOffset.x, GetBlueprint()->m_spriteCoords.y );
		OpenGLRenderer::SetVertex2f( 0.5f, 0.5f );

		OpenGLRenderer::SetTexCoords2f( GetBlueprint()->m_spriteCoords.x, GetBlueprint()->m_spriteCoords.y );
		OpenGLRenderer::SetVertex2f( -0.5f, 0.5f );
	}
	OpenGLRenderer::EndRender();
}
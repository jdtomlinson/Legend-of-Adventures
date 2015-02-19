#ifndef include_ActorBlueprint
#define include_ActorBlueprint
#pragma once

//-----------------------------------------------------------------------------------------------
#include <string>
#include "EntityBlueprint.hpp"


//-----------------------------------------------------------------------------------------------
enum attitudeTowardPlayer
{
	HATES_PLAYER,
	FEARS_PLAYER,
	NEUTRAL_TO_PLAYER,
};


//-----------------------------------------------------------------------------------------------
class ActorBlueprint : public EntityBlueprint
{
public:
	float					m_maxHP;
	float					m_experienceToGive;
	float					m_stats[ NUM_STAT_TYPES ];
	attitudeTowardPlayer	m_attitudeTowardPlayer;
	attitudeTowardPlayer	m_attitudeWhenAttacked;
};


#endif // include_ActorBlueprint
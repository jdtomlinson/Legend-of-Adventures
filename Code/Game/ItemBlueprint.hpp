#ifndef include_ItemBlueprint
#define include_ItemBlueprint
#pragma once

//-----------------------------------------------------------------------------------------------
#include <string>
#include "EntityBlueprint.hpp"


//-----------------------------------------------------------------------------------------------
enum equipType
{
	EQUIP_TO_NONE,
	EQUIP_TO_HAND,
	EQUIP_TO_BODY,
	EQUIP_TO_ALL,
};


//-----------------------------------------------------------------------------------------------
class ItemBlueprint : public EntityBlueprint
{
public:
	equipType		m_equipType;
	int				m_range;
	float			m_secondsPerMove;
	float			m_statModifiers[ NUM_STAT_TYPES ];
	std::string		m_nameItemTypeToSpawn;
};


#endif // include_ItemBlueprint
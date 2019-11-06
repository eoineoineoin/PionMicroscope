#pragma once
#include <stdint.h>

namespace Packets
{
struct CurrentState
{
	uint16_t m_x;
	uint16_t m_y;
	uint32_t m_input0;
};

struct ControlCommand
{
	enum class Type
	{
		NONE,
		LOCK_X,
		UNLOCK_X,
		SET_RESOLUTION,
	};

	Type m_type;
	uint32_t m_extraData;
};

}


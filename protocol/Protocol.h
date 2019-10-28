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

}


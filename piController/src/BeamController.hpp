#pragma once
#include <Protocol.h>

class BeamController
{
	public:
		BeamController();
		Packets::CurrentState step();
			
	protected:
		struct
		{
			uint16_t m_xResolution = 128;
			uint16_t m_yResolution = 128;
			int m_pauseUsec = 2e3;
		} m_imageProps;

		struct
		{
			uint16_t m_lastX;
			uint16_t  m_lastY;
		} m_xyPlateState;

};


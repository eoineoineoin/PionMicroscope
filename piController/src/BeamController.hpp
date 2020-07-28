#pragma once
#include <Protocol.h>
#include <CommandHandler.hpp>
#include <DACBoard.h>

class BeamController
	: public CommandHandler
{
	public:
		BeamController();
		Packets::BeamState step();
			
		virtual void setResolution(uint16_t resolutionX, uint16_t resolutionY) override;
		virtual void lock(Axis axis, float frac) override;
		virtual void free(Axis axis) override;

		uint16_t getResolutionX() const { return m_imageProps.m_xResolution; }
		uint16_t getResolutionY() const { return m_imageProps.m_yResolution; }

	protected:
		struct
		{
			uint16_t m_xResolution = 128;
			uint16_t m_yResolution = 128;
			int m_pauseUsec = 2e3;

			bool m_isLocked[2] = {false, false};
			// When the axis is locked, it will force the
			// location to this target in the image:
			uint16_t m_lockTarget[2] = {0, 0};
		} m_imageProps;

		struct
		{
			uint16_t m_lastX;
			uint16_t  m_lastY;
		} m_xyPlateState;

		DACBoard m_d2a;
};


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
		virtual void lockX(uint16_t xImagespace) override;
		virtual void freeX() override;

		uint16_t getResolutionX() const { return m_imageProps.m_xResolution; }
		uint16_t getResolutionY() const { return m_imageProps.m_yResolution; }

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

		DACBoard m_d2a;
};


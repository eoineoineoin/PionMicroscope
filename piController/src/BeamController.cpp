#include <BeamController.hpp>
#include <DAC8532.h>
#include <ADS1256.h>
#include <unistd.h>

BeamController::BeamController()
	: m_d2a(DACBoard::REF_5V)
{
}

Packets::BeamState BeamController::step()
{
	m_xyPlateState.m_lastX += 1;
	if(m_xyPlateState.m_lastX >= m_imageProps.m_xResolution)
	{
		m_xyPlateState.m_lastX = 0;
		m_xyPlateState.m_lastY = (m_xyPlateState.m_lastY + 1) % m_imageProps.m_yResolution;
	}


	float xFrac = (float)m_xyPlateState.m_lastX / (float)m_imageProps.m_xResolution;
	float yFrac = (float)m_xyPlateState.m_lastY / (float)m_imageProps.m_yResolution;

	float xVoltage = xFrac / DAC_VREF;
	float yVoltage = yFrac / DAC_VREF;

	m_d2a.writeVoltage(DACBoard::Channel::A, xVoltage);
	m_d2a.writeVoltage(DACBoard::Channel::B, xVoltage);

	usleep(m_imageProps.m_pauseUsec);
	
	uint32_t inputVoltage = A2D::getChannelValue(1);
	Packets::BeamState stateOut;
	stateOut.m_x = m_xyPlateState.m_lastX;
	stateOut.m_y = m_xyPlateState.m_lastY;
	stateOut.m_input0 = inputVoltage;

	return stateOut;
}

void BeamController::setResolution(uint16_t resolutionX, uint16_t resolutionY)
{
	m_imageProps.m_xResolution = resolutionX;
	m_imageProps.m_yResolution = resolutionY;
}

void BeamController::lockX(uint16_t xImagespace)
{
	assert(false);
}

void BeamController::freeX()
{
	assert(false);
}

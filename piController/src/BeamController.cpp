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

	// Apply manual controls, if enabled:
	if(m_imageProps.m_isLocked[0])
		m_xyPlateState.m_lastX = m_imageProps.m_lockTarget[0];
	if(m_imageProps.m_isLocked[1])
		m_xyPlateState.m_lastY = m_imageProps.m_lockTarget[1];

	float xFrac = (float)m_xyPlateState.m_lastX / (float)m_imageProps.m_xResolution;
	float yFrac = (float)m_xyPlateState.m_lastY / (float)m_imageProps.m_yResolution;

	float xVoltage = xFrac / DAC_VREF;
	float yVoltage = yFrac / DAC_VREF;

	m_d2a.writeVoltage(DACBoard::Channel::A, xVoltage);
	m_d2a.writeVoltage(DACBoard::Channel::B, yVoltage);

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
	// Recalculate the fraction, since we converted it to a "pixel" value:
	float fracX = (float)m_imageProps.m_lockTarget[0] / (float)m_imageProps.m_xResolution;
	float fracY = (float)m_imageProps.m_lockTarget[1] / (float)m_imageProps.m_xResolution;

	m_imageProps.m_xResolution = resolutionX;
	m_imageProps.m_yResolution = resolutionY;

	// And re-apply the fraction:
	m_imageProps.m_lockTarget[0] = fracX * resolutionX;
	m_imageProps.m_lockTarget[1] = fracY * resolutionY;
}

void BeamController::lock(Axis axis, float frac)
{
	int idx = axis == Axis::Y;
	m_imageProps.m_isLocked[idx] = true;
	uint16_t axisResolution = idx == 0 ? m_imageProps.m_xResolution : m_imageProps.m_yResolution;
	m_imageProps.m_lockTarget[idx] = axisResolution * frac;
}

void BeamController::free(Axis axis)
{
	int idx = axis == Axis::Y;
	m_imageProps.m_isLocked[idx] = false;
}

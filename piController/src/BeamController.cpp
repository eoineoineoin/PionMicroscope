#include <BeamController.hpp>
#include <DAC8532.h>
#include <ADS1256.h>
#include <unistd.h>

BeamController::BeamController() = default;

Packets::CurrentState BeamController::step()
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

	DAC8532_Out_Voltage(channel_A, xVoltage);
	DAC8532_Out_Voltage(channel_B, yVoltage);

	usleep(m_imageProps.m_pauseUsec);
	
	uint32_t inputVoltage = ADS1256_GetChannalValue(1);
	return {m_xyPlateState.m_lastX, m_xyPlateState.m_lastY, inputVoltage};
}

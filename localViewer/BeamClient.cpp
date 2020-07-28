#include <BeamClient.hpp>

void BeamClient::connect(QString hostname, uint16_t port)
{
	m_serverConnection = std::make_unique<QTcpSocket>();
	m_serverConnection->connectToHost(hostname, port);
	QObject::connect(m_serverConnection.get(), &QTcpSocket::readyRead, this, &BeamClient::dataReadyToRead);
}

void BeamClient::sendResolutionChange(int xyResolution)
{
	if(m_serverConnection == nullptr)
		return;

	Packets::SetResolution setRes;

	if(xyResolution < 0 || xyResolution > UINT16_MAX)
	{
		// Outside range of what we can display.
		return;
	}

	setRes.m_resolutionX = (uint16_t)xyResolution;
	setRes.m_resolutionY = (uint16_t)xyResolution;

	m_serverConnection->write(reinterpret_cast<const char*>(&setRes), sizeof(setRes));
}

void BeamClient::sendManualControlChange(bool xLock, float xFrac, bool yLock, float yFrac)
{
	if(m_serverConnection == nullptr)
		return;

	Packets::SetTargetMode setTarget;
	using AxisMode = Packets::SetTargetMode::AxisMode;
	setTarget.m_modeX = xLock ? AxisMode::LOCK : AxisMode::UNLOCK;
	setTarget.m_modeY = yLock ? AxisMode::LOCK : AxisMode::UNLOCK;
	setTarget.m_fracX = (uint16_t)(xFrac * UINT16_MAX);
	setTarget.m_fracY = (uint16_t)(yFrac * UINT16_MAX);

	m_serverConnection->write(reinterpret_cast<const char*>(&setTarget), sizeof(setTarget));
}

void BeamClient::dataReadyToRead()
{
	uint8_t inputBuffer[800];
	int bytesRead = m_serverConnection->read((char*)inputBuffer, sizeof(inputBuffer));
	assert(bytesRead % sizeof(Packets::BeamState) == 0);

	const Packets::BasePacket* curPacket = reinterpret_cast<Packets::BasePacket*>(inputBuffer);
	const Packets::BasePacket* packetEnd = reinterpret_cast<Packets::BasePacket*>(inputBuffer + bytesRead);

	std::vector<Packets::BeamState> beamStatesAccumulated;
	beamStatesAccumulated.reserve((packetEnd - curPacket) / sizeof(Packets::BeamState));
	while(curPacket < packetEnd)
	{
		if(curPacket->m_type == Packets::Type::BEAM_STATE)
		{
			const Packets::BeamState* beamState = curPacket->asBeamState();
			beamStatesAccumulated.push_back(*beamState);
			curPacket = beamState + 1;
		}
		else if(curPacket->m_type == Packets::Type::RESOLUTION_CHANGED)
		{
			const Packets::ResolutionChanged* resChanged = curPacket->asResolutionChanged();
			emit onResolutionChanged(*resChanged);
			curPacket = resChanged + 1;
		}
		else if(curPacket->m_type == Packets::Type::SET_TARGET_MODE)
		{
			const Packets::SetTargetMode* targetChanged = curPacket->asSetTargetMode();
			emit onManualControlsUpdated(*targetChanged);
			curPacket = targetChanged + 1;
		}
		else
		{
			assert(false && "Packet type not handled");
		}
	}

	if(beamStatesAccumulated.size())
	{
		emit onBeamUpdated(beamStatesAccumulated);
	}
}

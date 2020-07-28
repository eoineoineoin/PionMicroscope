#pragma once
#include <memory>
#include <Protocol.h>
#include <QTcpSocket>

class BeamClient : public QObject
{
public:
	Q_OBJECT;

signals:
	void onResolutionChanged(const Packets::ResolutionChanged& newResolution);
	void onBeamUpdated(const std::vector<Packets::BeamState>& beamUpdates);
	void onManualControlsUpdated(const Packets::SetTargetMode& newTarget);

public slots:
	void connect(QString hostname, uint16_t port);
	void sendResolutionChange(int xyResolution);
	void sendManualControlChange(bool xLock, float xFrac, bool yLock, float yFrac);

protected:
	void dataReadyToRead();

	std::unique_ptr<class QTcpSocket> m_serverConnection;
};


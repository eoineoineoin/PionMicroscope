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

public slots:
	void connect(QString hostname, uint16_t port);
	void sendResolutionChange(int xyResolution);

protected:
	void dataReadyToRead();

	std::unique_ptr<class QTcpSocket> m_serverConnection;
};


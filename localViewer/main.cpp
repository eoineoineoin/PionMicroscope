#include <ViewerWindow.h>
#include <ImageGenerator.h>
#include <QApplication>
#include <QTcpSocket>
#include <cassert>

#include <Protocol.h>

int main(int argc, char** argv)
{
	QApplication a(argc, argv);
	ViewerWindow window;
	window.show();

	ImageGenerator imageGenerator;
	QObject::connect(&imageGenerator, &ImageGenerator::updatedImage, &window, &ViewerWindow::updateImage);
	QObject::connect(&imageGenerator, &ImageGenerator::resolutionChanged, &window, &ViewerWindow::setImageSize);

	std::unique_ptr<class QTcpSocket> serverConnection;

	// Signals for network connection to inform viewers when we get data
	QObject::connect(&window, &ViewerWindow::connectRequested,
		[&serverConnection, &window, &imageGenerator](QString hostname, uint16_t port)
		{
			serverConnection = std::make_unique<QTcpSocket>();
			serverConnection->connectToHost(hostname, port);

			QObject::connect(serverConnection.get(), &QTcpSocket::readyRead,
			[&window, &serverConnection, &imageGenerator]()
			{
				//<TODO.eoin Getting unwieldy. Move to a seperate class.
				uint8_t inputBuffer[800];
				int bytesRead = serverConnection->read((char*)inputBuffer, sizeof(inputBuffer));
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
						const Packets::ResolutionChanged* resolutionChanged = curPacket->asResolutionChanged();
						imageGenerator.setResolution(resolutionChanged->m_resolutionX, resolutionChanged->m_resolutionY);
						curPacket = resolutionChanged + 1;
					}
					else
					{
						assert(false && "Packet type not handled");
					}
				}

				if(beamStatesAccumulated.size())
				{
					imageGenerator.updatePixels(&beamStatesAccumulated[0], beamStatesAccumulated.size());

					float lastFrac = (float)((beamStatesAccumulated.back().m_input0) / (float)(~(decltype(beamStatesAccumulated.back().m_input0))0));
					window.newReadout(lastFrac);
				}

			});
		});

	// Save image signal
	QObject::connect(&window, &ViewerWindow::saveImage, &imageGenerator, &ImageGenerator::saveImage);
	a.exec();
}

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
				Packets::CurrentState packet[100];
				int bytesRead = serverConnection->read((char*)packet, sizeof(packet));
				assert(bytesRead % sizeof(packet[0]) == 0);

				int numRead = bytesRead / sizeof(packet[0]);
				imageGenerator.updatePixels(packet, numRead);

				float lastFrac = (float)((double)packet[numRead - 1].m_input0 / (double)0x6fffff);
				window.newReadout(lastFrac);
			});
		});

	// Save image signal
	QObject::connect(&window, &ViewerWindow::saveImage, &imageGenerator, &ImageGenerator::saveImage);
	a.exec();
}

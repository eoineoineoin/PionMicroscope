#include <ViewerWindow.hpp>
#include <ImageGenerator.hpp>
#include <QApplication>
#include <cassert>

#include <Protocol.h>
#include <BeamClient.hpp>

int main(int argc, char** argv)
{
	QApplication a(argc, argv);
	ViewerWindow window;
	window.show();
	
	ImageGenerator imageGenerator;
	QObject::connect(&imageGenerator, &ImageGenerator::updatedImage, &window, &ViewerWindow::updateImage);
	QObject::connect(&imageGenerator, &ImageGenerator::resolutionChanged, &window, &ViewerWindow::setImageSize);

	BeamClient server;

	// For user pressing "connect" button:
	QObject::connect(&window, &ViewerWindow::connectRequested, &server, &BeamClient::connect);

	// When server sends a "resolution changed" confirmation, resize our image:
	QObject::connect(&server, &BeamClient::onResolutionChanged,
		[&imageGenerator](const Packets::ResolutionChanged& newRes)
		{
			printf("Got resolution changed\n");
			imageGenerator.setResolution(newRes.m_resolutionX, newRes.m_resolutionY);
		});

	// When server sends a batch of beam updates, write them to the image:
	QObject::connect(&server, &BeamClient::onBeamUpdated,
		[&imageGenerator, &window](const std::vector<Packets::BeamState>& beamStates)
		{
			imageGenerator.updatePixels(&beamStates[0], beamStates.size());

			//TODO: Formalize this, as it is wrong, after correcting input lib:
			float maxVal = (float)(~(decltype(beamStates.back().m_input0))0);
			float lastFrac = (float)((beamStates.back().m_input0) / maxVal);
			window.newReadout(lastFrac);
		});

	
	// For user pressing "save" button:
	QObject::connect(&window, &ViewerWindow::saveImage, &imageGenerator, &ImageGenerator::saveImage);
	a.exec();
}

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
	QObject::connect(&window, &ViewerWindow::clearImageRequested, &imageGenerator, &ImageGenerator::clearImage);

	BeamClient server;

	// For user pressing "connect" button:
	QObject::connect(&window, &ViewerWindow::connectRequested, &server, &BeamClient::connect);

	// For user pressing "change resolution" button. The way this will work is that we'll
	// send a packet to the server with our new resolution, and we'll eventually get a
	// confirmation back; only then will we change the display. This allows for multiple
	// clients connected to a server, where one might send the "change resolution" command.
	QObject::connect(&window, &ViewerWindow::newResolutionRequested,
		&server, &BeamClient::sendResolutionChange);

	// When server sends a "resolution changed" confirmation, resize our image:
	QObject::connect(&server, &BeamClient::onResolutionChanged,
		[&imageGenerator](const Packets::ResolutionChanged& newRes)
		{
			printf("Got resolution changed\n");
			imageGenerator.setResolution(newRes.m_resolutionX, newRes.m_resolutionY);
		});

	// Similarly, when the user changes the manual beam controls, send the command
	// that requests the mode and expect aconfirmation, so other clients can see:
	QObject::connect(&window, &ViewerWindow::newBeamStateRequested,
		&server, &BeamClient::sendManualControlChange);

	QObject::connect(&server, &BeamClient::onManualControlsUpdated,
		[&window](const Packets::SetTargetMode& newTarget)
		{
			bool lockedX = newTarget.m_modeX == Packets::SetTargetMode::AxisMode::LOCK;
			bool lockedY = newTarget.m_modeY == Packets::SetTargetMode::AxisMode::LOCK;
			float fracX = (float)newTarget.m_fracX / (float)UINT16_MAX;
			float fracY = (float)newTarget.m_fracY / (float)UINT16_MAX;
			window.setDisplayedManualControls(lockedX, fracX, lockedY, fracY);
		});
	
	// When server sends a batch of beam updates, write them to the image:
	QObject::connect(&server, &BeamClient::onBeamUpdated,
		[&imageGenerator, &window](const std::vector<Packets::BeamState>& beamStates)
		{
			imageGenerator.updatePixels(&beamStates[0], beamStates.size());

			// Select the most recent update and use it to update the UI, just
			// for some visual confirmation of activity, without having to
			// scrutinize the image in the case of small changes.
			float lastVoltage = beamStates.back().unpackVoltage();
			float lastFrac = lastVoltage / Packets::BeamState::maxVoltage();
			window.newReadout(lastFrac);
		});

	
	// For user pressing "save" button:
	QObject::connect(&window, &ViewerWindow::saveImage, &imageGenerator, &ImageGenerator::saveImage);
	a.exec();
}

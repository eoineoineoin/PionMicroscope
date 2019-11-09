#pragma once
#include <QObject>
#include <QRect>
#include <memory>
#include <deque>
#include <Protocol.h>

// A utility to generate an image; accepts protocol pixel
// updates and updates a pixmap in a way that makes it
// visible when the most recent pixels arrived
class ImageGenerator
	: public QObject 
{
	Q_OBJECT
	public:
		ImageGenerator();
		~ImageGenerator();

		void updatePixels(Packets::BeamState* newState, int numStates);
		void setResolution(uint16_t resX, uint16_t resY);

	signals:
		void updatedImage(QImage* image);
		void resolutionChanged(QRect imageSize);

	public slots:
		void saveImage(QString filenameOut);

	protected:
		std::unique_ptr<QImage> m_imageData;
		std::deque<Packets::BeamState> m_recvQueue;
};


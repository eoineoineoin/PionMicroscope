#pragma once
#include <QObject>
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

		void updatePixels(Packets::CurrentState* newState, int numStates);
	signals:
		void updatedImage(QImage* image);

	public slots:
		void saveImage(QString filenameOut);

	protected:
		std::unique_ptr<QImage> m_imageData;
		std::deque<Packets::CurrentState> m_recvQueue;
};


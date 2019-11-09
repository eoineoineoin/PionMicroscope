#pragma once
#include <QMainWindow>
#include <memory>

class QString;
class QGraphicsScene;
class QGraphicsPixmapItem;

class ViewerWindow
	: public QMainWindow
{
	Q_OBJECT
	public:
		ViewerWindow();
		~ViewerWindow();

	signals:
		// Update display with most recently read value, in range 0-1
		void newReadout(float readout);
		void connectRequested(QString hostname, uint16_t port);
		void saveImage(QString filenameOut);
		void newResolutionRequested(int res);
		void toggleXLockRequested();
	
	public slots:
		void updateImage(QImage* imageData);
		void setImageSize(QRect imageSize);

	protected:
		QGraphicsScene* m_imageScene;
		QGraphicsPixmapItem* m_imageDisplayItem;
};


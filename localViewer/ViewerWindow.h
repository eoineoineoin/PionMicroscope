#pragma once
#include <QMainWindow>
#include <memory>

class QString;
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
	
	public slots:
		void updateImage(QImage* imageData);

	protected:
		QGraphicsPixmapItem* m_imageDisplayItem;
};

